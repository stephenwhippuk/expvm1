cmake_minimum_required(VERSION 3.20)
project(lvm VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Collect library source files (exclude main programs)
file(GLOB LIB_SOURCES "src/memunit.cpp" "src/mem_access.cpp" "src/stack.cpp" "src/cpu.cpp" "src/vm.cpp")
# Add the VM test executable (original main.cpp)
add_executable(lvm src/main.cpp ${LIB_SOURCES})

# # Add the assembler executable
# add_executable(slmasm src/assembler.cpp)

# # Add the VM runner executable
# add_executable(slmvm src/vm.cpp ${LIB_SOURCES})
