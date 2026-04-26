# Nova Compiler

An End-to-End Compiler for the custom Domain-Specific Language **Nova**, written completely in C.

## Features
- **Lexer:** Tokenizes source files.
- **Parser:** Recursive descent parser building an AST.
- **Semantic Analyzer:** Symbol table and static type checking.
- **Codegen:** LLVM IR emitter using the LLVM C API.
- **Backend:** Native executable generation targeting the host architecture.

## Requirements (Windows)

This project uses the LLVM C API and CMake. For Windows, the easiest way to compile this is using MSYS2 (MinGW-w64).

1. Install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MinGW x64** terminal.
3. Run the following commands to install dependencies:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-cmake
   pacman -S mingw-w64-x86_64-llvm
   pacman -S make
   ```

## Building the Compiler

From the root of the project:
```bash
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```

This will produce the `novac` executable.

## Running

You can test the compiler with the provided test file:

```bash
# Compile the Nova script to an object file (output.o)
./novac ../tests/factorial.nv

# Link the object file to create an executable
gcc output.o -o factorial.exe

# Run the executable!
./factorial.exe
```

The output should be `120`.

## Directory Structure
- `include/`: Header files defining the compiler APIs.
- `src/`: Source code for the lexer, parser, analyzer, codegen, and driver.
- `tests/`: Sample `.nv` source files.
