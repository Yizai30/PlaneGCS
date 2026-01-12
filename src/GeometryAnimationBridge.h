/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Geometry Animation Bridge
 *
 * Provides the bridge layer between GCS constraint solver solutions
 * and animation keyframes, orchestrating the entire workflow.
 ***************************************************************************/

#ifndef PLANEGCS_GEOMETRYANIMATIONBRIDGE_H
#define PLANEGCS_GEOMETRYANIMATIONBRIDGE_H

#include "AnimationCommand.h"
#include "KeyframeGenerator.h"
#include <vector>
#include <string>

namespace GCS {

/**
 * @brief Bridge layer connecting GCS solutions to animation keyframes
 *
 * This class orchestrates the workflow of converting geometric graph
 * differences into animation keyframes through command detection and
 * keyframe generation.
 */
class GeometryAnimationBridge {
public:
    GeometryAnimationBridge() = default;

    /**
     * @brief Generates animation keyframes by comparing two geometry graph states
     *
     * Complete workflow:
     * 1. Compare old and new graphs to detect changes
     * 2. Determine animation command type
     * 3. Extract command parameters and property changes
     * 4. Generate interpolated keyframes for the command
     * 5. Serialize keyframes to JSON format
     *
     * @param old_graph Previous geometry graph state
     * @param new_graph Current geometry graph state
     * @param config Keyframe generation configuration
     * @return JSON string containing generated keyframes
     */
    std::string generateAnimationKeyframes(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph,
        const KeyframeGenerationConfig& config = KeyframeGenerationConfig()) const;

    /**
     * @brief Generates animation keyframes from pre-detected commands
     *
     * Use this when you already have animation commands and want to
     * generate keyframes from them.
     *
     * @param commands List of animation commands
     * @param config Keyframe generation configuration
     * @return JSON string containing generated keyframes
     */
    std::string generateKeyframesFromCommands(
        const std::vector<AnimationCommand>& commands,
        const KeyframeGenerationConfig& config = KeyframeGenerationConfig()) const;
};

} // namespace GCS

#endif // PLANEGCS_GEOMETRYANIMATIONBRIDGE_H
