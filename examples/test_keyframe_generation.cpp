/***************************************************************************
 * Test: Keyframe Generation System
 *
 * Simple test to verify the new keyframe generation API works correctly
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

void testBasicAPI() {
    std::cout << "=== Test 1: Basic API ===" << std::endl;

    // Test AnimationCommand creation
    AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
    cmd.element_id = "point_1";
    cmd.element_name = "Point 1";
    cmd.addPropertyChange("x", "0.0", "10.0");
    cmd.addPropertyChange("y", "0.0", "20.0");

    assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
    assert(cmd.element_id == "point_1");
    assert(cmd.property_changes.size() == 2);

    std::cout << "[PASS] AnimationCommand creation works" << std::endl;

    // Test KeyframeGenerationConfig
    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    assert(config.frames_per_command == 5);

    std::cout << "[PASS] KeyframeGenerationConfig works" << std::endl;
}

void testKeyframeGeneration() {
    std::cout << "\n=== Test 2: Keyframe Generation ===" << std::endl;

    // Create a simple MOVE_LINEAR command
    AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
    cmd.element_id = "point_1";
    cmd.element_name = "Point 1";
    cmd.addPropertyChange("x", "0.0", "10.0");
    cmd.addPropertyChange("y", "0.0", "10.0");

    std::vector<AnimationCommand> commands;
    commands.push_back(cmd);

    // Generate keyframes
    KeyframeGenerator generator;
    auto keyframes = generator.generateKeyframes(commands);

    std::cout << "Generated " << keyframes.size() << " keyframes" << std::endl;
    assert(keyframes.size() == 10); // Default is 10 frames per command

    // Check first frame
    const auto& first = keyframes[0];
    assert(first.frameIndex == 0);
    assert(first.elementId == "point_1");
    assert(first.elementType == std::string("MOVE_LINEAR"));

    std::cout << "[PASS] Keyframe generation works" << std::endl;
    std::cout << "  First frame index: " << first.frameIndex << std::endl;
    std::cout << "  Element ID: " << first.elementId << std::endl;
    std::cout << "  Element type: " << first.elementType << std::endl;
}

void testJSONSerialization() {
    std::cout << "\n=== Test 3: JSON Serialization ===" << std::endl;

    // Create a simple command
    AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
    cmd.element_id = "point_1";
    cmd.element_name = "Point 1";
    cmd.addPropertyChange("x", "0.0", "10.0");

    std::vector<AnimationCommand> commands;
    commands.push_back(cmd);

    // Generate and serialize
    KeyframeGenerator generator(KeyframeGenerationConfig(3, 1e-6, InterpolationMode::LINEAR));
    auto keyframes = generator.generateKeyframes(commands);
    std::string json = generator.serializeToJSON(keyframes);

    std::cout << "Generated JSON (" << json.length() << " chars)" << std::endl;
    assert(json.length() > 0);
    assert(json.find("\"frameIndex\":") != std::string::npos);
    assert(json.find("\"elementId\":") != std::string::npos);
    assert(json.find("point_1") != std::string::npos);

    std::cout << "[PASS] JSON serialization works" << std::endl;
    std::cout << "  JSON preview (first 200 chars):" << std::endl;
    std::cout << "  " << json.substr(0, 200) << "..." << std::endl;
}

void testCommandDetection() {
    std::cout << "\n=== Test 4: Command Detection ===" << std::endl;

    // Create two graphs: one with point at (0,0), another with point at (5,5)
    GeometryTypes::TestGraph oldGraph;
    oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);

    GeometryTypes::TestGraph newGraph;
    newGraph.addNode(1, GeometryTypes::POINT, 5.0, 5.0);

    // Detect command
    AnimationCommandDetector detector;
    AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

    assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
    assert(cmd.element_id == "1");

    std::cout << "[PASS] Command detection works" << std::endl;
    std::cout << "  Detected command: " << toString(cmd.command_type) << std::endl;
    std::cout << "  Element ID: " << cmd.element_id << std::endl;
    std::cout << "  Property changes: " << cmd.property_changes.size() << std::endl;
}

void testBridgeAPI() {
    std::cout << "\n=== Test 5: Bridge API ===" << std::endl;

    // Create two graphs
    GeometryTypes::TestGraph oldGraph;
    oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);

    GeometryTypes::TestGraph newGraph;
    newGraph.addNode(1, GeometryTypes::POINT, 10.0, 10.0);

    // Use bridge to generate keyframes
    GeometryAnimationBridge bridge;
    KeyframeGenerationConfig config;
    config.frames_per_command = 5;

    std::string json = bridge.generateAnimationKeyframes(oldGraph, newGraph, config);

    assert(json.length() > 0);
    assert(json.find("MOVE_LINEAR") != std::string::npos);

    std::cout << "[PASS] Bridge API works" << std::endl;
    std::cout << "  Generated JSON length: " << json.length() << " chars" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Keyframe Generation System Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testBasicAPI();
        testKeyframeGeneration();
        testJSONSerialization();
        testCommandDetection();
        testBridgeAPI();

        std::cout << "\n========================================" << std::endl;
        std::cout << "      ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
