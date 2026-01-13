/***************************************************************************
 * Unit Tests: Edge Cases and Validation
 *
 * Tests edge cases, boundary conditions, and error handling
 * for the keyframe generation system.
 ***************************************************************************/

#include "../src/AnimationCommand.h"
#include "../src/KeyframeGenerator.h"
#include "../src/GeometryAnimationBridge.h"
#include <iostream>
#include <cassert>

using namespace GCS;

// Mock implementation of GeometryGraph for testing
namespace GeometryTypes {
    class TestNode : public GeometryNode {
    private:
        int id_;
        GeometryNodeType type_;
        double x_, y_;
        double radius_;

    public:
        TestNode(int id, GeometryNodeType type, double x = 0, double y = 0, double radius = 1.0)
            : id_(id), type_(type), x_(x), y_(y), radius_(radius) {}

        int getId() const override { return id_; }
        GeometryNodeType getType() const override { return type_; }
        double getX() const override { return x_; }
        double getY() const override { return y_; }
        double getRadius() const override { return radius_; }
    };

    class TestGraph : public GeometryGraph {
    private:
        std::vector<std::unique_ptr<GeometryNode>> nodes_;

    public:
        void addNode(int id, GeometryNodeType type, double x = 0, double y = 0, double radius = 1.0) {
            nodes_.push_back(std::make_unique<TestNode>(id, type, x, y, radius));
        }

        const std::vector<std::unique_ptr<GeometryNode>>& getNodes() const override {
            return nodes_;
        }
    };
}

void testEmptyGraphs() {
    std::cout << "=== Unit Test: Empty Graphs ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Both graphs empty
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::NO_CHANGE && "Empty graphs should detect NO_CHANGE");
        std::cout << "[PASS] Empty graphs handled correctly" << std::endl;
    }

    // Test 2: Generate keyframes from empty graphs
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        GeometryAnimationBridge bridge;
        KeyframeGenerationConfig config;

        std::string json = bridge.generateAnimationKeyframes(oldGraph, newGraph, config);

        assert(!json.empty() && "Should generate JSON even for empty graphs");
        std::cout << "[PASS] Keyframes generated for empty graphs" << std::endl;
        std::cout << "  JSON length: " << json.length() << " characters" << std::endl;
    }
}

void testSingleNode() {
    std::cout << "\n=== Unit Test: Single Node ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Single node in both graphs (no change)
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);
        newGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::NO_CHANGE && "Identical single node should be NO_CHANGE");
        std::cout << "[PASS] Single node with no change detected" << std::endl;
    }

    // Test 2: Single node with change
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);
        newGraph.addNode(1, GeometryTypes::POINT, 100.0, 100.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR && "Moving single node should be MOVE_LINEAR");
        std::cout << "[PASS] Single node movement detected" << std::endl;
    }
}

void testVerySmallChanges() {
    std::cout << "\n=== Unit Test: Very Small Changes ===" << std::endl;

    AnimationCommandDetector detector;
    AnimationCommandDetector::DetectionConfig config;
    config.epsilon = 1e-9;  // Very strict threshold
    AnimationCommandDetector strictDetector(config);

    // Test 1: Change below default epsilon
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);
        newGraph.addNode(1, GeometryTypes::POINT, 50.0000001, 50.0);  // Tiny change

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // Should detect NO_CHANGE with default epsilon (1e-6)
        assert(cmd.command_type == AnimationCommandType::NO_CHANGE);
        std::cout << "[PASS] Tiny change below epsilon ignored" << std::endl;
    }

    // Test 2: Same change detected with strict epsilon
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);
        newGraph.addNode(1, GeometryTypes::POINT, 50.0000001, 50.0);  // Tiny change

        AnimationCommand cmd = strictDetector.detectCommand(oldGraph, newGraph);

        // Should detect MOVE_LINEAR with strict epsilon (1e-9)
        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Tiny change detected with strict epsilon" << std::endl;
    }
}

void testZeroFramesPerCommand() {
    std::cout << "\n=== Unit Test: Zero Frames Per Command ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 0;  // Invalid configuration
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test: Generate keyframes with zero frames
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "1";
        cmd.element_name = "Point1";
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        // Should generate at least 1 frame even with zero config
        assert(keyframes.size() >= 1 && "Should generate at least one frame");
        std::cout << "[PASS] Zero frames config handled gracefully" << std::endl;
        std::cout << "  Generated frames: " << keyframes.size() << std::endl;
    }
}

void testLargeFrameCounts() {
    std::cout << "\n=== Unit Test: Large Frame Counts ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 1000;  // Large number
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test: Generate many frames
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "1";
        cmd.element_name = "Point1";
        cmd.addPropertyChange("x", "0.0", "100.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 1000 && "Should generate exactly 1000 frames");
        assert(keyframes[0].frameIndex == 0 && "First frame should be 0");
        assert(keyframes[999].frameIndex == 999 && "Last frame should be 999");
        std::cout << "[PASS] Large frame count handled correctly" << std::endl;
        std::cout << "  Generated frames: " << keyframes.size() << std::endl;
    }
}

