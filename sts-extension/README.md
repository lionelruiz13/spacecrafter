# sts-extension README

## How to update language configuration

To update the language configuration for the STS language in this extension, follow these steps:

1. Open the `package.json` file located in the root directory of the extension.
2. Locate the `contributes.languages` section.
3. Modify the language configuration file path if necessary. The current configuration file is specified as `"./language-configuration.json"`.
4. Save the changes to `package.json`.

## How to update semantic tokens

To update the semantic tokens for the STS language in this extension, follow these steps:

1. Open the `src/extension.ts` file.
2. Locate the `tokenTypes` constant and the `provideDocumentSemanticTokens` method.
3. Modify or add new token types as needed in the `tokenTypes` constant.
4. Update the logic in the `provideDocumentSemanticTokens` method to assign the correct token types based on the STS language syntax.
5. Save the changes to `extension.ts`.

## How to build the extension

To build the STS extension, follow these steps:

1. Open a terminal and navigate to the root directory of the extension.
2. Run the following command:
    ```bash
    npm install
    npm run compile
    npm install -g @vscode/vsce
    vsce package --out ./dist/sts-extension.vsix
    ```
3. This will generate a `.vsix` file that can be installed in Visual Studio Code.
4. To install the generated `.vsix` file, open Visual Studio Code, go to the Extensions view, click on the three-dot menu in the top-right corner, and select "Install from VSIX...". Choose the generated `.vsix` file to install the extension.
5. Reload Visual Studio Code to activate the extension.
6. You can now use the STS language features provided by the extension.
