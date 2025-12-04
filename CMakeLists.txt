cmake_minimum_required(VERSION 3.20)
project(lvm VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(include)

# Collect library source files (exclude main programs)
file(GLOB LIB_SOURCES 
    "src/memunit.cpp" 
    "src/mem_access.cpp" 
    "src/stack.cpp" 
    "src/stack_access.cpp"
    "src/cpu.cpp" 
    "src/cpu_alu_ops.cpp"
    "src/vm.cpp"
    "src/alu.cpp"
    "src/register.cpp"
    "src/flags.cpp"
    "src/helpers.cpp"
    "src/instruction_unit.cpp"
    "src/instruction_unit_access.cpp"
)

# Add the VM test executable (original main.cpp)
add_executable(lvm src/main.cpp ${LIB_SOURCES})

# Add test executable
add_executable(lvm_tests 
    tests/test_main.cpp
    ${LIB_SOURCES}
)

target_link_libraries(lvm_tests 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lvm_tests)

# # Add the assembler executable
# add_executable(slmasm src/assembler.cpp)

# # Add the VM runner executable
# add_executable(slmvm src/vm.cpp ${LIB_SOURCES})
