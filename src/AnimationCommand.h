/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Animation Command Type Definitions and Detection
 *
 * Defines the core data structures for animation commands that represent
 * geometric changes between two states of a geometry graph.
 ***************************************************************************/

#ifndef PLANEGCS_ANIMATIONCOMMAND_H
#define PLANEGCS_ANIMATIONCOMMAND_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <any>
#include <cmath>

// Forward declarations for geometry graph structures
namespace GeometryTypes {
    enum GeometryNodeType { POINT, CIRCLE, LINE, PARABOLA, FOCUS, FOMULA, FUNCTION, DERIVATIVE };

    class GeometryNode {
    public:
        virtual int getId() const = 0;
        virtual GeometryNodeType getType() const = 0;
        virtual double getX() const = 0;
        virtual double getY() const = 0;
        virtual double getRadius() const = 0;
        virtual ~GeometryNode() = default;
    };

    class GeometryGraph {
    public:
        virtual const std::vector<std::unique_ptr<GeometryNode>>& getNodes() const = 0;
        virtual ~GeometryGraph() = default;
    };
}

namespace GCS {

/**
 * @brief Enumeration of all supported animation command types
 *
 * These commands represent the different ways geometric elements can change
 * between two states of a geometry graph.
 */
enum class AnimationCommandType {
    ADD_ELEMENT,          ///< A new geometric element is added to the graph
    REMOVE_ELEMENT,       ///< An existing geometric element is removed
    MOVE_LINEAR,          ///< A point moves in a straight line
    ROTATE_ON_CIRCLE,     ///< A point moves along a circular path
    SCALE_RADIUS,         ///< A circle's radius changes with fixed center
    ADD_ATTRIBUTE,        ///< A new attribute is added to an element
    MODIFY_ATTRIBUTE,     ///< An existing attribute's value changes
    MODIFY_STRUCTURE,     ///< The graph structure changes (edge count changes)
    NO_CHANGE             ///< No geometric changes detected
};

/**
 * @brief Converts AnimationCommandType to string representation
 */
inline const char* toString(AnimationCommandType type) {
    switch (type) {
        case AnimationCommandType::ADD_ELEMENT: return "ADD_ELEMENT";
        case AnimationCommandType::REMOVE_ELEMENT: return "REMOVE_ELEMENT";
        case AnimationCommandType::MOVE_LINEAR: return "MOVE_LINEAR";
        case AnimationCommandType::ROTATE_ON_CIRCLE: return "ROTATE_ON_CIRCLE";
        case AnimationCommandType::SCALE_RADIUS: return "SCALE_RADIUS";
        case AnimationCommandType::ADD_ATTRIBUTE: return "ADD_ATTRIBUTE";
        case AnimationCommandType::MODIFY_ATTRIBUTE: return "MODIFY_ATTRIBUTE";
        case AnimationCommandType::MODIFY_STRUCTURE: return "MODIFY_STRUCTURE";
        case AnimationCommandType::NO_CHANGE: return "NO_CHANGE";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Represents a single property change between old and new states
 *
 * Captures the transformation of a single geometric property (e.g., x coordinate,
 * radius, visibility) from an old value to a new value.
 */
struct PropertyChange {
    std::string property_name;   ///< Name of the changed property (e.g., "x", "y", "radius")
    std::string old_value;       ///< String representation of the old value
    std::string new_value;       ///< String representation of the new value

    PropertyChange() = default;

    PropertyChange(const std::string& name,
                   const std::string& old_val,
                   const std::string& new_val)
        : property_name(name), old_value(old_val), new_value(new_val) {}
};

/**
 * @brief Represents an animation command with all its parameters and property changes
 *
 * An animation command captures a geometric change detected when comparing
 * two geometry graph states. It includes the command type, affected element
 * information, command-specific parameters, and detailed property changes.
 */
struct AnimationCommand {
    AnimationCommandType command_type;              ///< Type of animation command
    std::string element_id;                          ///< Unique identifier of affected element
    std::string element_name;                        ///< Human-readable name of the element
    std::map<std::string, std::string> parameters;  ///< Command-specific parameters
    std::vector<PropertyChange> property_changes;    ///< List of property changes

    AnimationCommand()
        : command_type(AnimationCommandType::NO_CHANGE) {}

    explicit AnimationCommand(AnimationCommandType type)
        : command_type(type) {}

    /**
     * @brief Adds a property change to this command
     * @param name Property name
     * @param old_val Old value
     * @param new_val New value
     */
    void addPropertyChange(const std::string& name,
                          const std::string& old_val,
                          const std::string& new_val) {
        property_changes.emplace_back(name, old_val, new_val);
    }

    /**
     * @brief Sets a command-specific parameter
     * @param key Parameter key
     * @param value Parameter value
     */
    void setParameter(const std::string& key, const std::string& value) {
        parameters[key] = value;
    }

    /**
     * @brief Gets a command-specific parameter
     * @param key Parameter key
     * @param default_value Default value if key not found
     * @return Parameter value or default
     */
    std::string getParameter(const std::string& key,
                            const std::string& default_value = "") const {
        auto it = parameters.find(key);
        return (it != parameters.end()) ? it->second : default_value;
    }

    /**
     * @brief Checks if a parameter exists
     * @param key Parameter key
     * @return True if parameter exists
     */
    bool hasParameter(const std::string& key) const {
        return parameters.find(key) != parameters.end();
    }
};

/**
 * @brief Detects animation commands by comparing two geometry graph states
 *
 * This class implements priority-based detection of geometric changes,
 * identifying the most significant animation command type based on
 * structural differences between old and new graph states.
 */
class AnimationCommandDetector {
public:
    /**
     * @brief Configuration for command detection
     */
    struct DetectionConfig {
        double epsilon;  ///< Minimum change threshold (default: 1e-6)

        DetectionConfig() : epsilon(1e-6) {}
        explicit DetectionConfig(double eps) : epsilon(eps) {}
    };

    AnimationCommandDetector() = default;
    explicit AnimationCommandDetector(const DetectionConfig& config)
        : config_(config) {}

    /**
     * @brief Detects the primary animation command by comparing graph states
     *
     * Analyzes the old and new geometry graphs to determine the most
     * significant geometric change, following the priority order:
     * 1. ADD_ELEMENT  2. REMOVE_ELEMENT  3. SCALE_RADIUS
     * 4. ROTATE_ON_CIRCLE  5. MOVE_LINEAR  6. ADD_ATTRIBUTE
     * 7. MODIFY_ATTRIBUTE  8. MODIFY_STRUCTURE  9. NO_CHANGE
     *
     * @param old_graph Previous geometry graph state
     * @param new_graph Current geometry graph state
     * @return Detected animation command with parameters and property changes
     */
    AnimationCommand detectCommand(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

private:
    DetectionConfig config_;

    // Detection methods for each command type (in priority order)
    AnimationCommand detectAddElement(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectRemoveElement(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectScaleRadius(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectRotateOnCircle(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectMoveLinear(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectAddAttribute(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectModifyAttribute(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    AnimationCommand detectModifyStructure(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;

    // Helper methods
    bool isSignificantChange(double old_val, double new_val) const {
        return std::abs(new_val - old_val) > config_.epsilon;
    }

    double calculateDistance(double x1, double y1, double x2, double y2) const {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }
};

} // namespace GCS

#endif // PLANEGCS_ANIMATIONCOMMAND_H
