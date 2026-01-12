/***************************************************************************
 * Example 1: Simple Point Movement
 *
 * Demonstrates basic keyframe generation for a point moving in a straight line
 ***************************************************************************/

#include "../src/AnimationCommand.h"
#include "../src/KeyframeGenerator.h"
#include "../src/GeometryAnimationBridge.h"
#include <iostream>

using namespace GCS;

// Mock graph implementations (simplified for this example)
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
        const std::vector<std::unique_ptr<GeometryNode>>& getNodes() const override { return nodes_; }
    };
}

int main() {
    std::cout << "Example 1: Simple Point Movement" << std::endl;
    std::cout << "===================================" << std::endl;

    // Create two graphs: point moves from (0,0) to (100, 50)
    GeometryTypes::TestGraph oldGraph;
    oldGraph.addNode(1, GeometryTypes::POINT, 0.0, 0.0);

    GeometryTypes::TestGraph newGraph;
    newGraph.addNode(1, GeometryTypes::POINT, 100.0, 50.0);

    std::cout << "Old position: (0, 0)" << std::endl;
    std::cout << "New position: (100, 50)" << std::endl;

    // Generate keyframes
    GeometryAnimationBridge bridge;
    KeyframeGenerationConfig config;
    config.frames_per_command = 5;  // 5 frames for this animation

    std::string json = bridge.generateAnimationKeyframes(oldGraph, newGraph, config);

    std::cout << "\nGenerated keyframes:" << std::endl;
    std::cout << json << std::endl;

    return 0;
}
