# Keyframe Generation System

## Overview

The Keyframe Generation System provides a formalized, standardized approach to generating animation keyframes from geometric constraint solver solutions. It detects changes between geometry graph states and produces interpolated animation keyframes in JSON format.

## Architecture

The system is organized into four main modules:

### 1. Animation Command Module (`AnimationCommand.h/cpp`)

**Purpose**: Defines data structures and detects animation commands

**Key Components**:
- `AnimationCommandType`: Enum defining all command types (ADD_ELEMENT, MOVE_LINEAR, ROTATE_ON_CIRCLE, SCALE_RADIUS, etc.)
- `AnimationCommand`: Struct containing command type, element info, parameters, and property changes
- `PropertyChange`: Struct representing individual property changes
- `AnimationCommandDetector`: Class that compares two geometry graphs and detects the primary animation command

**Usage Example**:
```cpp
#include "AnimationCommand.h"

GCS::AnimationCommandDetector detector;
GCS::AnimationCommand cmd = detector.detectCommand(old_graph, new_graph);

std::cout << "Command: " << toString(cmd.command_type) << std::endl;
std::cout << "Element: " << cmd.element_name << std::endl;
```

### 2. Keyframe Generator Module (`KeyframeGenerator.h/cpp`)

**Purpose**: Generates interpolated animation keyframes from commands

**Key Components**:
- `Keyframe`: Struct representing a single animation frame
- `KeyframeGenerationConfig`: Configuration for generation (frames per command, epsilon, interpolation mode)
- `KeyframeGenerator`: Class that generates keyframes from animation commands

**Interpolation Modes**:
- `LINEAR`: Numeric interpolation: `current = old + (new - old) * progress`
- `STEP`: Categorical interpolation: `current = old if progress < 0.5 else new`

**Usage Example**:
```cpp
#include "KeyframeGenerator.h"

GCS::KeyframeGenerationConfig config;
config.frames_per_command = 20;
config.interpolation_mode = GCS::InterpolationMode::LINEAR;

GCS::KeyframeGenerator generator(config);
std::vector<GCS::Keyframe> keyframes = generator.generateKeyframes(commands);

// Serialize to JSON
std::string json = generator.serializeToJSON(keyframes);
```

### 3. Animation Command Coordination Module (Future)

**Status**: Not yet implemented

**Purpose**: Analyzes dependencies between commands, detects conflicts, and determines execution timing (sequential, simultaneous, overlapping)

### 4. Geometry Animation Bridge (`GeometryAnimationBridge.h/cpp`)

**Purpose**: Orchestrates the complete workflow from geometry graphs to JSON keyframes

**Key Components**:
- `GeometryAnimationBridge`: Bridge class that ties together detection and generation

**Usage Example**:
```cpp
#include "GeometryAnimationBridge.h"

GCS::GeometryAnimationBridge bridge;
std::string json = bridge.generateAnimationKeyframes(old_graph, new_graph);

// Or use pre-detected commands
std::string json = bridge.generateKeyframesFromCommands(commands);
```

## Supported Animation Command Types

| Command Type | Description | Example Use Case |
|-------------|-------------|------------------|
| `ADD_ELEMENT` | New geometric element added | Adding a point or circle to the graph |
| `REMOVE_ELEMENT` | Element removed from graph | Deleting a point or circle |
| `MOVE_LINEAR` | Point moves in straight line | Point translation from (x1,y1) to (x2,y2) |
| `ROTATE_ON_CIRCLE` | Point moves on circular path | Point rotating around a center |
| `SCALE_RADIUS` | Circle radius changes, center fixed | Circle growing or shrinking |
| `ADD_ATTRIBUTE` | New attribute added to element | Adding visibility flag |
| `MODIFY_ATTRIBUTE` | Attribute value changes | Changing color or style |
| `MODIFY_STRUCTURE` | Graph structure changes | Adding/removing constraints |
| `NO_CHANGE` | No geometric changes | Placeholder for static state |

## JSON Keyframe Format

Generated keyframes follow this JSON structure:

