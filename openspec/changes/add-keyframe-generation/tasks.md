# Implementation Tasks

## 1. API Design and Structure Definition
- [x] 1.1 Define `AnimationCommandType` enum with all command types (ADD_ELEMENT, MOVE_LINEAR, ROTATE_ON_CIRCLE, SCALE_RADIUS, REMOVE_ELEMENT, MODIFY_ATTRIBUTE, ADD_ATTRIBUTE, MODIFY_STRUCTURE, NO_CHANGE)
- [x] 1.2 Define `AnimationCommand` struct with command_type, element_id, element_name, parameters, property_changes
- [x] 1.3 Define `PropertyChange` struct with property_name, old_value, new_value
- [x] 1.4 Define `Keyframe` struct with frameIndex, elementId, elementType, properties, propertyChanges
- [x] 1.5 Define `KeyframeGenerationConfig` struct with frames_per_command, epsilon_threshold, interpolation_mode
- [x] 1.6 Add API documentation comments for all new types

## 2. Animation Command Detection Module
- [x] 2.1 Implement `AnimationCommandDetector` class in `src/AnimationCommand.h` and `src/AnimationCommand.cpp`
- [x] 2.2 Implement `detectCommand(old_graph, new_graph)` method with priority-based detection
- [x] 2.3 Implement ADD_ELEMENT detection (new nodes in new_graph)
- [x] 2.4 Implement REMOVE_ELEMENT detection (nodes in old_graph but not new_graph)
- [x] 2.5 Implement SCALE_RADIUS detection (circle radius change with fixed center)
- [x] 2.6 Implement ROTATE_ON_CIRCLE detection (point position change with constant radius from center)
- [x] 2.7 Implement MOVE_LINEAR detection (point position change without circle pattern)
- [ ] 2.8 Implement ADD_ATTRIBUTE detection (new attribute in new_graph node) - Placeholder
- [ ] 2.9 Implement MODIFY_ATTRIBUTE detection (attribute value change) - Placeholder
- [ ] 2.10 Implement MODIFY_STRUCTURE detection (edge/relationship count changes) - Placeholder
- [x] 2.11 Implement NO_CHANGE fallback (no geometric changes detected)
- [x] 2.12 Add parameter extraction for detected changes (old/new values, deltas, factors)
- [x] 2.13 Support multiple concurrent property changes in a single command (e.g., position + radius)
- [x] 2.14 Add validation of detected commands (check for required parameters, valid values)
- [ ] 2.15 Add unit tests for each command type detection
- [ ] 2.16 Add unit tests for concurrent property changes (multiple properties in one command)

## 3. Keyframe Generation Module
- [x] 3.1 Implement `KeyframeGenerator` class in `src/KeyframeGenerator.h` and `src/KeyframeGenerator.cpp`
- [x] 3.2 Implement `generateKeyframes(commands, config)` method
- [x] 3.3 Implement linear interpolation for numeric properties (old + (new-old)*progress)
- [x] 3.4 Implement step interpolation for categorical properties (old if progress<0.5 else new)
- [x] 3.5 Implement progress calculation (0.0 to 1.0 across frames_per_command)
- [x] 3.6 Implement JSON serialization of keyframes
- [ ] 3.7 Implement frameIndex management supporting three timing patterns: - Sequential only (partial)
  - Sequential (one command after another)
  - Simultaneous (same frameIndex for concurrent commands)
  - Overlapping (partial frame range overlap)
- [x] 3.8 Handle NO_CHANGE commands (single frame with no_change action)
- [x] 3.9 Add validation of generated keyframes (check progress range, required fields)
- [x] 3.10 Add error handling for interpolation failures (non-numeric in numeric field)
- [ ] 3.11 Add unit tests for interpolation algorithms
- [ ] 3.12 Add unit tests for JSON serialization
- [ ] 3.13 Add unit tests for sequential frame timing
- [ ] 3.14 Add unit tests for simultaneous frame timing (concurrent independent commands)
- [ ] 3.15 Add unit tests for overlapping frame timing

## 4. Animation Command Coordination Module
- [ ] 4.1 Implement `AnimationCommandCoordinator` class in `src/AnimationCommandCoordinator.h` and `src/AnimationCommandCoordinator.cpp`
- [ ] 4.2 Implement dependency analysis between commands (detect element relationships)
- [ ] 4.3 Implement conflict detection (identify contradictory commands for same element)
- [ ] 4.4 Implement command merging logic (combine related property changes)
- [ ] 4.5 Determine execution timing: sequential, simultaneous, or overlapping
- [ ] 4.6 Generate command scheduling metadata (start frame, end frame, dependencies)
- [ ] 4.7 Add validation for command dependencies and conflicts
- [ ] 4.8 Add unit tests for dependency detection (e.g., point on circle)
- [ ] 4.9 Add unit tests for independent command detection (concurrent execution)
- [ ] 4.10 Add unit tests for conflict detection and error reporting
- [ ] 4.11 Add unit tests for command merging (multiple property changes)

