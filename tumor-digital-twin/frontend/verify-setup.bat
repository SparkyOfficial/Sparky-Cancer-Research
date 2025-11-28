@echo off
echo ========================================
echo Tumor Digital Twin Frontend Setup Verification
echo ========================================
echo.

echo Checking Gradle wrapper...
call gradlew.bat --version
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Gradle wrapper not working
    exit /b 1
)
echo.

echo Compiling Kotlin code...
call gradlew.bat compileKotlin
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Kotlin compilation failed
    exit /b 1
)
echo.

echo Building project (without tests)...
call gradlew.bat build -x test
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
echo.

echo ========================================
echo SUCCESS: Frontend project structure is set up correctly!
echo ========================================
echo.
echo Project structure:
echo   - Gradle build system configured
echo   - Kotlin source directory: src/main/kotlin/
echo   - Test directory: src/test/kotlin/
echo   - Dependencies configured (Compose, gRPC, LWJGL, Kotest)
echo   - .gitignore for Kotlin/Gradle artifacts
echo.
echo Next steps:
echo   1. Define Protocol Buffers schema
echo   2. Implement gRPC client
echo   3. Create UI components
echo.
