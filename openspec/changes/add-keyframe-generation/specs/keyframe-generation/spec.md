# Keyframe Generation Capability

## ADDED Requirements

### Requirement: Animation Command Types
The system SHALL support a comprehensive set of animation command types that represent geometric changes between two states of a geometry graph.

#### Scenario: Command type enumeration
- **GIVEN** the system is initialized
- **WHEN** querying available animation command types
- **THEN** the system SHALL provide the following command types:
  - `ADD_ELEMENT` - A new geometric element is added to the graph
  - `REMOVE_ELEMENT` - An existing geometric element is removed from the graph
  - `MOVE_LINEAR` - A point moves in a straight line from old to new position
  - `ROTATE_ON_CIRCLE` - A point moves along a circular path
  - `SCALE_RADIUS` - A circle's radius changes while its center remains fixed
  - `ADD_ATTRIBUTE` - A new attribute is added to a geometric element
  - `MODIFY_ATTRIBUTE` - An existing attribute's value changes
  - `MODIFY_STRUCTURE` - The graph structure changes (edge/relationship count changes)
  - `NO_CHANGE` - No geometric changes detected

#### Scenario: Command type priority
- **GIVEN** multiple geometric changes exist between two graph states
- **WHEN** detecting the primary animation command
- **THEN** the system SHALL evaluate commands in priority order:
  1. ADD_ELEMENT
  2. REMOVE_ELEMENT
  3. SCALE_RADIUS
  4. ROTATE_ON_CIRCLE
  5. MOVE_LINEAR
  6. ADD_ATTRIBUTE
  7. MODIFY_ATTRIBUTE
  8. MODIFY_STRUCTURE
  9. NO_CHANGE
- **AND** the first matching command type SHALL be selected

### Requirement: Animation Command Structure
The system SHALL represent each animation command with a structured format capturing all relevant parameters and property changes.

#### Scenario: Basic command structure
- **GIVEN** an animation command is detected
- **WHEN** creating the command object
- **THEN** the system SHALL include:
  - `command_type` (string): The type of animation command
  - `element_id` (string): Unique identifier of the affected geometric element
  - `element_name` (string): Human-readable name of the element
  - `parameters` (map<string, string>): Generic command-specific parameters
  - `property_changes` (list): List of property changes with old and new values

#### Scenario: Property change structure
- **GIVEN** a geometric property changes between old and new graph states
- **WHEN** recording the property change
- **THEN** the system SHALL capture:
  - `property_name` (string): Name of the changed property (e.g., "x", "y", "radius")
  - `old_value` (string): String representation of the old value
  - `new_value` (string): String representation of the new value

#### Scenario: Command-specific parameters
- **GIVEN** a SCALE_RADIUS command is detected
- **WHEN** populating command parameters
- **THEN** the system SHALL include:
  - `OLD_RADIUS`: The original radius value
  - `NEW_RADIUS`: The new radius value
  - `FACTOR`: The scaling factor (new_radius / old_radius)
- **AND** the property_changes SHALL include the radius change
- **AND** property_changes MAY include additional concurrent property changes (e.g., position changes while scaling)

### Requirement: Animation Command Detection
The system SHALL detect animation commands by comparing old and new geometry graph states and identifying the most significant geometric change.

#### Scenario: Detect ADD_ELEMENT command
- **GIVEN** an old geometry graph with nodes A, B, C
- **AND** a new geometry graph with nodes A, B, C, D
- **WHEN** comparing the graphs
- **THEN** the system SHALL detect an ADD_ELEMENT command for node D
- **AND** element_id SHALL be D's unique identifier
- **AND** element_name SHALL be D's name attribute
- **AND** command_type SHALL be "ADD_ELEMENT"

#### Scenario: Detect MOVE_LINEAR command
- **GIVEN** an old geometry graph with point P at coordinates (0, 0)
- **AND** a new geometry graph with point P at coordinates (10, 10)
- **AND** the distance between old and new positions > epsilon threshold
- **AND** the movement does not follow a circular path pattern
- **WHEN** comparing the graphs
- **THEN** the system SHALL detect a MOVE_LINEAR command
- **AND** property_changes SHALL include x coordinate change (0 → 10)
- **AND** property_changes SHALL include y coordinate change (0 → 10)

