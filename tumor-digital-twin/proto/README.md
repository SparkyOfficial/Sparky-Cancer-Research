# Protocol Buffers Directory

This directory contains `.proto` files for gRPC service definitions and data structures used in the Tumor Digital Twin system.

## Proto Files

### common.proto
Defines common data types used across the system:
- `Position3D`: 3D spatial coordinates
- `AgentType`: Cell types (cancer, immune, fibroblast, etc.)
- `CellState`: Cell states (proliferating, quiescent, apoptotic, necrotic)
- `SimulationStatus`: Simulation execution status
- `SubstanceType`: Types of substances in the grid (oxygen, glucose, drugs, etc.)
- `Agent`: Individual cell representation
- `Genotype`: Mutation information
- `GridMetadata`: Grid dimensions and spatial resolution

### patient_data.proto
Defines patient data structures for multimodal input:
- `DicomData`: Medical imaging data (CT/MRI scans)
- `Mutation`: Individual genomic mutation
- `VcfData`: Genomic mutations from VCF files
- `GenomicSequenceData`: FASTQ/BAM sequence data
- `PatientData`: Complete patient data package

### simulation.proto
Defines simulation configuration and state:
- `Drug`: Drug properties for treatment protocols
- `DosingSchedule`: Drug dosing schedule
- `TreatmentProtocol`: Complete treatment protocol with multiple drugs
- `SimulationParameters`: All simulation configuration parameters
- `SubcloneInfo`: Subclonal population information
- `SimulationMetrics`: Metrics at a specific time point
- `GridData`: Grid data for substances
- `SimulationState`: Complete simulation state snapshot

### service.proto
Defines the gRPC service interface:
- `SimulationService`: Main service with RPC methods
  - `StartSimulation`: Start a new simulation
  - `GetSimulationStatus`: Query simulation progress
  - `GetSimulationResults`: Stream simulation results
  - `StopSimulation`: Stop a running simulation
  - `ListSimulations`: List all simulations
  - `LoadSimulation`: Load a saved simulation
  - `HealthCheck`: Service health check

## Code Generation

### C++ (Backend)

C++ code is generated automatically during CMake build:

```bash
cd build
cmake ..
make
```

Generated files are placed in `build/generated/`:
- `*.pb.h` / `*.pb.cc`: Protocol Buffer message classes
- `service.grpc.pb.h` / `service.grpc.pb.cc`: gRPC service stubs

### Kotlin (Frontend)

Kotlin code is generated automatically during Gradle build:

```bash
cd frontend
./gradlew build
```

Generated files are placed in `frontend/build/generated/source/proto/main/`:
- `java/`: Java Protocol Buffer classes
- `kotlin/`: Kotlin Protocol Buffer extensions
- `grpc/`: gRPC service stubs
- `grpckt/`: Kotlin gRPC coroutine stubs

## Manual Generation

To regenerate proto files manually:

### C++ (using protoc)
```bash
protoc --cpp_out=build/generated \
       --grpc_out=build/generated \
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
       -I proto \
       proto/*.proto
```

### Kotlin (using Gradle)
```bash
cd frontend
./gradlew generateProto
```

## Requirements Validation

This implementation satisfies:
- **Requirement 6.1**: gRPC Service exposes defined endpoints
- **Requirement 6.2**: StartSimulation validates patient data
- **Requirement 6.3**: GetSimulationStatus returns progress
- **Requirement 6.4**: GetSimulationResults serializes data with Protocol Buffers
