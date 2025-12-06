# ALU Library
# Arithmetic Logic Unit with flag operations

add_library(lvm_alu STATIC
    alu.cpp
)

target_include_directories(lvm_alu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../register/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
)

target_link_libraries(lvm_alu PUBLIC
    lvm_register
    lvm_helpers
)

# Tests
if(BUILD_TESTING)
    add_executable(lvm_alu_tests
        tests/alu_tests.cpp
    )
    
    target_link_libraries(lvm_alu_tests PRIVATE
        lvm_alu
        lvm_register
        lvm_helpers
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_alu_tests)
endif()
