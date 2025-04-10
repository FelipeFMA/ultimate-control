# VSCode Setup for Ultimate Control

This directory contains configuration files for Visual Studio Code to properly recognize gtkmm and provide a smooth development experience.

## Files Overview

- **c_cpp_properties.json**: Configures the C/C++ extension to recognize gtkmm headers
- **tasks.json**: Defines build tasks for cmake, make, and running the application
- **launch.json**: Configures debugging settings
- **settings.json**: Contains editor settings optimized for C++ development
- **extensions.json**: Recommends useful extensions for C++ and gtkmm development

## Required Extensions

Install these extensions for the best development experience:

1. **C/C++ Extension Pack** (ms-vscode.cpptools-extension-pack)
   - Includes the core C/C++ extension and additional tools

2. **CMake Tools** (ms-vscode.cmake-tools)
   - Provides CMake integration

3. **CMake** (twxs.cmake)
   - Syntax highlighting for CMake files

4. **Catch2 Test Explorer** (matepek.vscode-catch2-test-adapter)
   - If you use Catch2 for testing

5. **Doxygen Documentation Generator** (cschlosser.doxdocgen)
   - Helps with generating documentation comments

## Using the Build Tasks

Press `Ctrl+Shift+B` to access the build tasks:

- **build**: Runs cmake and make to build the project
- **cmake**: Configures the project with cmake
- **clean**: Cleans the build directory
- **run**: Runs the application

## Debugging

Press `F5` to start debugging the application.

## Troubleshooting

If IntelliSense doesn't recognize gtkmm headers:

1. Make sure you've built the project at least once to generate the compile_commands.json file
2. Reload VSCode window (`Ctrl+Shift+P` → "Reload Window")
3. If issues persist, try running:
   ```
   cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
   ```

## Notes on gtkmm Include Paths

The configuration includes common paths for gtkmm on Arch Linux. If your system has different paths, you may need to adjust them in c_cpp_properties.json.

To find the correct include paths for gtkmm on your system, run:

```bash
pkg-config --cflags gtkmm-3.0
```