#### Scenario: Detect ROTATE_ON_CIRCLE command
- **GIVEN** an old geometry graph with point P at coordinates (1, 0) relative to center C
- **AND** a new geometry graph with point P at coordinates (0, 1) relative to center C
- **AND** the distance from P to C remains constant (within epsilon)
- **WHEN** comparing the graphs
- **THEN** the system SHALL detect a ROTATE_ON_CIRCLE command
- **AND** parameters SHALL include "CENTER_X", "CENTER_Y", "RADIUS"
- **AND** property_changes SHALL include angle change in radians or degrees

#### Scenario: Detect SCALE_RADIUS command
- **GIVEN** an old geometry graph with circle C radius 5.0 at center (0, 0)
- **AND** a new geometry graph with circle C radius 10.0 at center (0, 0)
- **AND** the center remains unchanged (within epsilon threshold)
- **WHEN** comparing the graphs
- **THEN** the system SHALL detect a SCALE_RADIUS command
- **AND** parameters["OLD_RADIUS"] SHALL be "5.0"
- **AND** parameters["NEW_RADIUS"] SHALL be "10.0"
- **AND** parameters["FACTOR"] SHALL be "2.0"
- **AND** property_changes[0].property_name SHALL be "radius"

#### Scenario: Detect NO_CHANGE command
- **GIVEN** an old geometry graph identical to a new geometry graph
- **WHEN** comparing the graphs
- **THEN** the system SHALL detect a NO_CHANGE command
- **AND** element_id SHALL be "none"
- **AND** element_name SHALL be empty
- **AND** parameters SHALL be empty
- **AND** property_changes SHALL be empty

### Requirement: Keyframe Generation
The system SHALL generate a sequence of keyframes from animation commands, with each command producing multiple frames interpolated between old and new states.

#### Scenario: Generate keyframes for MOVE_LINEAR command
- **GIVEN** a MOVE_LINEAR command with x: 0→10, y: 0→10
- **AND** frames_per_command = 10
- **WHEN** generating keyframes
- **THEN** the system SHALL produce 10 frames
- **AND** frame[0].progress SHALL be 0.0
- **AND** frame[0].property_changes[0].currentValue SHALL be "0" (old value)
- **AND** frame[9].progress SHALL be 1.0
- **AND** frame[9].property_changes[0].currentValue SHALL be "10" (new value)
- **AND** frame[4].property_changes[0].currentValue SHALL be approximately "5" (interpolated)

#### Scenario: Numeric interpolation
- **GIVEN** a property change with old_value="0.0" and new_value="10.0"
- **AND** progress = 0.5
- **WHEN** calculating current value
- **THEN** current_value SHALL be "5.0"
- **AND** the formula SHALL be: current = old + (new - old) * progress

#### Scenario: Categorical interpolation
- **GIVEN** a property change with old_value="visible" and new_value="hidden"
- **AND** progress = 0.3
- **WHEN** calculating current value
- **THEN** current_value SHALL be "visible" (old value, since progress < 0.5)
- **AND** for progress = 0.7, current_value SHALL be "hidden" (new value)

#### Scenario: Frame index management
- **GIVEN** two animation commands
- **AND** frames_per_command = 10
- **WHEN** generating keyframes
- **THEN** the system SHALL determine frame timing based on command execution pattern:
  - **Sequential**: First command produces frames 0-9, second produces frames 10-19 (when commands must execute in sequence)
  - **Simultaneous**: Both commands produce frames 0-9 with the same frameIndex values (when commands execute concurrently without conflicts)
  - **Overlapping**: Commands produce partially overlapping frame ranges (when second command starts before first completes)
- **AND** each frame SHALL have a unique frameIndex within its command sequence
- **AND** frameIndex values SHALL reflect the temporal relationship between concurrent animations

#### Scenario: NO_CHANGE command handling
- **GIVEN** a NO_CHANGE command
- **WHEN** generating keyframes
- **THEN** the system SHALL produce exactly 1 frame
- **AND** elementId SHALL be "none"
- **AND** elementType SHALL be "none"
- **AND** properties["action"] SHALL be "no_change"

#### Scenario: Concurrent independent animations
- **GIVEN** two MOVE_LINEAR commands affecting different geometric elements
- **AND** Element A moves from (0,0) to (10,0)
- **AND** Element B moves from (0,10) to (0,20)
- **AND** the movements are independent (no shared constraints)
- **WHEN** generating keyframes with frames_per_command = 10
- **THEN** both commands SHALL produce frames with the same frameIndex values (0-9)
- **AND** frame[5] SHALL contain both Element A at (5,0) and Element B at (0,15)
- **AND** the animations SHALL appear simultaneous in the output

