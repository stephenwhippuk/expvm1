# Stack Library
# Stack implementation with frame pointer support

add_library(lvm_stack STATIC
    stack.cpp
    stack_access.cpp
    stack_new.cpp
)

target_include_directories(lvm_stack PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../register/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

target_link_libraries(lvm_stack PUBLIC
    lvm_memunit
    lvm_register
)

# Tests
if(BUILD_TESTING)
    add_executable(lvm_stack_tests
        tests/stack_tests.cpp
        tests/stack_new_tests.cpp
    )
    
    target_link_libraries(lvm_stack_tests PRIVATE
        lvm_stack
        lvm_memunit
        lvm_register
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_stack_tests)
endif()
