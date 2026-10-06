# Clang RTOS Static Analyzer

A custom LLVM/Clang Static Analyzer built in C++17 to enforce MISRA C and Real-Time Operating System (RTOS) safety constraints at compile time. 

This tool traverses the Clang Abstract Syntax Tree (AST) to detect and prevent prohibited function calls—such as dynamic memory allocation (`malloc`, `free`) and blocking delays (`sleep`, `vTaskDelay`)—inside Interrupt Service Routines (ISRs). Instead of hardcoding constraints, the analyzer uses a modern C++ Object-Oriented architecture to dynamically load rules from a JSON configuration file.

## Features
* **Custom AST Matchers:** Utilizes Clang's `LibTooling` API to pinpoint functions tagged with ARM interrupt attributes and inspect their nested call expressions.
* **Dynamic Rule Engine:** Integrates `nlohmann/json` and standard C++ algorithms to map AST nodes against dynamic constraints loaded from `rtos_rules.json`.
* **Custom Clang Diagnostics:** Generates native, formatted compiler errors pinpointing the exact line, column, and function name of the MISRA/RTOS violation.
* **Cross-Compilation Aware:** Uses target-specific AST generation (e.g., `--target=arm-none-eabi`) to accurately analyze bare-metal embedded code.

## Project Structure
* `IsrChecker.cpp`: The core Clang LibTooling application containing the AST Matchers and `FrontendAction` callbacks.
* `RuleParser.h`: A C++ class that parses the JSON configuration file and encapsulates the prohibited functions into standard template library (STL) vectors.
* `rtos_rules.json`: The dynamic configuration file dictating which functions are illegal inside an ISR.
* `rtos_test.c`: A mock bare-metal C file used to verify the analyzer's output.
* `CMakeLists.txt`: The "out-of-tree" build script that links the tool against locally built LLVM/Clang libraries.

## Prerequisites
To build this project, you need:
1. **LLVM and Clang (17.0+)**: Built from source with Tooling libraries enabled (`libclangTooling.a`, `libclangASTMatchers.a`, etc.).
2. **CMake**: Version 3.20.0 or higher.
3. **Nlohmann JSON**: The industry-standard C++ JSON library.
   * *macOS:* `brew install nlohmann-json`
   * *Linux:* `sudo apt-get install nlohmann-json3-dev`

## Build Instructions
This project uses an "out-of-tree" build, meaning it links against your existing LLVM build directory.

1. Ensure the `LLVM_DIR` and `Clang_DIR` paths in `CMakeLists.txt` point to your local LLVM build folder.
2. Generate the build files and compile:
   ```bash
   cmake -B build
   cmake --build build