cmake_minimum_required(VERSION 3.10)
project(NovaCompiler C CXX)

set(CMAKE_C_STANDARD 11)

# Ensure LLVM is found
# You may need to run `cmake -DLLVM_DIR=C:/path/to/llvm/lib/cmake/llvm ..` if it's not in PATH
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

include_directories(include)

# Find all source files
file(GLOB SOURCES "src/*.c")

add_executable(novac ${SOURCES})

# Map required LLVM components to library names
# This is typically how we link with LLVM in CMake
target_link_libraries(novac LLVM-21 stdc++)

# Add test files to build environment (optional, for easy access)
file(GLOB TESTS "tests/*.nv")
