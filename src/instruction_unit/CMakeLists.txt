# Instruction Unit Library
# Instruction execution and program control

add_library(lvm_instruction_unit STATIC
    instruction_unit.cpp
    instruction_unit_access.cpp
)

target_include_directories(lvm_instruction_unit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../stack/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../register/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

target_link_libraries(lvm_instruction_unit PUBLIC
    lvm_memunit
    lvm_stack
    lvm_register
)

# Tests
if(BUILD_TESTING)
    add_executable(lvm_instruction_unit_tests
        tests/instruction_unit_tests.cpp
    )
    
    target_link_libraries(lvm_instruction_unit_tests PRIVATE
        lvm_instruction_unit
        lvm_stack
        lvm_memunit
        lvm_register
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_instruction_unit_tests)
endif()
