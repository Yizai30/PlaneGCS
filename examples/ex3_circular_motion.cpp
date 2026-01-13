/***************************************************************************
 * Example 3: Circular Motion Animation
 *
 * Demonstrates a point moving along a circular path around a center point.
 * This example shows how the keyframe generation system handles circular
 * motion interpolation.
 ***************************************************************************/

#include "../src/GeometryAnimationBridge.h"
#include <iostream>
#include <iomanip>

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

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Example 3: Circular Motion Animation  " << std::endl;
    std::cout << "========================================" << std::endl;

    // Create geometry graphs
    GeometryTypes::TestGraph oldGraph;
    GeometryTypes::TestGraph newGraph;

    // Center point (fixed)
    oldGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);
    newGraph.addNode(1, GeometryTypes::POINT, 50.0, 50.0);

    // Moving point on circle
    // Starting at angle 0 degrees (point at rightmost position)
    double radius = 30.0;
    double start_angle = 0.0;
    double end_angle = 3.14159;  // 180 degrees (half circle)

    double start_x = 50.0 + radius * std::cos(start_angle);
    double start_y = 50.0 + radius * std::sin(start_angle);

    double end_x = 50.0 + radius * std::cos(end_angle);
    double end_y = 50.0 + radius * std::sin(end_angle);

    oldGraph.addNode(2, GeometryTypes::POINT, start_x, start_y);
    newGraph.addNode(2, GeometryTypes::POINT, end_x, end_y);

    std::cout << "\nCircular Motion Configuration:" << std::endl;
    std::cout << "  Center: (50.0, 50.0)" << std::endl;
    std::cout << "  Radius: " << radius << std::endl;
    std::cout << "  Start Angle: " << start_angle << " radians (0 degrees)" << std::endl;
    std::cout << "  End Angle: " << end_angle << " radians (180 degrees)" << std::endl;
    std::cout << "  Start Position: (" << start_x << ", " << start_y << ")" << std::endl;
    std::cout << "  End Position: (" << end_x << ", " << end_y << ")" << std::endl;

    // Configure keyframe generation
    KeyframeGenerationConfig config;
    config.frames_per_command = 12;  // 12 frames for smooth half-circle
    config.epsilon_threshold = 1e-6;
    config.interpolation_mode = InterpolationMode::LINEAR;

    std::cout << "\nKeyframe Generation Configuration:" << std::endl;
    std::cout << "  Frames per command: " << config.frames_per_command << std::endl;
    std::cout << "  Interpolation mode: LINEAR" << std::endl;

    // Generate keyframes using bridge
    GeometryAnimationBridge bridge;
    std::string json = bridge.generateAnimationKeyframes(oldGraph, newGraph, config);

    std::cout << "\nGenerated JSON keyframes:" << std::endl;
    std::cout << "  JSON length: " << json.length() << " characters" << std::endl;

    // Display first few keyframes
    std::cout << "\nFirst 3 keyframes (sample):" << std::endl;
    size_t pos = 0;
    int frame_count = 0;
    int bracket_count = 0;

    // Simple extraction of first 3 complete keyframe objects
    while (pos < json.length() && frame_count < 3) {
        size_t start = json.find('{', pos);
        if (start == std::string::npos) break;

        int depth = 0;
        size_t end = start;
        for (; end < json.length(); end++) {
            if (json[end] == '{') depth++;
            if (json[end] == '}') depth--;
            if (depth == 0) break;
        }

        if (end < json.length()) {
            std::cout << "\n--- Frame " << frame_count << " ---" << std::endl;
            std::cout << json.substr(start, end - start + 1) << std::endl;
            frame_count++;
            pos = end + 1;
        } else {
            break;
        }
    }

    std::cout << "\n...\n" << std::endl;

    // Note about circular motion
    std::cout << "\nNote:" << std::endl;
    std::cout << "  The current implementation uses linear interpolation between" << std::endl;
    std::cout << "  the start and end positions. For true circular motion along" << std::endl;
    std::cout << "  the arc path, you would need to add intermediate keyframes or" << std::endl;
    std::cout << "  implement specialized circular interpolation (future enhancement)." << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Circular Motion Example Complete!      " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
