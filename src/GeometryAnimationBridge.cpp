/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Geometry Animation Bridge Implementation
 *
 * Implements the bridge layer between GCS solutions and animation keyframes.
 ***************************************************************************/

#include "GeometryAnimationBridge.h"

namespace GCS {

std::string GeometryAnimationBridge::generateAnimationKeyframes(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph,
    const KeyframeGenerationConfig& config) const {

    // Step 1: Detect animation commands
    AnimationCommandDetector::DetectionConfig detectorConfig(config.epsilon_threshold);
    AnimationCommandDetector detector(detectorConfig);
    AnimationCommand command = detector.detectCommand(old_graph, new_graph);

    // Step 2: Coordinate commands (analyze dependencies, conflicts, timing)
    std::vector<AnimationCommand> commands;
    commands.push_back(command);

    AnimationCommandCoordinator coordinator;
    auto schedules = coordinator.coordinateCommands(commands, config.frames_per_command);

    // Step 3: Generate keyframes from coordinated commands
    KeyframeGenerator generator(config);

    // Use scheduling information to generate keyframes with correct timing
    std::vector<Keyframe> all_keyframes;
    for (const auto& schedule : schedules) {
        if (schedule.command_index >= 0 &&
            static_cast<size_t>(schedule.command_index) < commands.size()) {

            const auto& cmd = commands[schedule.command_index];

            // Generate keyframes for this command with its scheduled timing
            for (int frame = schedule.start_frame; frame < schedule.end_frame; ++frame) {
                double progress = (schedule.end_frame > schedule.start_frame) ?
                    static_cast<double>(frame - schedule.start_frame) /
                    (schedule.end_frame - schedule.start_frame - 1) : 1.0;

                Keyframe kf(frame, cmd.element_id, toString(cmd.command_type));
                kf.setProperty("elementName", cmd.element_name);
                kf.setProperty("progress", progress);

                // Add command-specific parameters
                for (const auto& [key, value] : cmd.parameters) {
                    kf.setProperty(key, value);
                }

                // Add property changes with interpolated current values
                for (const auto& change : cmd.property_changes) {
                    kf.propertyChanges.push_back(change);
                }

                all_keyframes.push_back(kf);
            }
        }
    }

    // Step 4: Serialize to JSON
    return generator.serializeToJSON(all_keyframes);
}

std::string GeometryAnimationBridge::generateKeyframesFromCommands(
    const std::vector<AnimationCommand>& commands,
    const KeyframeGenerationConfig& config) const {

    // Generate keyframes from commands
    KeyframeGenerator generator(config);
    auto keyframes = generator.generateKeyframes(commands);

    // Serialize to JSON
    return generator.serializeToJSON(keyframes);
}

} // namespace GCS
