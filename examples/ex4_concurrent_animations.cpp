/***************************************************************************
 * Example 4: Concurrent Independent Animations
 *
 * Demonstrates multiple animations running simultaneously on different
 * elements. The coordinator detects that these commands are independent
 * and schedules them to run at the same time.
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
    std::cout << "  Example 4: Concurrent Animations     " << std::endl;
    std::cout << "========================================" << std::endl;

    // Create geometry graphs
    GeometryTypes::TestGraph oldGraph;
    GeometryTypes::TestGraph newGraph;

    // Animation 1: Point 1 moves horizontally
    // From (10, 50) to (100, 50)
    oldGraph.addNode(1, GeometryTypes::POINT, 10.0, 50.0);
    newGraph.addNode(1, GeometryTypes::POINT, 100.0, 50.0);

    // Animation 2: Point 2 moves vertically
    // From (50, 10) to (50, 100)
    oldGraph.addNode(2, GeometryTypes::POINT, 50.0, 10.0);
    newGraph.addNode(2, GeometryTypes::POINT, 50.0, 100.0);

    // Animation 3: Circle scales up
    // Center at (150, 150), radius from 10 to 30
    oldGraph.addNode(3, GeometryTypes::CIRCLE, 150.0, 150.0, 10.0);
    newGraph.addNode(3, GeometryTypes::CIRCLE, 150.0, 150.0, 30.0);

    // IMPORTANT: For concurrent animation demonstration, we need to manually
    // create multiple AnimationCommand objects for different elements.
    // The detector will only pick the highest priority change, so let's
    // create commands directly to show how the coordinator handles them.
    std::cout << "\nNote: Using direct command creation to demonstrate coordination." << std::endl;

    std::cout << "\nAnimation Setup:" << std::endl;
    std::cout << "  Animation 1: Point 1 moves horizontally from (10, 50) to (100, 50)" << std::endl;
    std::cout << "  Animation 2: Point 2 moves vertically from (50, 10) to (50, 100)" << std::endl;
    std::cout << "  Animation 3: Circle scales up at (150, 150), radius 10 to 30" << std::endl;

    // Configure keyframe generation
    KeyframeGenerationConfig config;
    config.frames_per_command = 10;  // 10 frames per animation
    config.epsilon_threshold = 1e-6;
    config.interpolation_mode = InterpolationMode::LINEAR;

    std::cout << "\nKeyframe Generation Configuration:" << std::endl;
    std::cout << "  Frames per command: " << config.frames_per_command << std::endl;
    std::cout << "  Interpolation mode: LINEAR" << std::endl;

    // Create animation commands manually to demonstrate coordination
    std::vector<AnimationCommand> commands;

    AnimationCommand cmd1(AnimationCommandType::MOVE_LINEAR);
    cmd1.element_id = "1";
    cmd1.element_name = "Point1";
    cmd1.addPropertyChange("x", "10.0", "100.0");
    cmd1.addPropertyChange("y", "50.0", "50.0");
    commands.push_back(cmd1);

    AnimationCommand cmd2(AnimationCommandType::MOVE_LINEAR);
    cmd2.element_id = "2";
    cmd2.element_name = "Point2";
    cmd2.addPropertyChange("x", "50.0", "50.0");
    cmd2.addPropertyChange("y", "10.0", "100.0");
    commands.push_back(cmd2);

    AnimationCommand cmd3(AnimationCommandType::SCALE_RADIUS);
    cmd3.element_id = "3";
    cmd3.element_name = "Circle1";
    cmd3.addPropertyChange("radius", "10.0", "30.0");
    commands.push_back(cmd3);

    // Generate keyframes using bridge with manual commands
    GeometryAnimationBridge bridge;
    std::string json = bridge.generateKeyframesFromCommands(commands, config);

    std::cout << "\nGenerated JSON keyframes:" << std::endl;
    std::cout << "  JSON length: " << json.length() << " characters" << std::endl;

    // Analyze the keyframes to show concurrency
    std::cout << "\nAnalyzing Keyframe Structure:" << std::endl;

    // Count frames per element
    std::map<std::string, int> elementFrameCount;
    std::map<int, int> frameIndexCount;

    // Simple parsing to extract frame indices and element IDs
    size_t pos = 0;
    int totalFrames = 0;

    while (pos < json.length()) {
        size_t frameStart = json.find("\"frameIndex\":", pos);
        if (frameStart == std::string::npos) break;

        size_t frameEnd = json.find("}", frameStart);
        if (frameEnd == std::string::npos) break;

        // Extract frame index
        size_t colonPos = json.find(":", frameStart);
        size_t commaPos = json.find(",", frameStart);
        if (commaPos != std::string::npos && commaPos < frameEnd) {
            std::string frameStr = json.substr(colonPos + 1, commaPos - colonPos - 1);
            try {
                int frameIdx = std::stoi(frameStr);
                frameIndexCount[frameIdx]++;
                totalFrames++;
            } catch (...) {
                // Skip invalid conversions
            }
        }

        // Extract element ID
        size_t elemStart = json.find("\"elementId\":", frameStart);
        if (elemStart != std::string::npos && elemStart < frameEnd) {
            size_t elemColon = json.find(":", elemStart);
            size_t elemQuote1 = json.find("\"", elemColon);
            size_t elemQuote2 = json.find("\"", elemQuote1 + 1);
            if (elemQuote1 != std::string::npos && elemQuote2 != std::string::npos) {
                std::string elemId = json.substr(elemQuote1 + 1, elemQuote2 - elemQuote1 - 1);
                elementFrameCount[elemId]++;
            }
        }

        pos = frameEnd + 1;
    }

    std::cout << "  Total keyframes generated: " << totalFrames << std::endl;
    std::cout << "\n  Frames per element:" << std::endl;
    for (const auto& [elemId, count] : elementFrameCount) {
        std::cout << "    Element " << elemId << ": " << count << " frames" << std::endl;
    }

    std::cout << "\n  Frame index distribution:" << std::endl;
    std::cout << "    Unique frame indices: " << frameIndexCount.size() << std::endl;
    if (frameIndexCount.size() > 0) {
        int minFrame = frameIndexCount.begin()->first;
        int maxFrame = frameIndexCount.rbegin()->first;
        std::cout << "    Frame range: " << minFrame << " to " << maxFrame << std::endl;

        // Check if frames overlap (indicating concurrent execution)
        int maxFramesAtIndex = 0;
        for (const auto& [frameIdx, count] : frameIndexCount) {
            if (count > maxFramesAtIndex) {
                maxFramesAtIndex = count;
            }
        }

        std::cout << "    Maximum animations at same frame: " << maxFramesAtIndex << std::endl;

        if (maxFramesAtIndex > 1) {
            std::cout << "\n  *** CONCURRENT ANIMATION DETECTED ***" << std::endl;
            std::cout << "  Multiple elements animate at the same frame indices," << std::endl;
            std::cout << "  indicating the coordinator scheduled them to run simultaneously." << std::endl;
        } else {
            std::cout << "\n  *** SEQUENTIAL ANIMATION ***" << std::endl;
            std::cout << "  Animations run one after another." << std::endl;
        }
    }

    // Display a sample keyframe from each element
    std::cout << "\nSample keyframes (first frame for each element):" << std::endl;
    std::set<std::string> seenElements;
    pos = 0;
    int sampleCount = 0;

    while (pos < json.length() && sampleCount < 3) {
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
            std::string frame = json.substr(start, end - start + 1);

            // Extract element ID
            size_t elemStart = frame.find("\"elementId\":");
            if (elemStart != std::string::npos) {
                size_t elemColon = frame.find(":", elemStart);
                size_t elemQuote1 = frame.find("\"", elemColon);
                size_t elemQuote2 = frame.find("\"", elemQuote1 + 1);
                if (elemQuote1 != std::string::npos && elemQuote2 != std::string::npos) {
                    std::string elemId = frame.substr(elemQuote1 + 1, elemQuote2 - elemQuote1 - 1);

                    if (seenElements.find(elemId) == seenElements.end()) {
                        std::cout << "\n  Element " << elemId << " - Frame 0:" << std::endl;
                        // Pretty print a portion of the frame
                        size_t firstNewline = frame.find("\\n");
                        if (firstNewline != std::string::npos) {
                            std::cout << "    " << frame.substr(0, firstNewline) << std::endl;
                        } else {
                            std::cout << "    " << frame.substr(0, std::min(size_t(100), frame.length())) << "..." << std::endl;
                        }
                        seenElements.insert(elemId);
                        sampleCount++;
                    }
                }
            }
            pos = end + 1;
        } else {
            break;
        }
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Concurrent Animations Example Complete!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
