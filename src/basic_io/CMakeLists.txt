# Basic I/O Library
# Basic input/output operations for the virtual machine

add_library(lvm_basic_io STATIC
    basic_io.cpp
)

target_include_directories(lvm_basic_io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../stack/include
)

target_link_libraries(lvm_basic_io PUBLIC
    lvm_helpers
    lvm_memunit
    lvm_stack
)

# Tests
if(BUILD_TESTING)
    add_executable(lvm_basic_io_tests
        tests/basic_io_tests.cpp
    )
    
    target_link_libraries(lvm_basic_io_tests PRIVATE
        lvm_basic_io
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_basic_io_tests)
endif()
