# Memory Unit Library
# Core memory management with paging and access control

add_library(lvm_memunit STATIC
    memunit.cpp
    mem_access.cpp
    memory2.cpp
    context.cpp
    vmemunit.cpp
    paged_memory_accessor.cpp
    stack_accessor.cpp
)

target_include_directories(lvm_memunit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

# Depends on helpers for error handling

# Tests
if(BUILD_TESTING)
    add_executable(lvm_memunit_tests
        tests/memory2_tests.cpp
        tests/memunit_tests.cpp
        tests/paged_memory_accessor_tests.cpp
        tests/stack_accessor_tests.cpp
    )
    
    target_link_libraries(lvm_memunit_tests PRIVATE
        lvm_memunit
        lvm_helpers
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(lvm_memunit_tests)
endif()