#### Scenario: Sequential dependent animations
- **GIVEN** a SCALE_RADIUS command for circle C
- **AND** a MOVE_LINEAR command for a point P on circle C
- **AND** the point movement depends on the circle scaling completion
- **WHEN** generating keyframes with frames_per_command = 10
- **THEN** SCALE_RADIUS SHALL produce frames 0-9
- **AND** MOVE_LINEAR SHALL produce frames 10-19
- **AND** the point movement SHALL start after circle scaling completes

#### Scenario: Overlapping animations
- **GIVEN** Element A starts animating at frame 0 for 10 frames
- **AND** Element B starts animating at frame 5 for 10 frames
- **WHEN** generating keyframes
- **THEN** frames 0-4 SHALL contain only Element A animation
- **AND** frames 5-9 SHALL contain both Element A and Element B animations (overlapping)
- **AND** frames 10-14 SHALL contain only Element B animation
- **AND** frameIndex values SHALL reflect the overlap timing

### Requirement: JSON Keyframe Format
The system SHALL serialize keyframes to a standardized JSON format compatible with animation engines and rendering systems.

#### Scenario: Basic keyframe JSON structure
- **GIVEN** a keyframe with frameIndex=5, elementId="point_1", elementType="MOVE_LINEAR"
- **WHEN** serializing to JSON
- **THEN** the output SHALL include:
  ```json
  {
    "frameIndex": 5,
    "elementId": "point_1",
    "elementType": "MOVE_LINEAR",
    "properties": {
      "elementName": "Point 1",
      "progress": 0.5
    }
  }
  ```

#### Scenario: Keyframe with property changes
- **GIVEN** a keyframe with property changes (x: 0→10, y: 0→10) at progress 0.5
- **WHEN** serializing to JSON
- **THEN** the output SHALL include:
  ```json
  {
    "frameIndex": 5,
    "elementId": "point_1",
    "elementType": "MOVE_LINEAR",
    "properties": {
      "elementName": "Point 1",
      "progress": 0.5
    },
    "propertyChanges": [
      {
        "propertyName": "x",
        "oldValue": "0.0",
        "newValue": "10.0",
        "currentValue": "5.0"
      },
      {
        "propertyName": "y",
        "oldValue": "0.0",
        "newValue": "10.0",
        "currentValue": "5.0"
      }
    ]
  }
  ```

#### Scenario: Complete keyframe sequence
- **GIVEN** a list of 30 keyframes
- **WHEN** serializing all keyframes
- **THEN** the output SHALL be a JSON array of keyframe objects
- **AND** each keyframe SHALL be valid JSON
- **AND** the array SHALL be parseable by standard JSON parsers

### Requirement: Keyframe Generation Configuration
The system SHALL support configuration options for keyframe generation behavior.

#### Scenario: Configure frames per command
- **GIVEN** a KeyframeGenerationConfig with frames_per_command = 20
- **WHEN** generating keyframes for 3 commands
- **THEN** the system SHALL produce 60 keyframes total (3 × 20)

#### Scenario: Configure epsilon threshold
- **GIVEN** a KeyframeGenerationConfig with epsilon_threshold = 1e-6
- **WHEN** detecting changes between old and new graphs
- **THEN** property differences less than 1e-6 SHALL be considered unchanged
- **AND** coordinate changes greater than 1e-6 SHALL be detected as movement

#### Scenario: Default configuration
- **GIVEN** no explicit configuration is provided
- **WHEN** generating keyframes
- **THEN** frames_per_command SHALL default to 10
- **AND** epsilon_threshold SHALL default to 1e-6
- **AND** interpolation_mode SHALL default to "linear"

### Requirement: Animation Command Coordination
The system SHALL analyze constraints and dependencies between animation commands to determine whether they should execute simultaneously, sequentially, or with overlap.

#### Scenario: Detect command dependencies
- **GIVEN** a SCALE_RADIUS command for circle C (element_id: "circle_1")
- **AND** a MOVE_LINEAR command for point P on circle C (element_id: "point_1")
- **WHEN** analyzing command dependencies
- **THEN** the system SHALL detect that point P depends on circle C
- **AND** the system SHALL mark MOVE_LINEAR as dependent on SCALE_RADIUS
- **AND** the commands SHALL be scheduled sequentially

