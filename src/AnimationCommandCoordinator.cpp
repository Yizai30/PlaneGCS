/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Animation Command Coordination Implementation
 *
 * Implements dependency analysis, conflict detection, and timing
 * orchestration for animation commands.
 ***************************************************************************/

#include "AnimationCommandCoordinator.h"
#include <algorithm>
#include <iostream>

namespace GCS {

std::vector<CommandSchedule> AnimationCommandCoordinator::coordinateCommands(
    const std::vector<AnimationCommand>& commands,
    int frames_per_command) const {

    std::vector<CommandSchedule> schedules;

    if (commands.empty()) {
        return schedules;
    }

    // Detect conflicts and dependencies
    auto conflicts = detectConflicts(commands);
    auto dependencies = detectDependencies(commands);

    // Create schedule for each command
    for (size_t i = 0; i < commands.size(); ++i) {
        CommandSchedule schedule;
        schedule.command_index = static_cast<int>(i);
        schedule.start_frame = 0;
        schedule.end_frame = frames_per_command;
        schedule.timing = TimingPattern::SEQUENTIAL;

        // Add dependencies
        auto dep_it = dependencies.find(static_cast<int>(i));
        if (dep_it != dependencies.end()) {
            schedule.dependencies = dep_it->second;
        }

        // Add conflicts
        auto conf_it = conflicts.find(static_cast<int>(i));
        if (conf_it != conflicts.end()) {
            schedule.conflicts_with = conf_it->second;
        }

        schedules.push_back(schedule);
    }

    // Determine timing patterns between commands
    if (schedules.size() > 1) {
        for (size_t i = 0; i < schedules.size(); ++i) {
            for (size_t j = i + 1; j < schedules.size(); ++j) {
                TimingPattern timing = determineTiming(
                    schedules[i], schedules[j], dependencies, conflicts);

                // Apply timing pattern
                if (timing == TimingPattern::SEQUENTIAL) {
                    // Command j depends on command i
                    if (std::find(schedules[j].dependencies.begin(),
                                schedules[j].dependencies.end(),
                                static_cast<int>(i)) == schedules[j].dependencies.end()) {
                        schedules[j].dependencies.push_back(static_cast<int>(i));
                    }
                } else if (timing == TimingPattern::SIMULTANEOUS) {
                    // Commands execute at the same time
                    schedules[j].timing = TimingPattern::SIMULTANEOUS;
                }
            }
        }
    }

    // Calculate frame indices based on dependencies
    std::map<int, int> end_frames;  // Track when each command finishes
    for (auto& schedule : schedules) {
        int max_start = 0;

        // Find the latest ending dependency
        for (int dep_idx : schedule.dependencies) {
            auto it = end_frames.find(dep_idx);
            if (it != end_frames.end()) {
                max_start = std::max(max_start, it->second);
            }
        }

        schedule.start_frame = max_start;
        schedule.end_frame = max_start + frames_per_command;
        end_frames[schedule.command_index] = schedule.end_frame;
    }

    return schedules;
}

std::map<int, std::vector<int>> AnimationCommandCoordinator::detectConflicts(
    const std::vector<AnimationCommand>& commands) const {

    std::map<int, std::vector<int>> conflicts;

    for (size_t i = 0; i < commands.size(); ++i) {
        for (size_t j = i + 1; j < commands.size(); ++j) {
            if (areConflicting(commands[i], commands[j])) {
                conflicts[static_cast<int>(i)].push_back(static_cast<int>(j));
                conflicts[static_cast<int>(j)].push_back(static_cast<int>(i));
            }
        }
    }

    return conflicts;
}

std::map<int, std::vector<int>> AnimationCommandCoordinator::detectDependencies(
    const std::vector<AnimationCommand>& commands) const {

    std::map<int, std::vector<int>> dependencies;

    // Simple heuristic: If both commands are MOVE_LINEAR and affect the same point,
    // they are dependent (should execute sequentially)
    for (size_t i = 0; i < commands.size(); ++i) {
        for (size_t j = i + 1; j < commands.size(); ++j) {
            // Check if commands affect the same element
            if (affectsSameElement(commands[i], commands[j])) {
                // Same element - treat as dependent to be safe
                dependencies[static_cast<int>(j)].push_back(static_cast<int>(i));
            }
        }
    }

    return dependencies;
}

std::vector<AnimationCommand> AnimationCommandCoordinator::mergeCommands(
    const std::vector<AnimationCommand>& commands) const {

    std::vector<AnimationCommand> merged;
    std::vector<bool> processed(commands.size(), false);

    for (size_t i = 0; i < commands.size(); ++i) {
        if (processed[i]) continue;

        AnimationCommand current = commands[i];
        processed[i] = true;

        // Try to merge with subsequent commands
        for (size_t j = i + 1; j < commands.size(); ++j) {
            if (processed[j]) continue;

            if (canMerge(current, commands[j])) {
                current = mergeTwoCommands(current, commands[j]);
                processed[j] = true;
            }
        }

        merged.push_back(current);
    }

    return merged;
}

TimingPattern AnimationCommandCoordinator::determineTiming(
    const CommandSchedule& schedule1,
    const CommandSchedule& schedule2,
    const std::map<int, std::vector<int>>& dependencies,
    const std::map<int, std::vector<int>>& conflicts) const {

    // Check if there are conflicts
    bool has_conflict = false;
    auto conf_it = schedule1.conflicts_with.begin();
    for (; conf_it != schedule1.conflicts_with.end(); ++conf_it) {
        if (*conf_it == schedule2.command_index) {
            has_conflict = true;
            break;
        }
    }

    if (has_conflict) {
        // Cannot execute together - must be sequential
        return TimingPattern::SEQUENTIAL;
    }

    // Check if there are dependencies
    bool has_dependency = false;
    auto dep_it = schedule2.dependencies.begin();
    for (; dep_it != schedule2.dependencies.end(); ++dep_it) {
        if (*dep_it == schedule1.command_index) {
            has_dependency = true;
            break;
        }
    }

    if (has_dependency) {
        // Must execute sequentially
        return TimingPattern::SEQUENTIAL;
    }

    // No conflicts or dependencies - can execute simultaneously
    return TimingPattern::SIMULTANEOUS;
}

bool AnimationCommandCoordinator::affectsSameElement(
    const AnimationCommand& cmd1,
    const AnimationCommand& cmd2) const {

    return cmd1.element_id == cmd2.element_id &&
           !cmd1.element_id.empty() &&
           cmd1.element_id != "none";
}

bool AnimationCommandCoordinator::areConflicting(
    const AnimationCommand& cmd1,
    const AnimationCommand& cmd2) const {

    // Commands conflict if they affect the same element
    // but have incompatible types or contradictory changes
    if (!affectsSameElement(cmd1, cmd2)) {
        return false;
    }

    // Check for incompatible command types
    // e.g., ADD_ELEMENT and REMOVE_ELEMENT for same element
    if ((cmd1.command_type == AnimationCommandType::ADD_ELEMENT &&
         cmd2.command_type == AnimationCommandType::REMOVE_ELEMENT) ||
        (cmd1.command_type == AnimationCommandType::REMOVE_ELEMENT &&
         cmd2.command_type == AnimationCommandType::ADD_ELEMENT)) {
        return true;
    }

    // Check for contradictory movements
    // (simplified - full implementation would analyze property changes)
    if (cmd1.command_type == AnimationCommandType::MOVE_LINEAR &&
        cmd2.command_type == AnimationCommandType::MOVE_LINEAR) {
        // Both try to move the same point - likely conflict
        // unless they're moving in compatible directions
        // For now, treat as conflicting to be safe
        return true;
    }

    return false;
}

bool AnimationCommandCoordinator::canMerge(
    const AnimationCommand& cmd1,
    const AnimationCommand& cmd2) const {

    // Can only merge commands affecting the same element
    if (!affectsSameElement(cmd1, cmd2)) {
        return false;
    }

    // Can only merge if command types are compatible
    // e.g., two MOVE_LINEAR commands for same point
    if (cmd1.command_type != cmd2.command_type) {
        return false;
    }

    // Don't merge NO_CHANGE commands
    if (cmd1.command_type == AnimationCommandType::NO_CHANGE ||
        cmd2.command_type == AnimationCommandType::NO_CHANGE) {
        return false;
    }

    return true;
}

AnimationCommand AnimationCommandCoordinator::mergeTwoCommands(
    const AnimationCommand& cmd1,
    const AnimationCommand& cmd2) const {

    // Create merged command with combined property changes
    AnimationCommand merged(cmd1.command_type);
    merged.element_id = cmd1.element_id;
    merged.element_name = cmd1.element_name;

    // Copy parameters from first command
    merged.parameters = cmd1.parameters;

    // Combine property changes
    merged.property_changes = cmd1.property_changes;
    for (const auto& change : cmd2.property_changes) {
        merged.property_changes.push_back(change);
    }

    return merged;
}

std::vector<CommandSchedule> AnimationCommandCoordinator::calculateSequentialTiming(
    const std::vector<AnimationCommand>& commands,
    int frames_per_command) const {

    std::vector<CommandSchedule> schedules;
    int current_frame = 0;

    for (size_t i = 0; i < commands.size(); ++i) {
        CommandSchedule schedule;
        schedule.command_index = static_cast<int>(i);
        schedule.start_frame = current_frame;
        schedule.end_frame = current_frame + frames_per_command;
        schedule.timing = TimingPattern::SEQUENTIAL;

        schedules.push_back(schedule);
        current_frame = schedule.end_frame;
    }

    return schedules;
}

std::vector<CommandSchedule> AnimationCommandCoordinator::calculateSimultaneousTiming(
    const std::vector<AnimationCommand>& commands,
    int frames_per_command) const {

    std::vector<CommandSchedule> schedules;

    for (size_t i = 0; i < commands.size(); ++i) {
        CommandSchedule schedule;
        schedule.command_index = static_cast<int>(i);
        schedule.start_frame = 0;
        schedule.end_frame = frames_per_command;
        schedule.timing = TimingPattern::SIMULTANEOUS;

        schedules.push_back(schedule);
    }

    return schedules;
}

bool AnimationCommandCoordinator::validateSchedule(
    const CommandSchedule& schedule) const {

    // Check basic validity
    if (schedule.command_index < 0) {
        return false;
    }

    if (schedule.start_frame < 0 || schedule.end_frame < schedule.start_frame) {
        return false;
    }

    return true;
}

} // namespace GCS
