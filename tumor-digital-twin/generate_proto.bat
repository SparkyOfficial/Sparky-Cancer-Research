@echo off
REM Script to generate Protocol Buffer code for both C++ and Kotlin

echo ========================================
echo Tumor Digital Twin - Proto Generation
echo ========================================
echo.

REM Check if we're in the project root
if not exist "proto" (
    echo Error: proto directory not found. Please run this script from the project root.
    exit /b 1
)

echo [1/3] Generating C++ Protocol Buffer code...
echo.

REM Create build directory if it doesn't exist
if not exist "build" mkdir build
cd build

REM Run CMake to generate build files and proto code
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo Error: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
echo [2/3] Building C++ proto library...
echo.

REM Build the proto library
cmake --build . --target proto_lib --config Release
if errorlevel 1 (
    echo Error: C++ proto library build failed
    cd ..
    exit /b 1
)

cd ..

echo.
echo [3/3] Generating Kotlin Protocol Buffer code...
echo.

REM Generate Kotlin proto code
cd frontend
call gradlew.bat generateProto
if errorlevel 1 (
    echo Error: Kotlin proto generation failed
    cd ..
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Proto generation completed successfully!
echo ========================================
echo.
echo C++ generated files: build/generated/
echo Kotlin generated files: frontend/build/generated/source/proto/
echo.

exit /b 0