#### Scenario: Detect independent commands
- **GIVEN** a MOVE_LINEAR command for point A (element_id: "point_a")
- **AND** a MOVE_LINEAR command for point B (element_id: "point_b")
- **AND** point A and point B have no shared constraints
- **WHEN** analyzing command dependencies
- **THEN** the system SHALL detect that the commands are independent
- **AND** the commands SHALL be scheduled for simultaneous execution

#### Scenario: Detect conflicting commands
- **GIVEN** two MOVE_LINEAR commands for the same point P
- **AND** one command moves P to (10, 10)
- **AND** another command moves P to (20, 20)
- **WHEN** analyzing command conflicts
- **THEN** the system SHALL detect a conflict
- **AND** the system SHALL generate a validation error
- **AND** no keyframes SHALL be generated until the conflict is resolved

#### Scenario: Merge concurrent property changes
- **GIVEN** a point P that changes both x and y coordinates
- **AND** the x change is from 0 to 10
- **AND** the y change is from 0 to 20
- **WHEN** generating animation commands
- **THEN** the system SHALL create a single MOVE_LINEAR command
- **AND** property_changes SHALL include both x and y changes
- **AND** both properties SHALL be interpolated simultaneously in keyframes

### Requirement: Bridge Layer Integration
The system SHALL provide a bridge layer that converts GCS constraint solver solutions into animation keyframes.

#### Scenario: Bridge layer workflow
- **GIVEN** an old geometry graph state
- **AND** a new geometry graph state (from GCS solution)
- **AND** a keyframe generation configuration
- **WHEN** calling the bridge layer generateAnimationKeyframes() method
- **THEN** the system SHALL:
  1. Compare old and new graphs to detect changes
  2. Determine the animation command type
  3. Extract command parameters and property changes
  4. Generate interpolated keyframes for the command
  5. Serialize keyframes to JSON format
  6. Return the JSON keyframe array

#### Scenario: Bridge layer error handling
- **GIVEN** invalid geometry graph data (e.g., null nodes, missing attributes)
- **WHEN** calling the bridge layer
- **THEN** the system SHALL return an error status
- **AND** the error message SHALL indicate the specific validation failure
- **AND** no keyframes SHALL be generated

### Requirement: Validation and Error Handling
The system SHALL validate animation commands and generated keyframes to ensure correctness and provide meaningful error messages.

#### Scenario: Validate command parameters
- **GIVEN** a ROTATE_ON_CIRCLE command
- **WHEN** validating the command
- **THEN** the system SHALL verify:
  - element_id is not empty
  - parameters include "CENTER_X" and "CENTER_Y"
  - parameters include "RADIUS"
  - radius value is numeric and positive
- **AND** validation SHALL fail if any required parameter is missing or invalid

#### Scenario: Validate keyframe progress values
- **GIVEN** a generated keyframe
- **WHEN** validating the keyframe
- **THEN** the system SHALL verify:
  - frameIndex is non-negative
  - progress is in range [0.0, 1.0]
  - elementId is not empty (except for NO_CHANGE)
  - elementType is a valid command type
- **AND** validation SHALL fail if any check fails

#### Scenario: Interpolation error handling
- **GIVEN** a property change with old_value="not_a_number" and new_value="10.0"
- **WHEN** attempting numeric interpolation
- **THEN** the system SHALL log a warning
- **AND** fall back to categorical interpolation
- **AND** currentValue SHALL be "not_a_number" if progress < 0.5, else "10.0"

### Requirement: Extensibility
The system SHALL be designed to allow future addition of new animation command types without major refactoring.

#### Scenario: Adding new command type
- **GIVEN** the system architecture with modular command detection
- **WHEN** adding a new command type "ROTATE_AROUND_POINT"
- **THEN** the developer SHALL:
  1. Add the new command type to the AnimationCommandType enum
  2. Implement a detection method in AnimationCommandDetector
  3. Add the detection method to the priority order
  4. Document the new command type
- **AND** existing command types SHALL continue to work unchanged
- **AND** the keyframe generation logic SHALL work without modification

#### Scenario: Plugin architecture (future)
- **GIVEN** a future requirement for custom command types
- **WHEN** designing the plugin system
- **THEN** the system SHALL support:
  - Registration of custom command detectors
  - Custom interpolation algorithms per command type
  - Custom parameter extraction logic
- **AND** this SHALL be added without breaking existing functionality
