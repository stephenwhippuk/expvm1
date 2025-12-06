# Register and Flags Library
# Register implementation with flag support

add_library(lvm_register STATIC
    register.cpp
    flags.cpp
)

target_include_directories(lvm_register PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

target_link_libraries(lvm_register PUBLIC
    lvm_helpers
)

# Tests
if(BUILD_TESTING)
    add_executable(lvm_register_tests
        tests/register_tests.cpp
    )
    
    target_link_libraries(lvm_register_tests PRIVATE
        lvm_register
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_register_tests)
endif()
