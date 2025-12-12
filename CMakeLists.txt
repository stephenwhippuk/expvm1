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
set(BUILD_TESTING ON)

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

# Legacy include directory (for main.cpp and transition period)
# TODO: Eventually migrate all includes to module-specific includes
include_directories(include)

# =============================================================================
# Add all library subdirectories
# =============================================================================
# Each library has its own CMakeLists.txt defining its build configuration
# and dependencies. Libraries are built in dependency order.

add_subdirectory(src/memunit)
add_subdirectory(src/helpers)
add_subdirectory(src/register)
add_subdirectory(src/alu)
add_subdirectory(src/stack)
add_subdirectory(src/instruction_unit)
add_subdirectory(src/basic_io)
add_subdirectory(src/cpu)
add_subdirectory(src/vm)
add_subdirectory(src/assembler)

# =============================================================================
# Executables
# =============================================================================

# Add the VM test executable (original main.cpp)
add_executable(lvm src/main.cpp)

target_link_libraries(lvm PRIVATE
    lvm_vm
    lvm_cpu
    lvm_memunit
)

# Binary creation tools
add_executable(create_binary tools/create_binary.cpp)
target_link_libraries(create_binary PRIVATE lvm_helpers)

add_executable(create_binary_with_data tools/create_binary_with_data.cpp)
target_link_libraries(create_binary_with_data PRIVATE lvm_helpers)

add_executable(create_hello_world tools/create_hello_world.cpp)
target_link_libraries(create_hello_world PRIVATE lvm_helpers)

# Parser test tool
add_executable(test_parser tools/test_parser.cpp)
target_link_libraries(test_parser PRIVATE lvm_assembler)

# Assembler tool
add_executable(asm tools/asm.cpp)
target_link_libraries(asm PRIVATE lvm_assembler)
target_include_directories(asm PRIVATE ${CMAKE_SOURCE_DIR}/src)

# =============================================================================
# Legacy Test Support (Optional - can be removed once all tests are modular)
# =============================================================================
# Individual library tests are now defined in their respective CMakeLists.txt
# This section can be kept for integration tests or removed entirely

# # Add the assembler executable
# add_executable(slmasm src/assembler.cpp)

# # Add the VM runner executable
# add_executable(slmvm src/vm.cpp ${LIB_SOURCES})
