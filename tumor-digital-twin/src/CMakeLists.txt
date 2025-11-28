# gRPC server library
add_library(grpc_server_lib
    grpc_server.cpp
)

target_link_libraries(grpc_server_lib
    PUBLIC
    proto_lib
    gRPC::grpc++
    gRPC::grpc++_reflection
)

target_include_directories(grpc_server_lib
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Main server executable
add_executable(tumor_server
    main.cpp
)

target_link_libraries(tumor_server
    PRIVATE
    grpc_server_lib
)

# Placeholder for future core library
add_library(tumor_core INTERFACE)
target_include_directories(tumor_core INTERFACE ${PROJECT_SOURCE_DIR}/include)
