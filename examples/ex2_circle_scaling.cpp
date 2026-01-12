/***************************************************************************
 * Example 2: Circle Radius Scaling
 *
 * Demonstrates keyframe generation for a circle scaling its radius
 ***************************************************************************/

#include "../src/AnimationCommand.h"
#include "../src/KeyframeGenerator.h"
#include "../src/GeometryAnimationBridge.h"
#include <iostream>

using namespace GCS;

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
    std::cout << "Example 2: Circle Radius Scaling" << std::endl;
    std::cout << "====================================" << std::endl;

    // Create two graphs: circle scales from radius 5 to radius 15
    // Center stays fixed at (50, 50)
    GeometryTypes::TestGraph oldGraph;
    oldGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 5.0);

    GeometryTypes::TestGraph newGraph;
    newGraph.addNode(1, GeometryTypes::CIRCLE, 50.0, 50.0, 15.0);

    std::cout << "Old radius: 5.0 at center (50, 50)" << std::endl;
    std::cout << "New radius: 15.0 at center (50, 50)" << std::endl;
    std::cout << "Scaling factor: 3.0x" << std::endl;

    // Generate keyframes
    GeometryAnimationBridge bridge;
    KeyframeGenerationConfig config;
    config.frames_per_command = 8;

    std::string json = bridge.generateAnimationKeyframes(oldGraph, newGraph, config);

    std::cout << "\nGenerated keyframes:" << std::endl;
    std::cout << json << std::endl;

    return 0;
}
