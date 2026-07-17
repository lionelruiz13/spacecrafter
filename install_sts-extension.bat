@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Get the directory of the current script (absolute path)
set "SCRIPT_DIR=%~dp0"
REM Remove trailing backslash if needed
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM Build VSIX path
set "VSIX_PATH=%SCRIPT_DIR%\sts-extension\dist\sts-extension.vsix"

if exist "%VSIX_PATH%" (
    echo Installing STS extension for VS Code from %VSIX_PATH%
    code --install-extension "%VSIX_PATH%"
    echo.
    echo STS extension installed successfully.
    echo You may need to restart VS Code for the changes to take effect.
) else (
    echo.
    echo Error: STS extension file not found at %VSIX_PATH%
    exit /b 1
)

endlocal