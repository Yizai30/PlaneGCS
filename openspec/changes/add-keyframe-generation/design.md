# Design: Keyframe Generation System

## Context
The PlaneGCS library provides geometric constraint solving for CAD applications. The experimental LLM integration module has demonstrated the ability to generate animation keyframes from geometric solution steps, but this functionality lacks standardization and proper architecture.

We need to:
1. Formalize the animation command types and detection logic
2. Create a clean bridge between GCS solutions and keyframes
3. Standardize keyframe output format (JSON)
4. Enable extensibility for new animation command types
5. Provide clear validation and error handling

## Goals / Non-Goals

### Goals
- Formalize existing animation command types (ADD_ELEMENT, MOVE_LINEAR, ROTATE_ON_CIRCLE, SCALE_RADIUS, REMOVE_ELEMENT, MODIFY_ATTRIBUTE, ADD_ATTRIBUTE, MODIFY_STRUCTURE, NO_CHANGE)
- Create animation command coordination system for dependency analysis and conflict detection
- Support concurrent, sequential, and overlapping animation execution based on geometric constraints
- Create bridge layer that converts geometric graph differences into animation commands
- Generate standardized JSON keyframes with configurable frame count per command
- Support numeric interpolation (linear) and categorical interpolation (step-based)
- Provide validation for commands and generated keyframes
- Make the system extensible for future command types

### Non-Goals
- Real-time animation rendering (keyframe generation is offline)
- Complex easing functions beyond linear interpolation (can be added later)
- Integration with specific animation engines or frameworks (output is generic JSON)
- Automatic parameter extraction from LLM (handled by separate module)
- Constraint solving during animation (uses pre-computed GCS solutions)
- Automatic conflict resolution (conflicts are reported, not automatically resolved)

## Decisions

### Decision 1: Modular Architecture
**Choice**: Separate animation into four distinct modules:
1. **AnimationCommand** - Command type definitions and detection
2. **AnimationCommandCoordinator** - Dependency analysis, conflict detection, and scheduling
3. **KeyframeGenerator** - Keyframe generation from commands with interpolation
4. **GeometryAnimationBridge** - Converts GCS graph differences to commands and orchestrates workflow

**Rationale**: This separation of concerns allows:
- Independent testing of each component
- Easy extension of new command types without affecting keyframe generation
- Clear separation between detection (what changed), coordination (when to execute), and generation (how to interpolate)
- Reuse of command detection logic for different output formats
- Clear boundaries between geometric analysis and animation output

