/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Animation Command Detection Implementation
 *
 * Implements detection of animation commands by comparing old and new
 * geometry graph states.
 ***************************************************************************/

#include "AnimationCommand.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace GCS {

AnimationCommand AnimationCommandDetector::detectCommand(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Priority 1: Check for ADD_ELEMENT
    AnimationCommand cmd = detectAddElement(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 2: Check for REMOVE_ELEMENT
    cmd = detectRemoveElement(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 3: Check for SCALE_RADIUS
    cmd = detectScaleRadius(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 4: Check for ROTATE_ON_CIRCLE
    cmd = detectRotateOnCircle(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 5: Check for MOVE_LINEAR
    cmd = detectMoveLinear(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 6-9: Attribute and structure changes
    // (Simplified for initial implementation)
    cmd = detectModifyAttribute(old_graph, new_graph);
    if (cmd.command_type != AnimationCommandType::NO_CHANGE) {
        return cmd;
    }

    // Priority 9: No change detected
    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectAddElement(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Build maps of nodes by ID for comparison
    std::unordered_map<int, const GeometryTypes::GeometryNode*> old_nodes;
    for (const auto& node : old_graph.getNodes()) {
        old_nodes[node->getId()] = node.get();
    }

    // Look for nodes in new_graph that aren't in old_graph
    for (const auto& node : new_graph.getNodes()) {
        if (old_nodes.find(node->getId()) == old_nodes.end()) {
            AnimationCommand cmd(AnimationCommandType::ADD_ELEMENT);
            cmd.element_id = std::to_string(node->getId());
            cmd.element_name = "node_" + std::to_string(node->getId());
            return cmd;
        }
    }

    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectRemoveElement(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Build maps of nodes by ID for comparison
    std::unordered_map<int, const GeometryTypes::GeometryNode*> new_nodes;
    for (const auto& node : new_graph.getNodes()) {
        new_nodes[node->getId()] = node.get();
    }

    // Look for nodes in old_graph that aren't in new_graph
    for (const auto& node : old_graph.getNodes()) {
        if (new_nodes.find(node->getId()) == new_nodes.end()) {
            AnimationCommand cmd(AnimationCommandType::REMOVE_ELEMENT);
            cmd.element_id = std::to_string(node->getId());
            cmd.element_name = "node_" + std::to_string(node->getId());
            return cmd;
        }
    }

    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectScaleRadius(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Build maps of nodes by ID for comparison
    std::unordered_map<int, const GeometryTypes::GeometryNode*> old_nodes;
    std::unordered_map<int, const GeometryTypes::GeometryNode*> new_nodes;

    for (const auto& node : old_graph.getNodes()) {
        old_nodes[node->getId()] = node.get();
    }
    for (const auto& node : new_graph.getNodes()) {
        new_nodes[node->getId()] = node.get();
    }

    // Check for circle radius changes with fixed center
    for (const auto& [id, old_node] : old_nodes) {
        auto it = new_nodes.find(id);
        if (it == new_nodes.end()) continue;

        const auto* new_node = it->second;

        // Check if both are circles
        if (old_node->getType() == GeometryTypes::CIRCLE &&
            new_node->getType() == GeometryTypes::CIRCLE) {

            double old_radius = old_node->getRadius();
            double new_radius = new_node->getRadius();

            if (isSignificantChange(old_radius, new_radius)) {
                // Check if center is fixed
                double old_x = old_node->getX();
                double old_y = old_node->getY();
                double new_x = new_node->getX();
                double new_y = new_node->getY();

                double center_dist = calculateDistance(old_x, old_y, new_x, new_y);

                if (center_dist < config_.epsilon) {
                    // Center is fixed, this is SCALE_RADIUS
                    AnimationCommand cmd(AnimationCommandType::SCALE_RADIUS);
                    cmd.element_id = std::to_string(id);
                    cmd.element_name = "circle_" + std::to_string(id);
                    cmd.setParameter("OLD_RADIUS", std::to_string(old_radius));
                    cmd.setParameter("NEW_RADIUS", std::to_string(new_radius));
                    cmd.setParameter("FACTOR", std::to_string(new_radius / old_radius));
                    cmd.addPropertyChange("radius",
                                         std::to_string(old_radius),
                                         std::to_string(new_radius));
                    return cmd;
                }
            }
        }
    }

    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectRotateOnCircle(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Build maps of nodes by ID for comparison
    std::unordered_map<int, const GeometryTypes::GeometryNode*> old_nodes;
    std::unordered_map<int, const GeometryTypes::GeometryNode*> new_nodes;

    for (const auto& node : old_graph.getNodes()) {
        old_nodes[node->getId()] = node.get();
    }
    for (const auto& node : new_graph.getNodes()) {
        new_nodes[node->getId()] = node.get();
    }

    // Check for point position changes with constant radius from a center
    for (const auto& [id, old_node] : old_nodes) {
        auto it = new_nodes.find(id);
        if (it == new_nodes.end()) continue;

        const auto* new_node = it->second;

        // Only points can rotate
        if (old_node->getType() == GeometryTypes::POINT &&
            new_node->getType() == GeometryTypes::POINT) {

            double old_x = old_node->getX();
            double old_y = old_node->getY();
            double new_x = new_node->getX();
            double new_y = new_node->getY();

            double movement = calculateDistance(old_x, old_y, new_x, new_y);

            if (isSignificantChange(movement, 0.0)) {
                // Position changed - simplified check for circular motion
                // Full implementation would verify constant radius from a center point
                // For now, return NO_CHANGE to let MOVE_LINEAR handle it
                return AnimationCommand(AnimationCommandType::NO_CHANGE);
            }
        }
    }

    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectMoveLinear(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {

    // Build maps of nodes by ID for comparison
    std::unordered_map<int, const GeometryTypes::GeometryNode*> old_nodes;
    std::unordered_map<int, const GeometryTypes::GeometryNode*> new_nodes;

    for (const auto& node : old_graph.getNodes()) {
        old_nodes[node->getId()] = node.get();
    }
    for (const auto& node : new_graph.getNodes()) {
        new_nodes[node->getId()] = node.get();
    }

    // Check for point position changes
    for (const auto& [id, old_node] : old_nodes) {
        auto it = new_nodes.find(id);
        if (it == new_nodes.end()) continue;

        const auto* new_node = it->second;

        if (old_node->getType() == GeometryTypes::POINT &&
            new_node->getType() == GeometryTypes::POINT) {

            double old_x = old_node->getX();
            double old_y = old_node->getY();
            double new_x = new_node->getX();
            double new_y = new_node->getY();

            if (isSignificantChange(old_x, new_x) || isSignificantChange(old_y, new_y)) {
                AnimationCommand cmd(AnimationCommandType::MOVE_LINEAR);
                cmd.element_id = std::to_string(id);
                cmd.element_name = "point_" + std::to_string(id);

                if (isSignificantChange(old_x, new_x)) {
                    cmd.addPropertyChange("x",
                                         std::to_string(old_x),
                                         std::to_string(new_x));
                }
                if (isSignificantChange(old_y, new_y)) {
                    cmd.addPropertyChange("y",
                                         std::to_string(old_y),
                                         std::to_string(new_y));
                }
                return cmd;
            }
        }
    }

    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectAddAttribute(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {
    // Simplified - would check for new attributes in nodes
    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectModifyAttribute(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {
    // Simplified - would check for attribute value changes
    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

AnimationCommand AnimationCommandDetector::detectModifyStructure(
    const GeometryTypes::GeometryGraph& old_graph,
    const GeometryTypes::GeometryGraph& new_graph) const {
    // Simplified - would check for edge/relationship count changes
    return AnimationCommand(AnimationCommandType::NO_CHANGE);
}

} // namespace GCS
