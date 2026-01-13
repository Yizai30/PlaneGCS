/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Keyframe Type Definitions
 *
 * Defines the core data structures for animation keyframes and
 * keyframe generation configuration.
 ***************************************************************************/

#ifndef PLANEGCS_KEYFRAMEGENERATOR_H
#define PLANEGCS_KEYFRAMEGENERATOR_H

#include "AnimationCommand.h"
#include <string>
#include <map>
#include <vector>
#include <any>

namespace GCS {

/**
 * @brief Represents a single animation keyframe
 *
 * A keyframe captures the state of an animated element at a specific
 * point in time during an animation sequence.
 */
struct Keyframe {
    int frameIndex;                                    ///< Sequential frame index
    std::string elementId;                             ///< Unique element identifier
    std::string elementType;                           ///< Command type (from AnimationCommand)
    std::map<std::string, std::any> properties;        ///< Frame properties (elementName, progress, etc.)
    std::vector<PropertyChange> propertyChanges;        ///< Property changes with interpolated values

    Keyframe()
        : frameIndex(0) {}

    Keyframe(int index, const std::string& elemId, const std::string& elemType)
        : frameIndex(index), elementId(elemId), elementType(elemType) {}

    /**
     * @brief Sets a property value
     * @param key Property key
     * @param value Property value (any type)
     */
    void setProperty(const std::string& key, const std::any& value) {
        properties[key] = value;
    }

    /**
     * @brief Gets a string property value
     * @param key Property key
     * @param default_value Default value if key not found
     * @return Property value as string
     */
    std::string getPropertyString(const std::string& key,
                                  const std::string& default_value = "") const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (...) {
                return default_value;
            }
        }
        return default_value;
    }

    /**
     * @brief Gets a numeric property value
     * @param key Property key
     * @param default_value Default value if key not found
     * @return Property value as double
     */
    double getPropertyNumeric(const std::string& key,
                             double default_value = 0.0) const {
        auto it = properties.find(key);
        if (it != properties.end()) {
            try {
                return std::any_cast<double>(it->second);
            } catch (...) {
                return default_value;
            }
        }
        return default_value;
    }
};

/**
 * @brief Interpolation mode for property values
 */
enum class InterpolationMode {
    LINEAR,      ///< Linear interpolation: current = old + (new - old) * progress
    STEP         ///< Step function: current = old if progress < 0.5 else new
};

/**
 * @brief Configuration for keyframe generation
 *
 * Controls how keyframes are generated from animation commands.
 */
struct KeyframeGenerationConfig {
    int frames_per_command;        ///< Number of frames to generate per command (default: 10)
    double epsilon_threshold;      ///< Minimum change to detect (default: 1e-6)
    InterpolationMode interpolation_mode;  ///< Interpolation algorithm (default: LINEAR)

    KeyframeGenerationConfig()
        : frames_per_command(10),
          epsilon_threshold(1e-6),
          interpolation_mode(InterpolationMode::LINEAR) {}

    KeyframeGenerationConfig(int frames, double epsilon, InterpolationMode mode)
        : frames_per_command(frames),
          epsilon_threshold(epsilon),
          interpolation_mode(mode) {}
};

/**
 * @brief Generates animation keyframes from animation commands
 *
 * This class handles the generation of interpolated keyframes from animation
 * commands, supporting numeric and categorical interpolation.
 */
class KeyframeGenerator {
public:
    KeyframeGenerator() = default;
    explicit KeyframeGenerator(const KeyframeGenerationConfig& config)
        : config_(config) {}

    /**
     * @brief Generates keyframes from a list of animation commands
     *
     * For each command, generates multiple frames with interpolated property
     * values between old and new states.
     *
     * @param commands List of animation commands
     * @return Vector of generated keyframes
     */
    std::vector<Keyframe> generateKeyframes(
        const std::vector<AnimationCommand>& commands) const;

    /**
     * @brief Serializes keyframes to JSON format
     *
     * @param keyframes List of keyframes to serialize
     * @return JSON string representation
     */
    std::string serializeToJSON(const std::vector<Keyframe>& keyframes) const;

    /**
     * @brief Validates a keyframe for correctness
     *
     * Checks that the keyframe has valid frameIndex, progress range,
     * and required fields populated.
     *
     * @param kf Keyframe to validate
     * @return true if valid, false otherwise
     */
    bool validateKeyframe(const Keyframe& kf) const;

    /**
     * @brief Validates all keyframes in a list
     *
     * @param keyframes List of keyframes to validate
     * @return true if all valid, false otherwise
     */
    bool validateAllKeyframes(const std::vector<Keyframe>& keyframes) const;

private:
    KeyframeGenerationConfig config_;

    /**
     * @brief Generates keyframes for a single command
     *
     * @param command Animation command
     * @param start_frame Starting frame index
     * @return Vector of keyframes for this command
     */
    std::vector<Keyframe> generateKeyframesForCommand(
        const AnimationCommand& command,
        int start_frame) const;

    /**
     * @brief Performs linear interpolation for numeric values
     *
     * @param old_val Old value
     * @param new_val New value
     * @param progress Progress (0.0 to 1.0)
     * @return Interpolated value
     */
    double interpolateLinear(double old_val, double new_val, double progress) const {
        return old_val + (new_val - old_val) * progress;
    }

    /**
     * @brief Performs step interpolation for categorical values
     *
     * @param old_val Old value
     * @param new_val New value
     * @param progress Progress (0.0 to 1.0)
     * @return Interpolated value
     */
    std::string interpolateStep(const std::string& old_val,
                               const std::string& new_val,
                               double progress) const {
        return (progress < 0.5) ? old_val : new_val;
    }

    /**
     * @brief Calculates interpolated current value for a property change
     *
     * @param change Property change
     * @param progress Progress (0.0 to 1.0)
     * @return Current interpolated value as string
     */
    std::string calculateCurrentValue(const PropertyChange& change,
                                     double progress) const;
};

} // namespace GCS

#endif // PLANEGCS_KEYFRAMEGENERATOR_H
