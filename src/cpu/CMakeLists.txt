# CPU Library
# Central Processing Unit coordinating all operations

add_library(lvm_cpu STATIC
    cpu.cpp
    cpu_alu_ops.cpp
)

target_include_directories(lvm_cpu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../register/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../alu/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../stack/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../instruction_unit/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../helpers/include
)

target_link_libraries(lvm_cpu PUBLIC
    lvm_memunit
    lvm_register
    lvm_alu
    lvm_stack
    lvm_instruction_unit
    lvm_helpers
)
