# Test configuration
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

include(CTest)
include(Catch)

# gRPC server tests
add_executable(test_grpc_server
    test_grpc_server.cpp
)

target_link_libraries(test_grpc_server
    PRIVATE
    grpc_server_lib
    Catch2::Catch2WithMain
)

catch_discover_tests(test_grpc_server)
