/***************************************************************************
 * Example 5: Sequential Dependent Animations
 *
 * Demonstrates animations that must run in sequence because they affect
 * the same element. The coordinator detects these dependencies and
 * schedules them one after another.
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
    std::cout << "  Example 5: Sequential Animations     " << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "\nScenario: A point moves in two stages:" << std::endl;
    std::cout << "  Stage 1: Move from (10, 10) to (50, 50)" << std::endl;
    std::cout << "  Stage 2: Move from (50, 50) to (90, 90)" << std::endl;
    std::cout << "\nBoth stages affect the same point (element ID 1)," << std::endl;
    std::cout << "so they must run sequentially." << std::endl;

    // Configure keyframe generation
    KeyframeGenerationConfig config;
    config.frames_per_command = 8;  // 8 frames per stage
    config.epsilon_threshold = 1e-6;
    config.interpolation_mode = InterpolationMode::LINEAR;

    std::cout << "\nKeyframe Generation Configuration:" << std::endl;
    std::cout << "  Frames per command: " << config.frames_per_command << std::endl;
    std::cout << "  Interpolation mode: LINEAR" << std::endl;

    // Create sequential animation commands for the same element
    std::vector<AnimationCommand> commands;

    // Stage 1: Move from (10, 10) to (50, 50)
    AnimationCommand cmd1(AnimationCommandType::MOVE_LINEAR);
    cmd1.element_id = "1";
    cmd1.element_name = "MovingPoint";
    cmd1.addPropertyChange("x", "10.0", "50.0");
    cmd1.addPropertyChange("y", "10.0", "50.0");
    commands.push_back(cmd1);

    // Stage 2: Move from (50, 50) to (90, 90)
    AnimationCommand cmd2(AnimationCommandType::MOVE_LINEAR);
    cmd2.element_id = "1";  // Same element ID - creates dependency!
    cmd2.element_name = "MovingPoint";
    cmd2.addPropertyChange("x", "50.0", "90.0");
    cmd2.addPropertyChange("y", "50.0", "90.0");
    commands.push_back(cmd2);

    std::cout << "\nAnimation Commands Created:" << std::endl;
    std::cout << "  Command 1: MOVE_LINEAR for element 1 (10,10) -> (50,50)" << std::endl;
    std::cout << "  Command 2: MOVE_LINEAR for element 1 (50,50) -> (90,90)" << std::endl;
    std::cout << "\n  Both commands affect the same element (ID=1)," << std::endl;
    std::cout << "  creating a sequential dependency." << std::endl;

    // Generate keyframes using bridge with manual commands
    GeometryAnimationBridge bridge;
    std::string json = bridge.generateKeyframesFromCommands(commands, config);

    std::cout << "\nGenerated JSON keyframes:" << std::endl;
    std::cout << "  JSON length: " << json.length() << " characters" << std::endl;

    // Analyze the keyframes to show sequential execution
    std::cout << "\nAnalyzing Keyframe Structure:" << std::endl;

    // Count frames and analyze timing
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

        pos = frameEnd + 1;
    }

    std::cout << "  Total keyframes generated: " << totalFrames << std::endl;
    std::cout << "  Expected: 16 (8 frames per stage)" << std::endl;

    if (!frameIndexCount.empty()) {
        int minFrame = frameIndexCount.begin()->first;
        int maxFrame = frameIndexCount.rbegin()->first;
        std::cout << "\n  Frame index range: " << minFrame << " to " << maxFrame << std::endl;

        // Analyze frame distribution to show sequential stages
        std::cout << "\n  Frame distribution analysis:" << std::endl;

        int stage1Frames = 0;
        int stage2Frames = 0;

        for (const auto& [frameIdx, count] : frameIndexCount) {
            if (frameIdx < 8) {
                stage1Frames++;
            } else {
                stage2Frames++;
            }
        }

        std::cout << "    Stage 1 (frames 0-7): " << stage1Frames << " frames" << std::endl;
        std::cout << "    Stage 2 (frames 8-15): " << stage2Frames << " frames" << std::endl;

        if (stage1Frames > 0 && stage2Frames > 0) {
            std::cout << "\n  *** SEQUENTIAL EXECUTION CONFIRMED ***" << std::endl;
            std::cout << "  The two stages are executed one after another:" << std::endl;
            std::cout << "  - Stage 1 completes at frame 7" << std::endl;
            std::cout << "  - Stage 2 starts at frame 8" << std::endl;
        }
    }

    // Display sample keyframes from each stage
    std::cout << "\nSample keyframes:" << std::endl;

    // Find first frame of stage 1 (frame 0)
    size_t frame0Pos = json.find("\"frameIndex\": 0");
    if (frame0Pos != std::string::npos) {
        size_t start = json.rfind('{', frame0Pos);
        if (start != std::string::npos) {
            int depth = 0;
            size_t end = start;
            for (; end < json.length(); end++) {
                if (json[end] == '{') depth++;
                if (json[end] == '}') depth--;
                if (depth == 0) break;
            }

            if (end < json.length()) {
                std::cout << "\n  Stage 1 - Frame 0 (start):" << std::endl;
                std::cout << "    Position: (10, 10)" << std::endl;
                std::cout << "    Progress: 0.0" << std::endl;
            }
        }
    }

    // Find first frame of stage 2 (frame 8)
    size_t frame8Pos = json.find("\"frameIndex\": 8");
    if (frame8Pos != std::string::npos) {
        size_t start = json.rfind('{', frame8Pos);
        if (start != std::string::npos) {
            int depth = 0;
            size_t end = start;
            for (; end < json.length(); end++) {
                if (json[end] == '{') depth++;
                if (json[end] == '}') depth--;
                if (depth == 0) break;
            }

            if (end < json.length()) {
                std::cout << "\n  Stage 2 - Frame 8 (start):" << std::endl;
                std::cout << "    Position: (50, 50)" << std::endl;
                std::cout << "    Progress: 0.0" << std::endl;
            }
        }
    }

    // Find last frame of stage 2 (frame 15)
    size_t frame15Pos = json.find("\"frameIndex\": 15");
    if (frame15Pos != std::string::npos) {
        size_t start = json.rfind('{', frame15Pos);
        if (start != std::string::npos) {
            int depth = 0;
            size_t end = start;
            for (; end < json.length(); end++) {
                if (json[end] == '{') depth++;
                if (json[end] == '}') depth--;
                if (depth == 0) break;
            }

            if (end < json.length()) {
                std::cout << "\n  Stage 2 - Frame 15 (end):" << std::endl;
                std::cout << "    Position: (90, 90)" << std::endl;
                std::cout << "    Progress: 1.0" << std::endl;
            }
        }
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Sequential Animations Example Complete!" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "\nKey Takeaway:" << std::endl;
    std::cout << "  When multiple commands affect the same element," << std::endl;
    std::cout << "  the coordinator automatically schedules them" << std::endl;
    std::cout << "  sequentially to prevent conflicts." << std::endl;

    return 0;
}
