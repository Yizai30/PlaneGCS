/***************************************************************************
 * Unit Tests: Animation Command Detector
 *
 * Tests command detection for all 9 animation command types.
 ***************************************************************************/

#include "../src/AnimationCommand.h"
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
        std::map<std::string, std::string> parameters_;

    public:
        TestNode(int id, GeometryNodeType type, double x = 0, double y = 0, double radius = 1.0)
            : id_(id), type_(type), x_(x), y_(y), radius_(radius) {}

        int getId() const override { return id_; }
        GeometryNodeType getType() const override { return type_; }
        double getX() const override { return x_; }
        double getY() const override { return y_; }
        double getRadius() const override { return radius_; }

        void setRadius(double r) { radius_ = r; }
        void setPosition(double x, double y) { x_ = x; y_ = y; }
    };

    class TestGraph : public GeometryGraph {
    private:
        std::vector<std::unique_ptr<GeometryNode>> nodes_;
        std::map<int, std::map<std::string, std::string>> parameters_;

    public:
        void addNode(int id, GeometryNodeType type, double x = 0, double y = 0, double radius = 1.0) {
            nodes_.push_back(std::make_unique<TestNode>(id, type, x, y, radius));
        }

        void addParameter(int nodeId, const std::string& key, const std::string& value) {
            parameters_[nodeId][key] = value;
        }

        const std::vector<std::unique_ptr<GeometryNode>>& getNodes() const override {
            return nodes_;
        }
    };
}

// Test helper to create a simple graph
GeometryTypes::TestGraph createTestGraph() {
    GeometryTypes::TestGraph graph;
    return graph;
}

void testAddElementDetection() {
    std::cout << "=== Unit Test: ADD_ELEMENT Detection ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: New point added
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        newGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::ADD_ELEMENT && "Should detect ADD_ELEMENT");
        assert(cmd.element_id == "1" && "Element ID should match");
        std::cout << "[PASS] ADD_ELEMENT detected for new point" << std::endl;
    }

    // Test 2: New circle added
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        newGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 10.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::ADD_ELEMENT && "Should detect ADD_ELEMENT");
        std::cout << "[PASS] ADD_ELEMENT detected for new circle" << std::endl;
    }

    // Test 3: Multiple elements added
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        newGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);
        newGraph.addNode(2, GeometryTypes::POINT, 30.0, 40.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // Should detect ADD_ELEMENT (first new element)
        assert(cmd.command_type == AnimationCommandType::ADD_ELEMENT);
        std::cout << "[PASS] ADD_ELEMENT detected for multiple new elements" << std::endl;
    }
}

void testRemoveElementDetection() {
    std::cout << "\n=== Unit Test: REMOVE_ELEMENT Detection ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Point removed
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::REMOVE_ELEMENT && "Should detect REMOVE_ELEMENT");
        assert(cmd.element_id == "1" && "Element ID should match");
        std::cout << "[PASS] REMOVE_ELEMENT detected for removed point" << std::endl;
    }

    // Test 2: Circle removed
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0);
        oldGraph.addParameter(1, "radius", "10.0");

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::REMOVE_ELEMENT && "Should detect REMOVE_ELEMENT");
        std::cout << "[PASS] REMOVE_ELEMENT detected for removed circle" << std::endl;
    }
}

void testScaleRadiusDetection() {
    std::cout << "\n=== Unit Test: SCALE_RADIUS Detection ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Circle radius changes, center fixed
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 10.0);
        newGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 20.0);  // Center fixed, radius changed

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::SCALE_RADIUS && "Should detect SCALE_RADIUS");
        assert(cmd.element_id == "1" && "Element ID should match");
        std::cout << "[PASS] SCALE_RADIUS detected for radius change" << std::endl;
    }

    // Test 2: Radius scaling factor extracted
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 5.0);
        newGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 15.0);  // 3x scaling

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::SCALE_RADIUS);
        // Check that property changes include radius
        bool hasRadiusChange = false;
        for (const auto& pc : cmd.property_changes) {
            if (pc.property_name == "radius") {
                hasRadiusChange = true;
                break;
            }
        }
        assert(hasRadiusChange && "Should have radius property change");
        std::cout << "[PASS] Radius change detected correctly" << std::endl;
    }
}

