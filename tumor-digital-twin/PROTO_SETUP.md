# Protocol Buffers Setup Guide

This document describes the Protocol Buffers schema implementation for the Tumor Digital Twin system and provides instructions for code generation.

## Overview

The Protocol Buffers schema has been implemented with 4 proto files:

1. **common.proto** - Common data types (Position3D, Agent, enums)
2. **patient_data.proto** - Patient data structures (DICOM, VCF, genomic data)
3. **simulation.proto** - Simulation configuration and state
4. **service.proto** - gRPC service definitions

## Requirements Satisfied

This implementation satisfies the following requirements from the design document:

- ✅ **Requirement 6.1**: gRPC Service exposes defined endpoints for simulation management
- ✅ **Requirement 6.2**: StartSimulation RPC validates patient data and initiates computation
- ✅ **Requirement 6.3**: GetSimulationStatus RPC returns current progress and estimated completion time
- ✅ **Requirement 6.4**: GetSimulationResults RPC serializes and transmits 3D grid data and agent states using Protocol Buffers

## Schema Design

### Data Flow

```
Frontend (Kotlin)                    Backend (C++)
     |                                    |
     | StartSimulation(PatientData)      |
     |---------------------------------->|
     |                                   | Parse DICOM/VCF
     |                                   | Initialize simulation
     | SimulationResponse                |
     |<----------------------------------|
     |                                    |
     | GetSimulationStatus()              |
     |---------------------------------->|
     |                                   | Check progress
     | StatusResponse (progress %)       |
     |<----------------------------------|
     |                                    |
     | GetSimulationResults()             |
     |---------------------------------->|
     |                                   | Stream results
     | ResultsChunk (streaming)          |
     |<----------------------------------|
```

### Key Design Decisions

1. **Streaming Results**: `GetSimulationResults` uses server-side streaming to handle large result sets (millions of agents, large grids)

2. **Flexible Parameters**: `SimulationParameters` includes `extra_params` map for extensibility without breaking the schema

3. **Comprehensive Status**: `StatusResponse` includes progress percentage, estimated time remaining, and current metrics

4. **Checkpoint Support**: `StopRequest` allows saving checkpoints, `LoadSimulation` enables resumption

5. **Metadata Separation**: `SimulationSummary` provides lightweight metadata for listing without loading full state

## Prerequisites

Before generating code, you need to install:

### For C++ Backend

1. **Protocol Buffers Compiler (protoc)**
   - Download from: https://github.com/protocolbuffers/protobuf/releases
   - Add to PATH

2. **gRPC C++**
   - Install via vcpkg (recommended for Windows):
     ```bash
     vcpkg install grpc:x64-windows
     ```
   - Or build from source: https://grpc.io/docs/languages/cpp/quickstart/

3. **CMake** (version 3.15+)
   - Download from: https://cmake.org/download/

### For Kotlin Frontend

1. **Java Development Kit (JDK)** (version 17+)
   - Already configured in the project

2. **Gradle** (included via wrapper)
   - No additional installation needed

## Code Generation

### Option 1: Automated Script (Recommended)

Run the provided batch script from the project root:

```bash
generate_proto.bat
```

This will:
1. Generate C++ code via CMake
2. Build the proto library
3. Generate Kotlin code via Gradle

### Option 2: Manual Generation

#### C++ Code Generation

```bash
# Create build directory
mkdir build
cd build

# Configure CMake (this generates proto code)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build proto library
cmake --build . --target proto_lib --config Release
```

Generated files will be in `build/generated/`:
- `common.pb.h` / `common.pb.cc`
- `patient_data.pb.h` / `patient_data.pb.cc`
- `simulation.pb.h` / `simulation.pb.cc`
- `service.pb.h` / `service.pb.cc`
- `service.grpc.pb.h` / `service.grpc.pb.cc` (gRPC service)

#### Kotlin Code Generation

```bash
cd frontend
./gradlew generateProto
```

Generated files will be in `frontend/build/generated/source/proto/main/`:
- `java/tumordtwin/` - Java classes
- `kotlin/tumordtwin/` - Kotlin extensions
- `grpc/tumordtwin/` - gRPC service stubs
- `grpckt/tumordtwin/` - Kotlin coroutine stubs

## Using Generated Code

### C++ Backend Example

```cpp
#include "service.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class SimulationServiceImpl final : public tumordtwin::SimulationService::Service {
    grpc::Status StartSimulation(
        grpc::ServerContext* context,
        const tumordtwin::SimulationRequest* request,
        tumordtwin::SimulationResponse* response) override {
        
        // Access patient data
        const auto& patientData = request->data();
        const auto& dicom = patientData.dicom();
        
        // Validate and process
        response->set_simulation_id(generateUUID());
        response->set_status(tumordtwin::QUEUED);
        
        return grpc::Status::OK;
    }
};
```

### Kotlin Frontend Example

```kotlin
import tumordtwin.*
import io.grpc.ManagedChannelBuilder

class SimulationClient(host: String, port: Int) {
    private val channel = ManagedChannelBuilder
        .forAddress(host, port)
        .usePlaintext()
        .build()
    
    private val stub = SimulationServiceGrpcKt.SimulationServiceCoroutineStub(channel)
    
    suspend fun startSimulation(patientId: String, data: PatientData): SimulationResponse {
        val request = simulationRequest {
            this.patientId = patientId
            this.data = data
            params = simulationParameters {
                gridSizeX = 256
                gridSizeY = 256
                gridSizeZ = 256
                numSteps = 1000
                timeStep = 0.1
            }
        }
        
        return stub.startSimulation(request)
    }
}
```

## Verification

To verify the proto files are correctly defined:

1. Check syntax:
   ```bash
   protoc --proto_path=proto --decode_raw < /dev/null proto/*.proto
   ```

2. Verify imports:
   - `simulation.proto` imports `common.proto` ✓
   - `service.proto` imports all other protos ✓

3. Check for circular dependencies: None ✓

## Next Steps

After generating the proto code:

1. **Task 4**: Implement basic gRPC server in C++
   - Use generated `SimulationService::Service` base class
   - Implement stub handlers for each RPC method

2. **Task 5**: Implement gRPC client in Kotlin
   - Use generated `SimulationServiceCoroutineStub`
   - Implement connection management and retry logic

3. **Integration**: Wire up the generated code with actual simulation logic

## Troubleshooting

### CMake can't find Protobuf/gRPC

Install via vcpkg:
```bash
vcpkg install protobuf:x64-windows grpc:x64-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

### Gradle proto generation fails

Ensure the protobuf plugin is properly configured in `build.gradle.kts`:
```kotlin
plugins {
    id("com.google.protobuf") version "0.9.4"
}
```

### Import errors in proto files

Verify the import paths match the file structure:
- All proto files are in `proto/` directory
- Imports use relative paths: `import "common.proto";`

## References

- Protocol Buffers Documentation: https://protobuf.dev/
- gRPC Documentation: https://grpc.io/docs/
- gRPC-Kotlin: https://github.com/grpc/grpc-kotlin
- Design Document: `.kiro/specs/tumor-digital-twin/design.md`
