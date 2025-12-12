# VM Unit Tests

add_executable(lvm_vm_tests
    binary_loader_tests.cpp
)

target_link_libraries(lvm_vm_tests
    lvm_vm
    lvm_cpu
    lvm_memunit
    lvm_helpers
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lvm_vm_tests)