## 5. Bridge Layer Integration
- [x] 5.1 Implement `GeometryAnimationBridge` class in `src/GeometryAnimationBridge.h` and `src/GeometryAnimationBridge.cpp`
- [x] 5.2 Implement `generateAnimationKeyframes(old_graph, new_graph, config)` bridge method
- [x] 5.3 Integrate AnimationCommandDetector for command detection
- [ ] 5.4 Integrate AnimationCommandCoordinator for dependency analysis and scheduling - Not yet implemented
- [x] 5.5 Integrate KeyframeGenerator for keyframe generation with timing information
- [x] 5.6 Add workflow: graph diff → commands → coordinate scheduling → keyframes → JSON output - Basic workflow
- [x] 5.7 Add error handling and validation at each stage - Basic error handling
- [ ] 5.8 Add logging/trace output for debugging (optional, configurable)
- [ ] 5.9 Add integration tests with sample geometric graphs
- [ ] 5.10 Add integration tests for concurrent independent commands
- [ ] 5.11 Add integration tests for sequential dependent commands

## 6. GCS Integration
- [ ] 6.1 Add keyframe generation API to main GCS namespace - Available via includes
- [ ] 6.2 Provide convenience functions for common use cases
- [x] 6.3 Update CMakeLists.txt to include new source files
- [ ] 6.4 Add example usage in examples/ directory - Existing demo can use new API
- [x] 6.5 Update project documentation with keyframe generation section - docs/KEYFRAME_GENERATION.md created

## 7. LLM Module Refactoring
- [ ] 7.1 Refactor `SolutionToKeyframesProcessor` to use new bridge layer
- [ ] 7.2 Replace custom command detection with `AnimationCommandDetector`
- [ ] 7.3 Replace custom keyframe generation with `KeyframeGenerator`
- [ ] 7.4 Replace custom command coordination with `AnimationCommandCoordinator`
- [ ] 7.5 Maintain backward compatibility during transition
- [ ] 7.6 Update LLM module tests to use new APIs
- [ ] 7.7 Remove deprecated code after validation

## 8. Documentation and Examples
- [x] 8.1 Write API reference documentation for all public interfaces - Created docs/KEYFRAME_GENERATION.md
- [x] 8.2 Create tutorial for basic keyframe generation usage - Included in docs
- [x] 8.3 Document all animation command types with examples - Included in docs
- [x] 8.4 Document interpolation algorithms and behavior - Included in docs
- [ ] 8.5 Document command coordination and dependency analysis - Not yet implemented
- [ ] 8.6 Document frame timing patterns (sequential, simultaneous, overlapping) - Partial (sequential documented)
- [x] 8.7 Add troubleshooting guide for common issues - Added to docs/KEYFRAME_GENERATION.md
- [x] 8.8 Create example: simple point movement - examples/ex1_point_movement.cpp
- [x] 8.9 Create example: circle radius scaling - examples/ex2_circle_scaling.cpp
- [ ] 8.10 Create example: circular motion
- [ ] 8.11 Create example: concurrent independent animations
- [ ] 8.12 Create example: sequential dependent animations
- [ ] 8.13 Create example: complex multi-step animation

## 9. Testing and Validation
- [ ] 9.1 Add unit tests for AnimationCommandDetector (9 command types)
- [ ] 9.2 Add unit tests for concurrent property changes in commands
- [ ] 9.3 Add unit tests for KeyframeGenerator interpolation
- [ ] 9.4 Add unit tests for JSON serialization
- [ ] 9.5 Add unit tests for AnimationCommandCoordinator (dependency, conflict, merging)
- [ ] 9.6 Add integration tests for bridge layer
- [ ] 9.7 Add tests with real geometric graphs from LLM output
- [ ] 9.8 Add performance benchmarks for large graphs
- [ ] 9.9 Add validation tests for edge cases (empty graphs, single node, etc.)
- [ ] 9.10 Add regression tests comparing old vs new implementation

## 10. Code Quality and Review
- [ ] 10.1 Add code comments explaining detection algorithms
- [ ] 10.2 Add inline documentation for complex interpolation logic
- [ ] 10.3 Add documentation for dependency analysis algorithms
- [ ] 10.4 Ensure consistent naming conventions
- [ ] 10.5 Run code review with focus on command detection correctness
- [ ] 10.6 Run code review with focus on concurrency and timing logic
- [ ] 10.7 Validate JSON output format matches specification
- [ ] 10.8 Check for memory leaks and performance issues
- [ ] 10.9 Ensure thread safety if applicable (document if not)

## 11. Deployment and Migration
- [ ] 11.1 Update version number in CMakeLists.txt
- [ ] 11.2 Create migration guide for existing LLM module users
- [ ] 11.3 Mark old experimental APIs as deprecated
- [ ] 11.4 Package and test with vcpkg dependencies
- [ ] 11.5 Verify Windows MSVC compilation
- [ ] 11.6 Run full test suite and fix any failures
- [ ] 11.7 Update CHANGELOG with new features
- [ ] 11.8 Prepare release notes with examples
