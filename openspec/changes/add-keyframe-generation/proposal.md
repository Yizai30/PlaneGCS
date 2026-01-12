# Change: Keyframe Generation from Animation Instructions

## Why
The current keyframe generation implementation in `solution_to_keyframes_core.cpp` lacks:
- Clear specification of animation command types and their parameters
- Standardized keyframe output format
- Formal bridge between GCS constraint solutions and animation keyframes
- Proper validation and error handling
- Clear documentation for command detection and interpolation logic

This makes the code difficult to maintain, extend, and integrate with other systems.

## What Changes
- **ADDED**: Keyframe Generation capability - formalized system for generating animation keyframes from geometric changes
- **ADDED**: Animation Command specification - well-defined command types (ADD_ELEMENT, MOVE_LINEAR, ROTATE_ON_CIRCLE, SCALE_RADIUS, etc.)
- **ADDED**: Animation Command Coordination - dependency analysis, conflict detection, and timing orchestration
- **ADDED**: Bridge layer connecting GCS constraint solver solutions to animation keyframes
- **ADDED**: Keyframe interpolation engine with support for numeric and categorical properties
- **ADDED**: Support for concurrent, sequential, and overlapping animations
- **ADDED**: Validation framework for animation commands and generated keyframes

## Impact
- **Affected specs**: New capability `keyframe-generation`
- **Affected code**:
  - `examples/solution_to_keyframes_core.h` - refactor and formalize existing structures
  - `examples/solution_to_keyframes_core.cpp` - extract and standardize generation logic
  - New files: `src/KeyframeGenerator.h`, `src/KeyframeGenerator.cpp` (keyframe generation)
  - New files: `src/AnimationCommand.h`, `src/AnimationCommand.cpp` (command definitions and detection)
  - New files: `src/AnimationCommandCoordinator.h`, `src/AnimationCommandCoordinator.cpp` (dependency analysis and scheduling)
  - New files: `src/GeometryAnimationBridge.h`, `src/GeometryAnimationBridge.cpp` (bridge layer integration)
- **Breaking changes**: None - this is a formalization of existing experimental code
- **Dependencies**: Eigen3 (for interpolation calculations), existing GCS System
