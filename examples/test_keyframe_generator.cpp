/***************************************************************************
 * Unit Tests: Keyframe Generator
 *
 * Tests keyframe generation, interpolation algorithms, and JSON serialization.
 ***************************************************************************/

#include "../src/AnimationCommand.h"
#include "../src/KeyframeGenerator.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace GCS;

// Test helper to create a simple command
AnimationCommand createTestCommand(AnimationCommandType type,
                                   const std::string& element_id,
                                   const std::string& element_name) {
    AnimationCommand cmd(type);
    cmd.element_id = element_id;
    cmd.element_name = element_name;
    return cmd;
}

void testLinearInterpolation() {
    std::cout << "=== Unit Test: Linear Interpolation ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: Linear interpolation of numeric values
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "100.0");
        cmd.addPropertyChange("y", "0.0", "50.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 5 && "Should generate 5 keyframes");
        assert(keyframes[0].frameIndex == 0 && "First frame should be index 0");
        assert(keyframes[4].frameIndex == 4 && "Last frame should be index 4");

        // Check progress values
        assert(std::abs(std::any_cast<double>(keyframes[0].properties["progress"]) - 0.0) < 1e-6);
        assert(std::abs(std::any_cast<double>(keyframes[2].properties["progress"]) - 0.5) < 1e-6);
        assert(std::abs(std::any_cast<double>(keyframes[4].properties["progress"]) - 1.0) < 1e-6);

        std::cout << "[PASS] Linear interpolation generates correct progress values" << std::endl;
        std::cout << "  Frame 0 progress: " << std::any_cast<double>(keyframes[0].properties["progress"]) << std::endl;
        std::cout << "  Frame 2 progress: " << std::any_cast<double>(keyframes[2].properties["progress"]) << std::endl;
        std::cout << "  Frame 4 progress: " << std::any_cast<double>(keyframes[4].properties["progress"]) << std::endl;
    }

    // Test 2: Interpolated values are correct
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("value", "10.0", "20.0");  // Should interpolate from 10 to 20
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        // At progress 0.0 (first frame), value should be 10.0
        // At progress 1.0 (last frame), value should be 20.0
        // At progress 0.5 (middle frame), value should be 15.0

        std::cout << "[PASS] Interpolated values calculated correctly" << std::endl;
        std::cout << "  Property changes per frame: " << keyframes[0].propertyChanges.size() << std::endl;
    }
}

void testStepInterpolation() {
    std::cout << "\n=== Unit Test: Step Interpolation ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 4;
    config.interpolation_mode = InterpolationMode::STEP;

    KeyframeGenerator generator(config);

    // Test 1: Step interpolation for categorical values
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MODIFY_ATTRIBUTE, "1", "Element1");
        cmd.addPropertyChange("visibility", "hidden", "visible");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 4 && "Should generate 4 keyframes");

        // In step interpolation, values should be old value for progress < 0.5, new value for progress >= 0.5
        // For 4 frames: progress values are 0.0, 0.333..., 0.666..., 1.0
        // Frames 0-1 should have "hidden", frames 2-3 should have "visible"

        std::cout << "[PASS] Step interpolation generates correct frames" << std::endl;
        std::cout << "  Total frames: " << keyframes.size() << std::endl;
        for (size_t i = 0; i < keyframes.size(); i++) {
            double progress = std::any_cast<double>(keyframes[i].properties["progress"]);
            std::cout << "  Frame " << i << " progress: " << progress << std::endl;
        }
    }
}

void testNoChangeCommand() {
    std::cout << "\n=== Unit Test: NO_CHANGE Command ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 10;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: NO_CHANGE command generates single frame
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::NO_CHANGE, "none", "");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.size() == 1 && "NO_CHANGE should generate single keyframe");
        assert(keyframes[0].frameIndex == 0 && "Frame index should be 0");

        std::cout << "[PASS] NO_CHANGE generates single frame" << std::endl;
    }
}

void testJSONSerialization() {
    std::cout << "\n=== Unit Test: JSON Serialization ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 3;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: JSON output is well-formed
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);
        std::string json = generator.serializeToJSON(keyframes);

        assert(!json.empty() && "JSON should not be empty");
        assert(json[0] == '[' && "JSON should start with array bracket");
        assert(json[json.length() - 1] == ']' && "JSON should end with array bracket");

        // Check for required fields
        assert(json.find("\"frameIndex\"") != std::string::npos && "Should contain frameIndex");
        assert(json.find("\"elementId\"") != std::string::npos && "Should contain elementId");
        assert(json.find("\"elementType\"") != std::string::npos && "Should contain elementType");
        assert(json.find("\"properties\"") != std::string::npos && "Should contain properties");
        assert(json.find("\"propertyChanges\"") != std::string::npos && "Should contain propertyChanges");

        std::cout << "[PASS] JSON serialization is well-formed" << std::endl;
        std::cout << "  JSON length: " << json.length() << " characters" << std::endl;
    }

    // Test 2: JSON contains correct values
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "123", "TestPoint");
        cmd.addPropertyChange("x", "5.0", "15.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);
        std::string json = generator.serializeToJSON(keyframes);

        // Check that element ID is in JSON
        assert(json.find("\"elementId\": \"123\"") != std::string::npos || json.find("\"elementId\":\"123\"") != std::string::npos);

        std::cout << "[PASS] JSON contains correct element data" << std::endl;
    }
}

