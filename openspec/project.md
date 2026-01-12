# Project Context

## Purpose
PlaneGCS is a geometric constraint solver library designed for 2D CAD applications. Originally derived from FreeCAD's constraint system, it provides algorithms for solving geometric constraints between points, lines, circles, arcs, and other conic sections. The project also includes experimental LLM-based features for geometric animation and graph extraction.

## Tech Stack
- **Language**: C++17
- **Build System**: CMake 3.15+
- **Package Manager**: vcpkg
- **Core Dependencies**:
  - Eigen3 (linear algebra, sparse matrices)
  - Boost (locale, graph algorithms)
  - cpprestsdk (HTTP client for LLM integration)
  - libcurl (HTTP requests)
- **Platform**: Windows (MSVC), with Linux compatibility
- **License**: GNU Library General Public License (LGPL)

## Project Conventions

### Code Style
- **Header Guards**: Use `#ifndef PLANEGCS_<FILENAME>_H` format
- **Namespaces**: Main code in `GCS` namespace
- **Naming Conventions**:
  - Classes: PascalCase (e.g., `System`, `SubSystem`, `Constraint`)
  - Methods: camelCase (e.g., `addConstraint`, `solve`)
  - Member variables: camelCase with trailing underscore (e.g., `plist_`, `clist_`)
  - Macros: UPPER_SNAKE_CASE (e.g., `XconvergenceRough`)
- **File Organization**:
  - `src/` - Core GCS library
  - `aichater/` - LLM integration components
  - `models_for_test/` - Testing data structures
  - `examples/` - Demo applications

### Architecture Patterns
- **System Class**: Main entry point holding constraints and parameters
- **Constraint-Based**: Geometric relationships modeled as constraint objects
- **Solver Algorithms**:
  - BFGS (Broyden-Fletcher-Goldfarb-Shanno)
  - Levenberg-Marquardt
  - DogLeg
- **Decomposition**: System partitions into subsystems for efficient solving
- **QR Decomposition**: Dense and sparse QR for redundancy/conflict detection

### Testing Strategy
- Example/demo programs in `examples/` directory
- Manual testing through `solution_to_keyframes_demo` executable
- No automated test suite currently established

### Git Workflow
- Main branch: `main`
- Commit messages: Conventional commits format preferred
- Recent work focused on LLM integration and geometric graph extraction

## Domain Context

### Geometric Constraint Solving
This project implements a **geometric constraint solver** used in CAD applications. Key concepts:

- **Parameters**: Double-precision floating point values representing geometric properties (coordinates, angles, distances)
- **Constraints**: Mathematical relationships between geometric entities (e.g., "point on line", "tangent to circle", "parallel lines")
- **DOF (Degrees of Freedom)**: Number of independent parameters in the system
- **Redundancy**: Constraints that don't add new information
- **Conflict**: Constraints that cannot be simultaneously satisfied

### Supported Geometric Entities
- Points, Lines, Circles, Arcs
- Ellipses, Hyperbolas, Parabolas
- Arcs of conic sections
- B-spline curves

### LLM Integration (Experimental)
The project explores using LLMs to:
- Extract geometric graph structures from mathematical problem descriptions
- Generate animation keyframes from geometric solutions
- Analyze geometric relationships in natural language text

### Key File Locations
- `src/GCS.h` - Main System class and constraint API
- `src/Geo.h` - Geometric entity definitions
- `src/Constraints.h` - Constraint type definitions
- `aichater/AIChater.h` - LLM prompt construction and API calls
- `examples/solution_to_keyframes_demo.cpp` - LLM animation demo

## Important Constraints

### Performance Constraints
- Solver iterations limited by `maxIter` and `maxIterRedundant` settings
- Convergence thresholds: `convergence` (default 1e-8)
- Large systems (1000+ constraints) require sparse QR decomposition

### Platform Constraints
- Windows MSVC is primary development platform
- vcpkg required for dependency management
- C++17 features used (e.g., `std::map`, standard library containers)

### Licensing Constraints
- LGPL v2 - derivative works must be LGPL-compatible
- Originally from FreeCAD project

### Stability Constraints
- Core GCS library is mature and stable
- LLM integration features are experimental and under active development
- Breaking changes should be avoided in public API

## External Dependencies

### Eigen3
- **Purpose**: Linear algebra (matrix operations, QR decomposition)
- **Version**: 3.x (SparseQR requires >= 3.2.2)
- **Usage**: Jacobian computation, system solving

### Boost
- **Purpose**: Graph algorithms, locale utilities
- **Components**: `boost::graph`, `boost::locale`
- **Usage**: Constraint graph representation

### cpprestsdk & libcurl
- **Purpose**: HTTP client for LLM API calls
- **Usage**: Calling DeepSeek/other LLM endpoints in AIChater

### OpenSpec
- **Purpose**: Spec-driven development workflow
- **Usage**: Managing changes and specifications
- **Integration**: Use `/openspec:*` commands for proposals
