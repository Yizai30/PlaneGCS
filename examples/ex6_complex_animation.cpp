/***************************************************************************
 * Example 6: Complex Multi-Step Animation
 *
 * Demonstrates a complex animation scenario with multiple elements,
 * different animation types, and mixed sequential/independent execution.
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
    std::cout << "  Example 6: Complex Multi-Step        " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "\nScenario: A multi-stage construction animation:" << std::endl;
    std::cout << "\n  Stage 1: Create circle at (50, 50) with radius 10" << std::endl;
    std::cout << "  Stage 2: Scale circle to radius 25" << std::endl;
    std::cout << "  Stage 3: Add point 1 at (30, 50)" << std::endl;
    std::cout << "  Stage 4: Add point 2 at (70, 50)" << std::endl;
    std::cout << "  Stage 5: Move point 1 to (20, 30)" << std::endl;
    std::cout << "  Stage 6: Move point 2 to (80, 70)" << std::endl;

    std::cout << "\nThis demonstrates:" << std::endl;
    std::cout << "  - ADD_ELEMENT commands" << std::endl;
    std::cout << "  - SCALE_RADIUS command" << std::endl;
    std::cout << "  - MOVE_LINEAR commands" << std::endl;
    std::cout << "  - Sequential dependencies (same element)" << std::endl;
    std::cout << "  - Independent operations (different elements)" << std::endl;

    // Configure keyframe generation
    KeyframeGenerationConfig config;
    config.frames_per_command = 5;  // 5 frames per stage for brevity
    config.epsilon_threshold = 1e-6;
    config.interpolation_mode = InterpolationMode::LINEAR;

    std::cout << "\nKeyframe Generation Configuration:" << std::endl;
    std::cout << "  Frames per command: " << config.frames_per_command << std::endl;
    std::cout << "  Interpolation mode: LINEAR" << std::endl;

    // Create complex multi-step animation commands
    std::vector<AnimationCommand> commands;

    // Stage 1: Create circle (simulate by showing initial state)
    AnimationCommand cmd1(AnimationCommandType::ADD_ELEMENT);
    cmd1.element_id = "circle1";
    cmd1.element_name = "MainCircle";
    cmd1.addPropertyChange("x", "50.0", "50.0");
    cmd1.addPropertyChange("y", "50.0", "50.0");
    cmd1.addPropertyChange("radius", "10.0", "10.0");
    commands.push_back(cmd1);

    // Stage 2: Scale circle
    AnimationCommand cmd2(AnimationCommandType::SCALE_RADIUS);
    cmd2.element_id = "circle1";
    cmd2.element_name = "MainCircle";
    cmd2.addPropertyChange("radius", "10.0", "25.0");
    commands.push_back(cmd2);

    // Stage 3: Add point 1
    AnimationCommand cmd3(AnimationCommandType::ADD_ELEMENT);
    cmd3.element_id = "point1";
    cmd3.element_name = "Point1";
    cmd3.addPropertyChange("x", "30.0", "30.0");
    cmd3.addPropertyChange("y", "50.0", "50.0");
    commands.push_back(cmd3);

    // Stage 4: Add point 2
    AnimationCommand cmd4(AnimationCommandType::ADD_ELEMENT);
    cmd4.element_id = "point2";
    cmd4.element_name = "Point2";
    cmd4.addPropertyChange("x", "70.0", "70.0");
    cmd4.addPropertyChange("y", "50.0", "50.0");
    commands.push_back(cmd4);

    // Stage 5: Move point 1
    AnimationCommand cmd5(AnimationCommandType::MOVE_LINEAR);
    cmd5.element_id = "point1";
    cmd5.element_name = "Point1";
    cmd5.addPropertyChange("x", "30.0", "20.0");
    cmd5.addPropertyChange("y", "50.0", "30.0");
    commands.push_back(cmd5);

    // Stage 6: Move point 2
    AnimationCommand cmd6(AnimationCommandType::MOVE_LINEAR);
    cmd6.element_id = "point2";
    cmd6.element_name = "Point2";
    cmd6.addPropertyChange("x", "70.0", "80.0");
    cmd6.addPropertyChange("y", "50.0", "70.0");
    commands.push_back(cmd6);

    std::cout << "\nAnimation Commands Created:" << std::endl;
    for (size_t i = 0; i < commands.size(); i++) {
        std::cout << "  Stage " << (i + 1) << ": " << toString(commands[i].command_type)
                  << " for " << commands[i].element_name
                  << " (ID: " << commands[i].element_id << ")" << std::endl;
    }

    // Generate keyframes using bridge
    GeometryAnimationBridge bridge;
    std::string json = bridge.generateKeyframesFromCommands(commands, config);

    std::cout << "\nGenerated JSON keyframes:" << std::endl;
    std::cout << "  JSON length: " << json.length() << " characters" << std::endl;

    // Analyze the keyframes
    std::cout << "\nAnalyzing Animation Structure:" << std::endl;

    std::map<std::string, std::vector<int>> elementFrames;
    std::map<int, int> frameIndexCount;
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
                size_t idxColon = json.find(":", frameStart);
                size_t idxComma = json.find(",", frameStart);
                if (idxComma != std::string::npos && idxComma < frameEnd) {
                    std::string frameStr = json.substr(idxColon + 1, idxComma - idxColon - 1);
                    try {
                        int frameIdx = std::stoi(frameStr);
                        elementFrames[elemId].push_back(frameIdx);
                    } catch (...) {
                        // Skip invalid conversions
                    }
                }
            }
        }

        pos = frameEnd + 1;
    }

    std::cout << "  Total keyframes generated: " << totalFrames << std::endl;
    std::cout << "  Expected: 30 (6 stages x 5 frames each)" << std::endl;

    if (!frameIndexCount.empty()) {
        int minFrame = frameIndexCount.begin()->first;
        int maxFrame = frameIndexCount.rbegin()->first;
        std::cout << "\n  Frame index range: " << minFrame << " to " << maxFrame << std::endl;
    }

    std::cout << "\n  Frames per element:" << std::endl;
    for (const auto& [elemId, frames] : elementFrames) {
        std::cout << "    " << elemId << ": " << frames.size() << " frames";
        if (!frames.empty()) {
            std::cout << " (frames " << frames.front() << "-" << frames.back() << ")";
        }
        std::cout << std::endl;
    }

    // Analyze execution pattern
    std::cout << "\nExecution Pattern Analysis:" << std::endl;

    // Check for overlaps (independent elements animating simultaneously)
    std::map<int, int> elementCountAtFrame;
    for (const auto& [elemId, frames] : elementFrames) {
        for (int frame : frames) {
            elementCountAtFrame[frame]++;
        }
    }

    int maxSimultaneous = 0;
    for (const auto& [frame, count] : elementCountAtFrame) {
        if (count > maxSimultaneous) {
            maxSimultaneous = count;
        }
    }

    std::cout << "  Maximum elements animating simultaneously: " << maxSimultaneous << std::endl;

    if (maxSimultaneous > 1) {
        std::cout << "\n  *** MIXED EXECUTION PATTERN ***" << std::endl;
        std::cout << "  Some stages run sequentially (same element)," << std::endl;
        std::cout << "  others could run independently (different elements)." << std::endl;
    } else {
        std::cout << "\n  *** SEQUENTIAL EXECUTION ***" << std::endl;
        std::cout << "  All stages execute one after another." << std::endl;
    }

    // Show stage breakdown
    std::cout << "\nStage Breakdown:" << std::endl;
    std::cout << "\n  Stage 1: ADD_ELEMENT (circle)" << std::endl;
    std::cout << "    Creates the main circle" << std::endl;
    std::cout << "    Frames: 0-4" << std::endl;

    std::cout << "\n  Stage 2: SCALE_RADIUS" << std::endl;
    std::cout << "    Scales circle from radius 10 to 25" << std::endl;
    std::cout << "    Sequential dependency (same element)" << std::endl;
    std::cout << "    Frames: 5-9" << std::endl;

    std::cout << "\n  Stage 3 & 4: ADD_ELEMENT (points)" << std::endl;
    std::cout << "    Adds two points to the scene" << std::endl;
    std::cout << "    Independent operations (different elements)" << std::endl;
    std::cout << "    Frames: 10-19" << std::endl;

    std::cout << "\n  Stage 5 & 6: MOVE_LINEAR" << std::endl;
    std::cout << "    Moves points to new positions" << std::endl;
    std::cout << "    Could be independent (different elements)" << std::endl;
    std::cout << "    Frames: 20-29" << std::endl;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Complex Animation Example Complete!   " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "\nKey Takeaways:" << std::endl;
    std::cout << "  1. The coordinator handles mixed execution patterns" << std::endl;
    std::cout << "  2. Same-element operations are always sequential" << std::endl;
    std::cout << "  3. Different-element operations can be independent" << std::endl;
    std::cout << "  4. Complex animations are broken into manageable stages" << std::endl;

    return 0;
}
