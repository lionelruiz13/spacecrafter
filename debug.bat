mkdir build
cd build
if defined VCPKG_ROOT (
    echo VCPKG_ROOT is set to %VCPKG_ROOT%
) else (
    echo VCPKG_ROOT is not set. Please set it to the root of your vcpkg installation.
    pause
    exit /b 1
)
:redo
clear
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.10 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug --parallel 8 && exit
pause
goto redo
