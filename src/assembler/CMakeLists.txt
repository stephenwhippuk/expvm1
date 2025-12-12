cmake_minimum_required(VERSION 3.20)

# =============================================================================
# Assembler Library
# =============================================================================

# Collect all source files
set(ASSEMBLER_SOURCES
    lexer/token.cpp
    lexer/lexer.cpp
    parser/ast.cpp
    parser/parser.cpp
    semantic/symbol_table.cpp
    semantic/semantic_analyzer.cpp
    ir/code_graph.cpp
    ir/code_graph_builder.cpp
    codegen/address_resolver.cpp
    codegen/binary_writer.cpp
)

# Create the assembler library
add_library(lvm_assembler ${ASSEMBLER_SOURCES})

# Set include directories
target_include_directories(lvm_assembler
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..  # For #include "assembler/..."
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}     # For internal includes
)

# Set C++20 standard
set_target_properties(lvm_assembler PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Enable warnings
if(MSVC)
    target_compile_options(lvm_assembler PRIVATE /W4)
else()
    target_compile_options(lvm_assembler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# Tests
# =============================================================================

# Unit tests for lexer
add_executable(test_assembler_lexer
    tests/test_lexer.cpp
)

target_link_libraries(test_assembler_lexer
    PRIVATE
        lvm_assembler
        GTest::gtest_main
)

# Unit tests for parser
add_executable(test_assembler_parser
    tests/test_parser.cpp
)

target_link_libraries(test_assembler_parser
    PRIVATE
        lvm_assembler
        GTest::gtest_main
)

# Unit tests for semantic analyzer
add_executable(test_assembler_semantic
    tests/test_semantic.cpp
)

target_link_libraries(test_assembler_semantic
    PRIVATE
        lvm_assembler
        GTest::gtest_main
)

# Unit tests for code graph and address resolution
add_executable(test_assembler_codegen
    tests/test_codegen.cpp
)

target_link_libraries(test_assembler_codegen
    PRIVATE
        lvm_assembler
        GTest::gtest_main
)

# Unit tests for binary writer
add_executable(test_assembler_binary_writer
    tests/test_binary_writer.cpp
)

target_link_libraries(test_assembler_binary_writer
    PRIVATE
        lvm_assembler
        GTest::gtest_main
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(test_assembler_lexer)
gtest_discover_tests(test_assembler_parser)
gtest_discover_tests(test_assembler_semantic)
gtest_discover_tests(test_assembler_codegen)
gtest_discover_tests(test_assembler_binary_writer)
