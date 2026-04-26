# Nova Compiler 🚀

![C Standard](https://img.shields.io/badge/Language-C11-blue.svg)
![Backend](https://img.shields.io/badge/Backend-LLVM-red.svg)
![Build](https://img.shields.io/badge/Build-CMake-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)

An **End-to-End Compiler** for a custom, statically typed Domain-Specific Language (DSL) named **Nova**. This compiler is built entirely from scratch in C and leverages the powerful **LLVM C API** to generate highly optimized, native machine code executables.

Designed with modularity, scalability, and clean architecture in mind, this project demonstrates a complete compilation pipeline from raw source text to a native OS executable.

---

## 🌟 Key Features

* **Custom Lexer (Scanner):** Efficiently tokenizes the DSL source, handling keywords, identifiers, types, and mathematical operators.
* **Recursive Descent Parser:** Constructs a strongly-typed Abstract Syntax Tree (AST) representing the program's structural flow.
* **Semantic Analyzer:** Features a robust symbol table for variable scoping, duplicate detection, and strict static type checking.
* **LLVM Code Generation:** Traverses the verified AST and maps constructs directly to LLVM Intermediate Representation (IR).
* **Native Backend Emitter:** Compiles the IR into native object files (`.o`), ready to be linked into standalone executables.

---

## 💻 Sample Code

Nova features a clean, C/Rust-inspired syntax. Here is a sample program that calculates the factorial of a number using recursion:

```rust
// factorial.nv
fn factorial(n: int) -> int {
    if (n == 0) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

fn main() -> int {
    let result: int = factorial(5);
    print(result);  // Output: 120
    return 0;
}
```

---

## 🏗️ Architecture Pipeline

The compiler follows a traditional frontend-backend pipeline architecture:

1. `source.nv` → **Lexer** → *Token Stream*
2. *Token Stream* → **Parser** → *Abstract Syntax Tree (AST)*
3. *AST* → **Semantic Analyzer** → *Type-Checked AST*
4. *Type-Checked AST* → **LLVM Codegen** → *LLVM IR*
5. *LLVM IR* → **LLVM Target Machine** → `output.o` (Native Object File)

---

## 🛠️ Building the Compiler

### Windows Setup (MSYS2)

Building a C project that links with LLVM on Windows requires [MSYS2](https://www.msys2.org/). 
1. Open the **MSYS2 MinGW x64** terminal.
2. Install the necessary dependencies:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-llvm make
```

### Compilation

Clone the repository and build the compiler using CMake:

```bash
git clone https://github.com/sameera0602/compiler.git
cd compiler
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```

This will produce the `novac` executable compiler.

---

## 🚀 Running

To compile a Nova script into a native executable:

```bash
# 1. Compile the Nova script to an object file
./novac ../tests/factorial.nv

# 2. Link the object file into an executable
gcc output.o -o factorial.exe

# 3. Run the resulting program!
./factorial.exe
```

---

## 📁 Repository Structure

```
├── CMakeLists.txt        # Build system configuration
├── include/              # Public API headers for the compiler phases
│   ├── ast.h
│   ├── codegen.h
│   ├── lexer.h
│   ├── parser.h
│   └── semantic.h
├── src/                  # Implementation of the compiler phases
│   ├── ast.c             # AST memory management
│   ├── codegen.c         # LLVM IR emitter
│   ├── lexer.c           # Tokenizer
│   ├── main.c            # Compiler driver
│   ├── parser.c          # Recursive descent parser
│   └── semantic.c        # Scope & type checking
└── tests/
    └── factorial.nv      # Sample Nova script
```

---

<p align="center">
  <i>Built for learning and open-source contribution.</i>
</p>
