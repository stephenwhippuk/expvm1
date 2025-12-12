# VM Library
# Virtual Machine orchestrator

add_library(lvm_vm STATIC
    vm.cpp
    binary_loader.cpp
)

target_include_directories(lvm_vm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpu/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

target_link_libraries(lvm_vm PUBLIC
    lvm_cpu
    lvm_memunit
)

# Add tests subdirectory
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