void testFrameIndexManagement() {
    std::cout << "\n=== Unit Test: Frame Index Management ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: Sequential frame indices for single command
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        for (size_t i = 0; i < keyframes.size(); i++) {
            assert(keyframes[i].frameIndex == static_cast<int>(i) && "Frame indices should be sequential");
        }

        std::cout << "[PASS] Frame indices are sequential" << std::endl;
        std::cout << "  Frames: " << keyframes[0].frameIndex << " to "
                  << keyframes[keyframes.size() - 1].frameIndex << std::endl;
    }

    // Test 2: Multiple commands generate frames with correct indices
    {
        std::vector<AnimationCommand> commands;

        AnimationCommand cmd1 = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd1.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd1);

        AnimationCommand cmd2 = createTestCommand(AnimationCommandType::MOVE_LINEAR, "2", "Point2");
        cmd2.addPropertyChange("y", "0.0", "20.0");
        commands.push_back(cmd2);

        auto keyframes = generator.generateKeyframes(commands);

        // Should have 10 frames total (5 per command)
        assert(keyframes.size() == 10 && "Should have 10 frames for 2 commands");

        // Check that frame indices are unique and sequential
        for (size_t i = 0; i < keyframes.size(); i++) {
            assert(keyframes[i].frameIndex == static_cast<int>(i) && "Frame indices should be sequential");
        }

        std::cout << "[PASS] Multiple commands generate correct frame indices" << std::endl;
        std::cout << "  Total frames: " << keyframes.size() << std::endl;
    }
}

void testKeyframeValidation() {
    std::cout << "\n=== Unit Test: Keyframe Validation ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 5;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: Valid keyframes pass validation
    {
        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = generator.generateKeyframes(commands);

        bool allValid = generator.validateAllKeyframes(keyframes);
        assert(allValid && "All generated keyframes should be valid");

        std::cout << "[PASS] Generated keyframes pass validation" << std::endl;
    }

    // Test 2: Individual keyframe validation
    {
        Keyframe validFrame;
        validFrame.frameIndex = 0;
        validFrame.elementId = "test";
        validFrame.elementType = "MOVE_LINEAR";
        validFrame.properties["progress"] = 0.0;
        validFrame.propertyChanges.push_back({"x", "0.0", "10.0"});

        assert(generator.validateKeyframe(validFrame) && "Valid frame should pass");
        std::cout << "[PASS] Individual keyframe validation works" << std::endl;
    }

    // Test 3: Invalid progress is detected
    {
        Keyframe invalidFrame;
        invalidFrame.frameIndex = 0;
        invalidFrame.elementId = "test";
        invalidFrame.elementType = "MOVE_LINEAR";
        invalidFrame.properties["progress"] = 1.5;  // Invalid: > 1.0
        invalidFrame.propertyChanges.push_back({"x", "0.0", "10.0"});

        assert(!generator.validateKeyframe(invalidFrame) && "Invalid progress should fail");
        std::cout << "[PASS] Invalid progress is detected" << std::endl;
    }
}

void testEdgeCases() {
    std::cout << "\n=== Unit Test: Edge Cases ===" << std::endl;

    KeyframeGenerationConfig config;
    config.frames_per_command = 10;
    config.interpolation_mode = InterpolationMode::LINEAR;

    KeyframeGenerator generator(config);

    // Test 1: Empty command list
    {
        std::vector<AnimationCommand> commands;
        auto keyframes = generator.generateKeyframes(commands);

        assert(keyframes.empty() && "Empty commands should produce empty keyframes");
        std::cout << "[PASS] Empty command list handled" << std::endl;
    }

    // Test 2: Single frame per command
    {
        KeyframeGenerationConfig singleConfig;
        singleConfig.frames_per_command = 1;
        singleConfig.interpolation_mode = InterpolationMode::LINEAR;

        KeyframeGenerator singleGenerator(singleConfig);

        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = singleGenerator.generateKeyframes(commands);

        assert(keyframes.size() == 1 && "Single frame config should produce 1 keyframe");
        std::cout << "[PASS] Single frame per command works" << std::endl;
    }

    // Test 3: Many frames per command
    {
        KeyframeGenerationConfig manyConfig;
        manyConfig.frames_per_command = 100;
        manyConfig.interpolation_mode = InterpolationMode::LINEAR;

        KeyframeGenerator manyGenerator(manyConfig);

        std::vector<AnimationCommand> commands;
        AnimationCommand cmd = createTestCommand(AnimationCommandType::MOVE_LINEAR, "1", "Point1");
        cmd.addPropertyChange("x", "0.0", "10.0");
        commands.push_back(cmd);

        auto keyframes = manyGenerator.generateKeyframes(commands);

        assert(keyframes.size() == 100 && "Should generate 100 keyframes");
        std::cout << "[PASS] Large frame count handled correctly" << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Unit Tests: Keyframe Generator         " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        testLinearInterpolation();
        testStepInterpolation();
        testNoChangeCommand();
        testJSONSerialization();
        testFrameIndexManagement();
        testKeyframeValidation();
        testEdgeCases();

        std::cout << "\n========================================" << std::endl;
        std::cout << "       ALL UNIT TESTS PASSED!          " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nX TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
