# Tumor Digital Twin - Frontend

Desktop application for visualizing and controlling tumor simulations.

## Technology Stack

- **Language**: Kotlin 1.9.21
- **UI Framework**: Compose for Desktop 1.5.11
- **Build Tool**: Gradle 8.5
- **JVM**: Java 17
- **Testing**: Kotest 5.8.0

## Project Structure

```
frontend/
├── src/
│   ├── main/
│   │   └── kotlin/
│   │       └── com/
│   │           └── tumordtwin/
│   │               └── Main.kt          # Application entry point
│   └── test/
│       └── kotlin/
│           └── com/
│               └── tumordtwin/
│                   └── MainTest.kt      # Test files
├── build.gradle.kts                     # Build configuration
├── settings.gradle.kts                  # Project settings
├── gradle.properties                    # Gradle properties
└── .gitignore                          # Git ignore rules
```

## Building

### Prerequisites

- Java 17 or higher
- Gradle 8.5 (or use the included wrapper)

### Build Commands

```bash
# Build the project
gradlew.bat build

# Run the application
gradlew.bat run

# Run tests
gradlew.bat test

# Create distributable package
gradlew.bat packageDistributionForCurrentOS
```

## Development

### Running in Development Mode

```bash
gradlew.bat run
```

### Running Tests

```bash
# Run all tests
gradlew.bat test

# Run tests with detailed output
gradlew.bat test --info
```

## Dependencies

### Core Dependencies
- Compose for Desktop - Declarative UI framework
- Kotlin Coroutines - Asynchronous programming
- kotlinx.serialization - Data serialization

### Communication
- gRPC Kotlin - Backend communication
- Protocol Buffers - Data serialization

### Rendering
- LWJGL - OpenGL bindings for 3D visualization

### Testing
- Kotest - Testing framework with property-based testing support

## Configuration

Build configurations are managed in `build.gradle.kts`:
- Debug: Development builds with debugging symbols
- Release: Optimized production builds

## Next Steps

1. Define Protocol Buffers schema (shared with Backend)
2. Implement gRPC client for Backend communication
3. Create UI components for data upload and simulation control
4. Implement 2D/3D visualization components
5. Add comprehensive tests

## Requirements

This frontend implements requirements from the design document:
- **Requirement 11.1**: Kotlin with Compose for Desktop framework
- **Requirement 11.2**: Kotlin Coroutines for non-blocking operations
- **Requirement 11.3**: kotlinx.serialization for type-safe data conversion
- **Requirement 11.4**: Compose Canvas for 2D rendering
- **Requirement 11.5**: LWJGL for 3D OpenGL rendering
