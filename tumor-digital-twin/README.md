# Tumor Digital Twin - Backend
tumor digital twin — цифровой двойник опухолевой ткани, включающего симуляции, обмен структурированными данными и инструменты для расширения модели.

⚠️ Важно: проект создан исключительно в образовательных и научно-исследовательских целях.
Он не является медицинским инструментом и не предназначен для диагностики или лечения.

## Project Structure

```
.
├── CMakeLists.txt          # Main CMake configuration
├── src/                    # Source files (.cpp)
│   └── CMakeLists.txt
├── include/                # Header files (.h, .hpp)
├── tests/                  # Unit and property-based tests
│   └── CMakeLists.txt
├── proto/                  # Protocol Buffer definitions (.proto)
└── .gitignore             # Git ignore patterns
```

## Build Requirements

- CMake 3.15 or higher
- C++17 or C++20 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Dependencies (to be added):
  - gRPC and Protocol Buffers
  - Eigen3 (linear algebra)
  - ITK (medical imaging)
  - htslib (genomic data)
  - spdlog (logging)
  - OpenMP (parallelization)
  - MPI (distributed computing)
  - CUDA (optional, GPU acceleration)

## Build Instructions

### Debug Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Release with Debug Info
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

## Running Tests

```bash
cd build
ctest --output-on-failure
```

## Development Status
This project is in active development. The current task is setting up the foundational project structure.