**Alternatives considered**:
- Monolithic approach: All logic in one class (rejected - too complex, hard to test)
- Three modules without coordination (rejected - can't handle concurrent/sequential animations)
- Include animation in GCS System class (rejected - GCS should remain focused on constraint solving)

### Decision 2: JSON Keyframe Format
**Choice**: Standardize on JSON format matching existing template:
```json
{
  "frameIndex": 0,
  "elementId": "unique_id",
  "elementType": "command_type",
  "properties": {
    "elementName": "name",
    "progress": 0.5,
    "param1": "value1"
  },
  "propertyChanges": [{
    "propertyName": "x",
    "oldValue": "0.0",
    "newValue": "10.0",
    "currentValue": "5.0"
  }]
}
```

**Rationale**:
- JSON is widely supported by animation engines
- Matches existing experimental output
- Human-readable for debugging
- Supports nested structures for complex properties

**Alternatives considered**:
- Binary format (rejected - less debuggable)
- Custom text format (rejected - less standard)

### Decision 3: Interpolation Strategy
**Choice**: Support two interpolation modes:
- **Numeric**: Linear interpolation: `current = old + (new - old) * progress`
- **Categorical**: Step function: `current = old if progress < 0.5 else new`

**Rationale**:
- Linear interpolation is simple and predictable
- Step function for non-numeric properties prevents invalid states
- Easy to extend with easing functions later
- Matches current implementation behavior

**Alternatives considered**:
- Bezier curves (rejected - over-engineering for initial version)
- No interpolation (rejected - defeats purpose of keyframes)
- Complex easing functions (rejected - can add later as extension)

### Decision 4: Concurrency and Timing Patterns
**Choice**: Support three execution timing patterns for animation commands:
1. **Sequential** - Commands execute one after another (e.g., dependent operations)
2. **Simultaneous** - Commands execute concurrently with same frame indices (e.g., independent elements)
3. **Overlapping** - Commands have partially overlapping frame ranges (e.g., staggered starts)

**Rationale**:
- Real-world animations often have concurrent movements
- Dependency analysis determines when sequential execution is required
- Independent animations can run simultaneously for efficiency
- Overlapping provides natural transitions between phases

**Implementation**:
- AnimationCommandCoordinator analyzes geometric constraints to determine dependencies
- Conflict detection prevents impossible situations (e.g., two destinations for same point)
- Command merging combines related property changes into single animation
- Scheduling metadata defines start/end frames and dependencies

**Alternatives considered**:
- Only sequential execution (rejected - doesn't match real animation needs)
- Only simultaneous execution (rejected - can't handle dependencies)
- Manual timing specification (rejected - error-prone, automatic is better)

### Decision 5: Command Detection Priority
**Choice**: Define strict priority order for command detection:
1. ADD_ELEMENT (new nodes appear)
2. REMOVE_ELEMENT (nodes disappear)
3. SCALE_RADIUS (circle radius changes, center fixed)
4. ROTATE_ON_CIRCLE (point moves on circular path)
5. MOVE_LINEAR (point moves linearly)
6. ADD_ATTRIBUTE (new attribute added)
7. MODIFY_ATTRIBUTE (attribute value changes)
8. MODIFY_STRUCTURE (node/edge count changes)
9. NO_CHANGE (no geometric changes)

**Rationale**:
- Higher-level structural changes (add/remove) take precedence
- More specific movements (rotation) detected before generic (linear)
- Ensures deterministic command selection
- Matches current implementation logic

**Alternatives considered**:
- Parallel detection with scoring (rejected - complex, non-deterministic)
- User-specified priority (rejected - adds configuration complexity)

### Decision 6: Bridge Layer Placement
**Choice**: Create bridge in `src/GeometryAnimationBridge.*` as part of the main PlaneGCS library

**Rationale**:
- Animation is a valid output format for geometric solutions
- Keeps bridge close to geometric data structures
- Allows both LLM module and direct API usage
- Maintains separation from experimental LLM code
- Orchestrates all four modules (detection, coordination, generation)

**Alternatives considered**:
- Keep in examples/ (rejected - should be library feature)
- Create separate animation library (rejected - over-engineering)
- Part of LLM module (rejected - limits reusability)

## Risks / Trade-offs

### Risk: Performance
**Risk**: Processing large geometric graphs with many changes may be slow
**Mitigation**:
- Use efficient data structures (unordered_map for node lookups)
- Cache command detection results
- Configurable frame count to balance output size vs smoothness
- Optimize dependency analysis with graph algorithms

### Risk: Numeric Precision
**Risk**: Floating point comparison for change detection may miss small movements
**Mitigation**:
- Use configurable epsilon threshold (default 1e-6)
- Document precision limitations
- Provide validation warnings for near-zero changes

### Risk: Dependency Analysis Complexity
**Risk**: Analyzing geometric constraints for dependencies is algorithmically complex
**Mitigation**:
- Start with simple heuristics (element sharing, constraint relationships)
- Provide manual override for timing when automatic detection fails
- Document limitations of dependency detection
- Add comprehensive logging for debugging timing issues

### Risk: Conflict False Positives
**Risk**: Dependency analyzer may flag valid concurrent operations as conflicts
**Mitigation**:
- Conservative conflict detection only for true contradictions
- Validation warnings instead of errors when possible
- Allow user to override conflict detection with explicit timing
- Comprehensive test suite with real-world geometric scenarios

### Trade-off: Extensibility vs Complexity
**Trade-off**: Plugin system for new command types adds complexity
**Decision**: Start with fixed command set, refactor to plugin system when needed
**Rationale**: YAGNI principle - don't add complexity until proven necessary

## Migration Plan

### Phase 1: Formalize Structures (No Breaking Changes)
1. Add new header files with command definitions
2. Create bridge layer with clear API
3. Keep existing experimental code working
4. Add unit tests for command detection

### Phase 2: Refactor Implementation
1. Move command detection logic to AnimationCommand module
2. Implement AnimationCommandCoordinator for dependency analysis
3. Extract keyframe generation to KeyframeGenerator with timing support
4. Update LLM module to use new bridge API
5. Add integration tests for concurrent and sequential animations

### Phase 3: Deprecate Old Code
1. Mark old functions as deprecated
2. Update documentation and examples
3. Remove experimental code after transition period

### Rollback
- Keep experimental implementation as fallback
- Git revert of bridge layer if issues arise
- No changes to GCS core library ensures safety

## Open Questions
1. **Q**: Should keyframe generation support custom frame counts per command type?
   **A**: Start with global frame count, add per-command config if needed

2. **Q**: How to handle failed interpolation (e.g., non-numeric values in numeric fields)?
   **A**: Log warning and fall back to categorical interpolation

3. **Q**: Should the bridge layer support filtering specific geometric elements?
   **A**: Not in initial version, can add element whitelist/blacklist later

4. **Q**: How to validate geometric graph consistency before command detection?
   **A**: Add optional validation step that can be enabled for debugging

5. **Q**: How sophisticated should dependency analysis be?
   **A**: Start with direct constraint relationships, add transitive dependency analysis if needed

6. **Q**: Should users be able to manually override automatic timing?
   **A**: Not in initial version, add explicit timing API if automatic detection proves insufficient
