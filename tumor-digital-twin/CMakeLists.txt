cmake_minimum_required(VERSION 3.15)
project(TumorDigitalTwin VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Compiler flags for different configurations
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Protocol Buffers generation
set(PROTO_PATH "${CMAKE_SOURCE_DIR}/proto")
set(GENERATED_PROTOBUF_PATH "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

# Proto files
set(PROTO_FILES
    ${PROTO_PATH}/common.proto
    ${PROTO_PATH}/patient_data.proto
    ${PROTO_PATH}/simulation.proto
    ${PROTO_PATH}/service.proto
)

# Generate C++ code from proto files
set(PROTO_SRCS "")
set(PROTO_HDRS "")
set(GRPC_SRCS "")
set(GRPC_HDRS "")

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    list(APPEND PROTO_SRCS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.pb.cc")
    list(APPEND PROTO_HDRS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.pb.h")
    
    # Only service.proto has gRPC service definitions
    if(PROTO_NAME STREQUAL "service")
        list(APPEND GRPC_SRCS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.grpc.pb.cc")
        list(APPEND GRPC_HDRS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.grpc.pb.h")
    endif()
endforeach()

# Custom command to generate protobuf files
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${GENERATED_PROTOBUF_PATH}
         --grpc_out=${GENERATED_PROTOBUF_PATH}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I${PROTO_PATH}
         ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating Protocol Buffer and gRPC C++ files"
    VERBATIM
)

# Create a library for generated protobuf code
add_library(proto_lib STATIC ${PROTO_SRCS} ${GRPC_SRCS})
target_link_libraries(proto_lib
    PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
)
target_include_directories(proto_lib PUBLIC ${GENERATED_PROTOBUF_PATH})

# Additional packages (commented out until needed)
# find_package(Eigen3 REQUIRED)
# find_package(ITK REQUIRED)
# find_package(OpenMP REQUIRED)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
