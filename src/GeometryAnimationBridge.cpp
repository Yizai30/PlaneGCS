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

    // Step 2: Generate keyframes from command
    std::vector<AnimationCommand> commands;
    commands.push_back(command);

    KeyframeGenerator generator(config);
    auto keyframes = generator.generateKeyframes(commands);

    // Step 3: Serialize to JSON
    return generator.serializeToJSON(keyframes);
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
