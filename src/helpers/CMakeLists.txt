# Helper/Utilities Library
# Common utility functions and error handling

add_library(lvm_helpers STATIC
    helpers.cpp
)

target_include_directories(lvm_helpers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../memunit/include
)

# Depends on memunit for type definitions