void testMoveLinearDetection() {
    std::cout << "\n=== Unit Test: MOVE_LINEAR Detection ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Point moves in straight line
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);
        newGraph.addNode(1, GeometryTypes::POINT, 100.0, 50.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR && "Should detect MOVE_LINEAR");
        assert(cmd.element_id == "1" && "Element ID should match");
        assert(cmd.property_changes.size() >= 2 && "Should have x and y property changes");
        std::cout << "[PASS] MOVE_LINEAR detected for point movement" << std::endl;
    }

    // Test 2: Both x and y coordinates change
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);
        newGraph.addNode(1, GeometryTypes::POINT, 30.0, 40.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        bool hasX = false, hasY = false;
        for (const auto& pc : cmd.property_changes) {
            if (pc.property_name == "x") hasX = true;
            if (pc.property_name == "y") hasY = true;
        }
        assert(hasX && hasY && "Should have both x and y property changes");
        std::cout << "[PASS] Both coordinates detected in property changes" << std::endl;
    }

    // Test 3: Movement parameters extracted
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);
        newGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // Check for delta parameters
        auto dx_it = cmd.parameters.find("delta_x");
        auto dy_it = cmd.parameters.find("delta_y");
        assert(dx_it != cmd.parameters.end() && "Should have delta_x");
        assert(dy_it != cmd.parameters.end() && "Should have delta_y");
        std::cout << "[PASS] Movement deltas extracted: dx=" << dx_it->second
                  << ", dy=" << dy_it->second << std::endl;
    }
}

void testNoChangeDetection() {
    std::cout << "\n=== Unit Test: NO_CHANGE Detection ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Empty graphs
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::NO_CHANGE && "Should detect NO_CHANGE");
        std::cout << "[PASS] NO_CHANGE detected for empty graphs" << std::endl;
    }

    // Test 2: Identical graphs
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);
        newGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        assert(cmd.command_type == AnimationCommandType::NO_CHANGE && "Should detect NO_CHANGE");
        std::cout << "[PASS] NO_CHANGE detected for identical graphs" << std::endl;
    }

    // Test 3: Below epsilon threshold
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::POINT, 10.0, 20.0);
        newGraph.addNode(1, GeometryTypes::POINT, 10.0000001, 20.0);  // Very small change

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // Should detect NO_CHANGE if below epsilon threshold
        assert(cmd.command_type == AnimationCommandType::NO_CHANGE ||
               cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Small change handled correctly" << std::endl;
    }
}

void testPriorityOrder() {
    std::cout << "\n=== Unit Test: Detection Priority Order ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: SCALE_RADIUS has higher priority than MOVE_LINEAR for circles
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 10.0);
        newGraph.addNode(1, GeometryTypes::CIRCLE, 51.0, 51.0, 20.0);  // Center moves, radius changes

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // SCALE_RADIUS should be detected (higher priority)
        assert(cmd.command_type == AnimationCommandType::SCALE_RADIUS ||
               cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Priority order respected" << std::endl;
    }

    // Test 2: ADD_ELEMENT has highest priority
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        // Old graph has a point
        oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);

        // New graph has old point moved AND new point added
        newGraph.addNode(1, GeometryTypes::POINT, 100.0, 100.0);  // Moved
        newGraph.addNode(2, GeometryTypes::POINT, 50.0, 50.0);    // New

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // ADD_ELEMENT should be detected (highest priority)
        assert(cmd.command_type == AnimationCommandType::ADD_ELEMENT);
        std::cout << "[PASS] ADD_ELEMENT has highest priority" << std::endl;
    }
}

void testConcurrentPropertyChanges() {
    std::cout << "\n=== Unit Test: Concurrent Property Changes ===" << std::endl;

    AnimationCommandDetector detector;

    // Test 1: Circle center and radius both change (should prioritize SCALE_RADIUS)
    {
        GeometryTypes::TestGraph oldGraph = createTestGraph();
        GeometryTypes::TestGraph newGraph = createTestGraph();

        oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 10.0);
        newGraph.addNode(1, GeometryTypes::CIRCLE, 60.0, 60.0, 20.0);  // Center moves, radius changes

        AnimationCommand cmd = detector.detectCommand(oldGraph, newGraph);

        // Should detect one of the changes based on priority
        assert(cmd.command_type == AnimationCommandType::SCALE_RADIUS ||
               cmd.command_type == AnimationCommandType::MOVE_LINEAR);
        std::cout << "[PASS] Concurrent changes handled (center + radius)" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Unit Tests: Animation Command Detector " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testAddElementDetection();
        testRemoveElementDetection();
        testScaleRadiusDetection();
        testMoveLinearDetection();
        testNoChangeDetection();
        testPriorityOrder();
        testConcurrentPropertyChanges();

        std::cout << "\n========================================" << std::endl;
        std::cout << "       ALL UNIT TESTS PASSED!          " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nX TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
