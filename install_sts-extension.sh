#!/bin/bash

# Get the path of the script
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

# Append the path to the sts-extension.vsix file
VSIX_PATH="$SCRIPT_DIR/sts-extension/dist/sts-extension.vsix"

if [ -f "$VSIX_PATH" ]; then
    echo "Installing STS extension for vscode from $VSIX_PATH"
    code --install-extension "$VSIX_PATH"
    echo -e "\033[32mSTS extension installed successfully.\033[0m"
    echo -e "\033[33mYou may need to restart VS Code for the changes to take effect.\033[0m"
else
    echo -e "\033[31mError: STS extension file not found at $VSIX_PATH\033[0m"
    exit 1
fi