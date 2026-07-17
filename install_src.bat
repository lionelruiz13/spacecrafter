@echo off

@REM Create and navigate to build directory
mkdir build
cd build

@REM Initialize git submodules
git submodule update --init

@REM Check if VCPKG_ROOT is set
if defined VCPKG_ROOT (
    echo VCPKG_ROOT is set to %VCPKG_ROOT%
) else (
    echo VCPKG_ROOT is not set. Please set it to the root of your vcpkg installation.
    pause
    exit /b 1
)

@REM Configure cmake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.10 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
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
