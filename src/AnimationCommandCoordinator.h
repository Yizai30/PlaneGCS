/***************************************************************************
 * PlaneGCS - Geometric Constraint Solver
 * Copyright (c) 2025 - Keyframe Generation System
 *
 * Animation Command Coordination
 *
 * Analyzes dependencies between animation commands, detects conflicts,
 * and determines execution timing (sequential, simultaneous, overlapping).
 ***************************************************************************/

#ifndef PLANEGCS_ANIMATIONCOMMANDCOORDINATOR_H
#define PLANEGCS_ANIMATIONCOMMANDCOORDINATOR_H

#include "AnimationCommand.h"
#include <vector>
#include <map>
#include <set>

namespace GCS {

/**
 * @brief Execution timing pattern for animation commands
 */
enum class TimingPattern {
    SEQUENTIAL,      ///< Commands execute one after another
    SIMULTANEOUS,    ///< Commands execute concurrently with same frame indices
    OVERLAPPING      ///< Commands have partially overlapping frame ranges
};

/**
 * @brief Scheduling metadata for an animation command
 */
struct CommandSchedule {
    int command_index;              ///< Index in the command list
    int start_frame;                ///< Starting frame index
    int end_frame;                  ///< Ending frame index
    TimingPattern timing;           ///< When this command executes
    std::vector<int> dependencies;  ///< Indices of commands this depends on
    std::vector<int> conflicts_with; ///< Indices of conflicting commands

    CommandSchedule()
        : command_index(-1), start_frame(0), end_frame(0),
          timing(TimingPattern::SEQUENTIAL) {}
};

/**
 * @brief Coordinates animation commands by analyzing dependencies and timing
 *
 * This class analyzes a set of animation commands to determine:
 * - Which commands depend on each other (element relationships)
 * - Which commands conflict (contradictory changes)
 * - Which commands can be merged (related property changes)
 * - The optimal execution timing (sequential/simultaneous/overlapping)
 */
class AnimationCommandCoordinator {
public:
    AnimationCommandCoordinator() = default;

    /**
     * @brief Coordinates a list of animation commands
     *
     * Analyzes dependencies, detects conflicts, and generates
     * scheduling metadata for each command.
     *
     * @param commands List of animation commands to coordinate
     * @param frames_per_command Number of frames per command for timing calculation
     * @return Vector of scheduling information for each command
     */
    std::vector<CommandSchedule> coordinateCommands(
        const std::vector<AnimationCommand>& commands,
        int frames_per_command) const;

    /**
     * @brief Detects conflicts between commands
     *
     * Identifies commands that cannot execute together because they
     * affect the same element in contradictory ways.
     *
     * @param commands List of animation commands
     * @return Map of command index to list of conflicting command indices
     */
    std::map<int, std::vector<int>> detectConflicts(
        const std::vector<AnimationCommand>& commands) const;

    /**
     * @brief Detects dependencies between commands
     *
     * Identifies when one command must execute after another due to
     * element relationships (e.g., point on circle).
     *
     * @param commands List of animation commands
     * @return Map of command index to list of dependency command indices
     */
    std::map<int, std::vector<int>> detectDependencies(
        const std::vector<AnimationCommand>& commands) const;

    /**
     * @brief Merges compatible commands
     *
     * Combines commands that affect the same element into a single
     * command with multiple property changes.
     *
     * @param commands List of animation commands
     * @return Merged list of commands
     */
    std::vector<AnimationCommand> mergeCommands(
        const std::vector<AnimationCommand>& commands) const;

    /**
     * @brief Determines timing pattern for a set of commands
     *
     * Analyzes dependencies and conflicts to decide whether commands
     * should execute sequentially, simultaneously, or with overlap.
     *
     * @param schedule1 Scheduling info for first command
     * @param schedule2 Scheduling info for second command
     * @param dependencies Dependency map
     * @param conflicts Conflict map
     * @return Timing pattern for the relationship between these commands
     */
    TimingPattern determineTiming(
        const CommandSchedule& schedule1,
        const CommandSchedule& schedule2,
        const std::map<int, std::vector<int>>& dependencies,
        const std::map<int, std::vector<int>>& conflicts) const;

private:
    /**
     * @brief Checks if two commands affect the same element
     */
    bool affectsSameElement(const AnimationCommand& cmd1,
                           const AnimationCommand& cmd2) const;

    /**
     * @brief Checks if two commands are conflicting
     */
    bool areConflicting(const AnimationCommand& cmd1,
                       const AnimationCommand& cmd2) const;

    /**
     * @brief Checks if two commands can be merged
     */
    bool canMerge(const AnimationCommand& cmd1,
                 const AnimationCommand& cmd2) const;

    /**
     * @brief Merges two commands into one
     */
    AnimationCommand mergeTwoCommands(const AnimationCommand& cmd1,
                                     const AnimationCommand& cmd2) const;

    /**
     * @brief Calculates timing for sequential execution
     */
    std::vector<CommandSchedule> calculateSequentialTiming(
        const std::vector<AnimationCommand>& commands,
        int frames_per_command) const;

    /**
     * @brief Calculates timing for simultaneous execution
     */
    std::vector<CommandSchedule> calculateSimultaneousTiming(
        const std::vector<AnimationCommand>& commands,
        int frames_per_command) const;

    /**
     * @brief Validates scheduling information
     */
    bool validateSchedule(const CommandSchedule& schedule) const;
};

} // namespace GCS

#endif // PLANEGCS_ANIMATIONCOMMANDCOORDINATOR_H