```json
[
  {
    "frameIndex": 0,
    "elementId": "point_1",
    "elementType": "MOVE_LINEAR",
    "properties": {
      "elementName": "Point 1",
      "progress": 0.0
    },
    "propertyChanges": [
      {
        "propertyName": "x",
        "oldValue": "0.0",
        "newValue": "10.0"
      },
      {
        "propertyName": "y",
        "oldValue": "0.0",
        "newValue": "10.0"
      }
    ]
  }
]
```

## Integration with Existing Code

### For LLM Module Users

Replace existing keyframe generation with the new bridge:

```cpp
// Old way (experimental)
// SolutionToKeyframesProcessor::generateKeyframes()

// New way (standardized)
#include "GeometryAnimationBridge.h"

GCS::GeometryAnimationBridge bridge;
std::string json = bridge.generateAnimationKeyframes(old_graph, new_graph);
```

### For Direct API Usage

```cpp
#include "AnimationCommand.h"
#include "KeyframeGenerator.h"

// Detect commands
GCS::AnimationCommandDetector detector;
auto commands = {detector.detectCommand(old_graph, new_graph)};

// Generate keyframes
GCS::KeyframeGenerator generator;
auto keyframes = generator.generateKeyframes(commands);

// Process keyframes
for (const auto& kf : keyframes) {
    std::cout << "Frame " << kf.frameIndex << ": "
              << kf.elementType << std::endl;
}
```

## Configuration Options

### Detection Configuration

```cpp
GCS::AnimationCommandDetector::DetectionConfig config;
config.epsilon = 1e-6;  // Minimum change threshold
```

### Generation Configuration

```cpp
GCS::KeyframeGenerationConfig config;
config.frames_per_command = 10;        // Frames per animation command
config.epsilon_threshold = 1e-6;       // Precision threshold
config.interpolation_mode = GCS::InterpolationMode::LINEAR;
```

## Current Limitations

1. **Command Coordination**: Dependency analysis and conflict detection not yet implemented
2. **Timing Patterns**: Only sequential frame timing is currently supported
3. **ROTATE_ON_CIRCLE**: Simplified detection, falls back to MOVE_LINEAR
4. **Attribute Changes**: ADD_ATTRIBUTE and MODIFY_ATTRIBUTE are placeholders
5. **Structure Changes**: MODIFY_STRUCTURE is a placeholder

## Future Enhancements

- [ ] Implement AnimationCommandCoordinator for dependency analysis
- [ ] Support for concurrent/overlapping animations
- [ ] Enhanced ROTATE_ON_CIRCLE detection with center finding
- [ ] Full attribute change detection
- [ ] Structure change detection (edge relationships)
- [ ] Conflict detection and reporting
- [ ] Custom timing overrides
- [ ] Easing functions (bezier, ease-in/out)

## Building

The keyframe generation system is built as part of the PlaneGCS library:

```bash
cmake -B build -S .
cmake --build build
```

The new files are automatically included in the PlaneGCS static library.

## Examples

See `examples/solution_to_keyframes_demo.cpp` for a complete example of using the keyframe generation system with LLM-based geometric problem solving.

## API Reference

### AnimationCommandDetector

```cpp
class AnimationCommandDetector {
public:
    AnimationCommandDetector();
    explicit AnimationCommandDetector(const DetectionConfig& config);

    AnimationCommand detectCommand(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph) const;
};
```

### KeyframeGenerator

```cpp
class KeyframeGenerator {
public:
    KeyframeGenerator();
    explicit KeyframeGenerator(const KeyframeGenerationConfig& config);

    std::vector<Keyframe> generateKeyframes(
        const std::vector<AnimationCommand>& commands) const;

    std::string serializeToJSON(
        const std::vector<Keyframe>& keyframes) const;
};
```

### GeometryAnimationBridge

```cpp
class GeometryAnimationBridge {
public:
    std::string generateAnimationKeyframes(
        const GeometryTypes::GeometryGraph& old_graph,
        const GeometryTypes::GeometryGraph& new_graph,
        const KeyframeGenerationConfig& config = {}) const;

    std::string generateKeyframesFromCommands(
        const std::vector<AnimationCommand>& commands,
        const KeyframeGenerationConfig& config = {}) const;
};
```

## License