void testMultiplePropertyChanges() {
    std::cout << "\n=== Unit Test: Multiple Property Changes ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test: Command with many property changes
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "1";
        cmd.element_name = "Point1";
        cmd.addPropertyChange("x", "0.0", "10.0");
        cmd.addPropertyChange("y", "0.0", "20.0");
        cmd.addPropertyChange("z", "0.0", "30.0");  // Extra property
        cmd.addPropertyChange("width", "1.0", "5.0");
        cmd.addPropertyChange("height", "1.0", "5.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 5 && "Should generate 5 frames");
        assert(keyframes[0].propertyChanges.size() == 5 && "Should have all 5 property changes");
        std::cout << "[PASS] Multiple property changes handled" << std::endl;
        std::cout << "  Property changes per frame: " << keyframes[0].propertyChanges.size() << std::endl;
    }
}

void testInvalidElementId() {
    std::cout << "\n=== Unit Test: Invalid Element ID ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test: Command with empty element ID
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "";  // Empty ID
        cmd.element_name = "Point1";
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        // Should still generate keyframes
        assert(keyframes.size() == 5 && "Should generate frames even with empty ID");
        std::cout << "[PASS] Empty element ID handled" << std::endl;
    }

    // Test: Command with negative element ID
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "-1";  // Negative ID
        cmd.element_name = "Point1";
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 5 && "Should generate frames with negative ID");
        std::cout << "[PASS] Negative element ID handled" << std::endl;
    }
}

void testMixedCommandTypes() {
    std::cout << "\n=== Unit Test: Mixed Command Types ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 3;
    config.interpolation_mode = InterpolationMode::LINEAR;

    // Test: Commands of different types
    {
        std::vector<AnimationCommand> commands;

        AnimationCommand cmd1(AnimationCommandType::ADD_ELEMENT);
        cmd1.element_id = "1";
        cmd1.element_name = "Point1";
        cmd1.addPropertyChange("x", "10.0", "10.0");
        commands.push_back(cmd1);

        AnimationCommand cmd2(AnimationCommandType::MOVE_LINEAR);
        cmd2.element_id = "2";
        cmd2.element_name = "Point2";
        cmd2.addPropertyChange("x", "0.0", "100.0");
        commands.push_back(cmd2);

        AnimationCommand cmd3(AnimationCommandType::NO_CHANGE);
        cmd3.element_id = "none";
        cmd3.element_name = "";
        commands.push_back(cmd3);

        GeometryAnimationBridge bridge;
        std::string json = bridge.generateKeyframesFromCommands(commands, config);

        assert(!json.empty() && "Should generate JSON for mixed commands");
        std::cout << "[PASS] Mixed command types handled" << std::endl;
        std::cout << "  JSON length: " << json.length() << " characters" << std::endl;
    }
}

void testExtremeCoordinateValues() {
    std::cout << "\n=== Unit Test: Extreme Coordinate Values ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Very large coordinates
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 1e10, 1e10);
        newGraph.addNode(1, GeometryTypes::POINT, 1e10 + 100.0, 1e10 + 200.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Very large coordinates handled" << std::endl;
    }

    // Test 2: Negative coordinates
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, -1000.0, -2000.0);
        newGraph.addNode(1, GeometryTypes::POINT, -500.0, -1000.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Negative coordinates handled" << std::endl;
    }

    // Test 3: Zero coordinates
    {
        GeometryTypes::TestGraph oldGraph;
        GeometryTypes::TestGraph newGraph;

        oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);
        newGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Zero coordinates handled" << std::endl;
    }
}

void testSpecialCharactersInNames() {
    std::cout << "\n=== Unit Test: Special Characters in Names ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 3;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test: Element name with special characters
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
        cmd.element_id = "1";
        cmd.element_name = "Point_With-Spaces And.Dots";
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 3 && "Should handle special characters in names");
        std::cout << "[PASS] Special characters in names handled" << std::endl;
        std::cout << "  Element name: " << keyframes[0].elementId << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Unit Tests: Edge Cases & Validation  " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testEmptyGraphs();
        testSingleNode();
        testVerySmallChanges();
        testZeroFramesPerCommand();
        testLargeFrameCounts();
        testMultiplePropertyChanges();
        testInvalidElementId();
        testMixedCommandTypes();
        testExtremeCoordinateValues();
        testSpecialCharactersInNames();

        std::cout << "\n========================================" << std::endl;
        std::cout << "       ALL EDGE CASE TESTS PASSED!     " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nX TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
