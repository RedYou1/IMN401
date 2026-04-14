@echo off
REM requires MSVC v143 installed. When installing visual studio select desktop development with c++ then select MSVC v143.
REM Navigate to the project root directory (if not already there)
cd %~dp0

REM Create a directory for the build files if it doesn't exist
if not exist "build" (
    mkdir build
)

REM Navigate to the build directory
cd build

REM Run CMake to generate the Visual Studio 2026 solution
cmake ..

REM Navigate back to the root directory
cd ..

pause