Part of PlaneGCS, licensed under LGPL v2.

## Troubleshooting

### Common Issues and Solutions

#### Issue: "No keyframes generated"
**Symptom**: Empty JSON output or zero keyframes
**Possible Causes**:
- NO_CHANGE command detected (graphs are identical)
- Epsilon threshold too high (small changes ignored)
**Solutions**:
- Check if graphs actually differ: `detector.detectCommand(old_graph, new_graph)`
- Reduce epsilon threshold: `config.epsilon_threshold = 1e-9`
- Enable debug logging to see detected commands

#### Issue: "Wrong command type detected"
**Symptom**: Expected MOVE_LINEAR but got NO_CHANGE or different command
**Possible Causes**:
- Epsilon threshold too high (movement below threshold)
- Command priority order (SCALE_RADIUS detected before MOVE_LINEAR)
**Solutions**:
- Adjust epsilon: `AnimationCommandDetector::DetectionConfig cfg(1e-9)`
- Check node types (POINT vs CIRCLE affects detection)
- Review command priority in AnimationCommand.h

#### Issue: "Interpolation produces wrong values"
**Symptom**: Current values don't match expected interpolation
**Possible Causes**:
- Using wrong interpolation mode for data type
- Non-numeric values in numeric fields
**Solutions**:
- Use LINEAR for numbers, STEP for categories
- Ensure property values are numeric for LINEAR mode
- Check progress calculation (0.0 to 1.0)

#### Issue: "JSON output malformed"
**Symptom**: JSON parsing errors in consuming applications
**Possible Causes**:
- Non-UTF8 characters in property values
- Missing required fields
**Solutions**:
- Validate keyframes before serialization: `generator.validateAllKeyframes(keyframes)`
- Check for empty or null values in properties
- Use ASCII strings for element names and IDs

#### Issue: "Build errors: missing symbols"
**Symptom**: Linker errors for AnimationCommand or KeyframeGenerator
**Possible Causes**:
- CMake not configured properly
- Source files not added to build
**Solutions**:
- Reconfigure CMake: `cmake -B build -S .`
- Verify CMakeLists.txt includes new source files
- Check PlaneGCS library is linked: `target_link_libraries(your_target PlaneGCS)`

#### Issue: "Performance too slow"
**Symptom**: Keyframe generation takes too long
**Possible Causes**:
- Too many frames per command
- Large geometry graphs
**Solutions**:
- Reduce frames: `config.frames_per_command = 5`
- Cache command detection results
- Profile to identify bottlenecks

### Debugging Tips

**Enable detailed output**:
```cpp
// Add logging to see what's happening
AnimationCommandDetector detector;
AnimationCommand cmd = detector.detectCommand(old_graph, new_graph);
std::cout << "Detected: " << toString(cmd.command_type) << std::endl;
std::cout << "Property changes: " << cmd.property_changes.size() << std::endl;
```

**Validate geometry graphs**:
```cpp
// Check graph contents before processing
std::cout << "Old graph nodes: " << old_graph.getNodes().size() << std::endl;
std::cout << "New graph nodes: " << new_graph.getNodes().size() << std::endl;
```

**Test with simple cases first**:
```cpp
// Start with single point movement
GeometryTypes::TestGraph graph1, graph2;
graph1.addNode(1, GeometryTypes::POINT, 0.0, 0.0);
graph2.addNode(1, GeometryTypes::POINT, 10.0, 10.0);
// Verify this works before testing complex scenarios
```

### Getting Help

- Check API documentation above
- Review test code in `examples/test_keyframe_generation.cpp`
- See existing demo: `examples/solution_to_keyframes_demo.cpp`
- Consult OpenSpec specification: `openspec/changes/add-keyframe-generation/specs/`

### Known Limitations

- ROTATE_ON_CIRCLE detection is simplified (may detect as MOVE_LINEAR)
- No automatic conflict resolution (conflicts are reported)
- Attribute-based commands (ADD_ATTRIBUTE, MODIFY_ATTRIBUTE) are placeholders
- Structure-based commands (MODIFY_STRUCTURE) are placeholders
- No concurrent/overlapping animation support yet (Section 4 not implemented)
