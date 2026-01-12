/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Keyframe Generation Implementation
 *
 * Implements generation of animation keyframes from animation commands
 * with support for numeric and categorical interpolation.
 ***************************************************************************/

#include "KeyframeGenerator.h"
#include <sstream>
#include <cmath>

namespace GCS {

std::vector<Keyframe> KeyframeGenerator::generateKeyframes(
    const std::vector<AnimationCommand>& commands) const {

    std::vector<Keyframe> all_keyframes;
    int current_frame_index = 0;

    for (const auto& command : commands) {
        auto command_keyframes = generateKeyframesForCommand(command, current_frame_index);
        all_keyframes.insert(all_keyframes.end(),
                           command_keyframes.begin(),
                           command_keyframes.end());

        // Update frame index for next command
        if (!command_keyframes.empty()) {
            current_frame_index = command_keyframes.back().frameIndex + 1;
        }
    }

    return all_keyframes;
}

std::vector<Keyframe> KeyframeGenerator::generateKeyframesForCommand(
    const AnimationCommand& command,
    int start_frame) const {

    std::vector<Keyframe> keyframes;

    // Handle NO_CHANGE command - single frame
    if (command.command_type == AnimationCommandType::NO_CHANGE) {
        Keyframe kf(start_frame, "none", "none");
        kf.setProperty("action", std::string("no_change"));
        keyframes.push_back(kf);
        return keyframes;
    }

    // Generate frames for the command
    int num_frames = config_.frames_per_command;

    for (int i = 0; i < num_frames; ++i) {
        double progress = (num_frames > 1) ? static_cast<double>(i) / (num_frames - 1) : 1.0;

        Keyframe kf(start_frame + i, command.element_id, toString(command.command_type));
        kf.setProperty("elementName", command.element_name);
        kf.setProperty("progress", progress);

        // Add command-specific parameters
        for (const auto& [key, value] : command.parameters) {
            kf.setProperty(key, value);
        }

        // Add property changes with interpolated current values
        for (const auto& change : command.property_changes) {
            std::string current_value = calculateCurrentValue(change, progress);
            kf.propertyChanges.push_back({
                change.property_name,
                change.old_value,
                change.new_value
            });
            // Store current_value as additional metadata
            // (In full implementation, would extend PropertyChange to include current_value)
        }

        keyframes.push_back(kf);
    }

    return keyframes;
}

std::string KeyframeGenerator::calculateCurrentValue(
    const PropertyChange& change,
    double progress) const {

    // Try numeric interpolation first
    try {
        double old_val = std::stod(change.old_value);
        double new_val = std::stod(change.new_value);

        if (config_.interpolation_mode == InterpolationMode::LINEAR) {
            return std::to_string(interpolateLinear(old_val, new_val, progress));
        } else {
            // STEP mode for numeric - use threshold
            return (progress < 0.5) ? change.old_value : change.new_value;
        }
    } catch (const std::exception&) {
        // Not numeric, use categorical interpolation
        if (config_.interpolation_mode == InterpolationMode::LINEAR) {
            // Linear mode with categorical values - still use step
            return interpolateStep(change.old_value, change.new_value, progress);
        } else {
            return interpolateStep(change.old_value, change.new_value, progress);
        }
    }
}

std::string KeyframeGenerator::serializeToJSON(
    const std::vector<Keyframe>& keyframes) const {

    std::ostringstream json;
    json << "[\n";

    for (size_t i = 0; i < keyframes.size(); ++i) {
        const auto& kf = keyframes[i];

        json << "  {\n";
        json << "    \"frameIndex\": " << kf.frameIndex << ",\n";
        json << "    \"elementId\": \"" << kf.elementId << "\",\n";
        json << "    \"elementType\": \"" << kf.elementType << "\",\n";
        json << "    \"properties\": {\n";

        // Serialize properties
        bool first_prop = true;
        for (const auto& [key, value] : kf.properties) {
            if (!first_prop) json << ",\n";
            first_prop = false;

            json << "      \"" << key << "\": ";

            // Try to extract different types
            try {
                double num_val = std::any_cast<double>(value);
                json << num_val;
            } catch (...) {
                try {
                    std::string str_val = std::any_cast<std::string>(value);
                    json << "\"" << str_val << "\"";
                } catch (...) {
                    json << "\"<unknown>\"";
                }
            }
        }

        json << "\n    }";

        // Serialize property changes
        if (!kf.propertyChanges.empty()) {
            json << ",\n    \"propertyChanges\": [\n";

            for (size_t j = 0; j < kf.propertyChanges.size(); ++j) {
                const auto& change = kf.propertyChanges[j];

                json << "      {\n";
                json << "        \"propertyName\": \"" << change.property_name << "\",\n";
                json << "        \"oldValue\": \"" << change.old_value << "\",\n";
                json << "        \"newValue\": \"" << change.new_value << "\"\n";
                json << "      }";

                if (j < kf.propertyChanges.size() - 1) {
                    json << ",";
                }
                json << "\n";
            }

            json << "    ]";
        }

        json << "\n  }";

        if (i < keyframes.size() - 1) {
            json << ",";
        }
        json << "\n";
    }

    json << "]\n";
    return json.str();
}

bool KeyframeGenerator::validateKeyframe(const Keyframe& kf) const {
    // Check frameIndex is non-negative
    if (kf.frameIndex < 0) {
        return false;
    }

    // Check elementId is not empty (except for NO_CHANGE)
    if (kf.elementType != "none" && kf.elementId.empty()) {
        return false;
    }

    // Check progress value if present
    try {
        double progress = kf.getPropertyNumeric("progress", -1.0);
        if (progress < 0.0 || progress > 1.0) {
            return false;
        }
    } catch (...) {
        // Progress not found or invalid, which may be OK for some commands
    }

    // Check elementType is valid
    if (kf.elementType.empty()) {
        return false;
    }

    // Verify elementType matches known command types
    if (kf.elementType != "ADD_ELEMENT" &&
        kf.elementType != "REMOVE_ELEMENT" &&
        kf.elementType != "MOVE_LINEAR" &&
        kf.elementType != "ROTATE_ON_CIRCLE" &&
        kf.elementType != "SCALE_RADIUS" &&
        kf.elementType != "ADD_ATTRIBUTE" &&
        kf.elementType != "MODIFY_ATTRIBUTE" &&
        kf.elementType != "MODIFY_STRUCTURE" &&
        kf.elementType != "none") {
        return false;
    }

    return true;
}

bool KeyframeGenerator::validateAllKeyframes(
    const std::vector<Keyframe>& keyframes) const {

    for (const auto& kf : keyframes) {
        if (!validateKeyframe(kf)) {
            return false;
        }
    }
    return true;
}

} // namespace GCS
