@echo off
setlocal enabledelayedexpansion

@REM Check if VCPKG_ROOT is set
if defined VCPKG_ROOT (
    echo VCPKG_ROOT is set to %VCPKG_ROOT%
) else (
    echo VCPKG_ROOT is not set. Please set it to the root of your vcpkg installation.
    pause
    exit /b 1
)

@REM Triplet (default)
set TRIPLET=x64-windows

@REM Ensure vcpkg exists
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg.exe not found at %VCPKG_ROOT%
    exit /b 1
)

@REM Install deps from manifest (vcpkg.json in repo root)
pushd "%~dp0" @REM Change to script directory
"%VCPKG_ROOT%\vcpkg.exe" install --triplet %TRIPLET% --recurse
if errorlevel 1 (
    echo vcpkg install failed
    popd
    exit /b 1
)

echo Dependencies installed successfully.
popd
endlocal
