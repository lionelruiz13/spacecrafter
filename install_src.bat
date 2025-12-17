@echo off

@REM Create and navigate to build directory
mkdir build
cd build

@REM Initialize git submodules
git submodule update --init

@REM Configure cmake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.10
@REM If cmake configuration fail exit with error message
if errorlevel 1 (
    echo CMake configuration failed. Exiting.
    exit /b 1
)

@REM Build with number of processors available or 8 if not defined
set processor_to_use=8
if defined NUMBER_OF_PROCESSORS (
    set processor_to_use=%NUMBER_OF_PROCESSORS%
)

@REM Build the project
cmake --build . --config Release --parallel %processor_to_use%
@REM If build fail exit with error message
if errorlevel 1 (
    echo Build failed. Exiting.
    exit /b 1
)

@REM Install the project
cmake --install . --config Release
@REM If install fail exit with error message
if errorlevel 1 (
    echo Install failed. Exiting.
    exit /b 1
)

echo Installation completed successfully.
pause
