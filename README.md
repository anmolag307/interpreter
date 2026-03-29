# Lox Interpreter

A C++ implementation of the Lox programming language interpreter based on [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom.

## Overview

This is a tree-walk interpreter for Lox, a simple dynamically-typed scripting language. The interpreter supports:

- Variables and scoping
- Functions and recursion
- Control flow (if/else, while, for loops)
- Basic operators and expressions
- Print statements
- Comments

## Building

Prerequisites:
- CMake 3.10+
- C++17 compiler
- vcpkg (for dependency management)

Build instructions:
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
cmake --build ./build
```

## Usage

```bash
./build/interpreter run <filename>
./build/interpreter parse <filename>
./build/interpreter evaluate <filename>
./build/interpreter tokenize <filename>
```

## Project Structure

- `src/` - Source implementation files
- `include/` - Header files
- `CMakeLists.txt` - Build configuration

## Features Implemented

- ✅ Tokenization and scanning
- ✅ Parsing and AST generation
- ✅ Variable declaration and assignment
- ✅ Function definitions and calls
- ✅ Scope management
- ✅ Control flow statements
- ✅ Error handling with exit code 70 for runtime errors

# break var into int and stuff