#include <gtest/gtest.h>
#include "basic_io.h"
#include "vmemunit.h"
#include "stack.h"

using namespace lvm;

// Test fixture for BasicIO tests
class BasicIOTest : public ::testing::Test {
protected:
    std::shared_ptr<VMemUnit> vmem_unit;
    std::shared_ptr<Stack> stack;
    
    void SetUp() override {
        // Setup test environment
        vmem_unit = std::make_shared<VMemUnit>();
        stack = std::make_shared<Stack>(vmem_unit, 1024);
    }
};

// Basic creation test
TEST_F(BasicIOTest, Creation) {
    // Test that BasicIO can be created
    BasicIO io(vmem_unit, stack);
    SUCCEED();
}

// TODO: Add more tests
