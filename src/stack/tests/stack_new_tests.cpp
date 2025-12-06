#include <gtest/gtest.h>
#include "stack_new.h"
#include "vmemunit.h"
#include "errors.h"
#include <stdexcept>

using namespace lvm;

// Test fixture for new Stack tests
class StackNewTest : public ::testing::Test {
protected:
    VMemUnit vmem_unit;
    
    void SetUp() override {
        // Tests start in unprotected mode
    }
};

// Test stack creation in unprotected mode
TEST_F(StackNewTest, StackCreation) {
    Stack2 stack(vmem_unit, 1024);
    
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    EXPECT_TRUE(accessor->is_empty());
    EXPECT_EQ(accessor->get_size(), 0);
    EXPECT_EQ(accessor->get_capacity(), 1024);
    EXPECT_EQ(accessor->get_sp(), 0);
    EXPECT_EQ(accessor->get_fp(), -1);
}

// Test stack creation fails in protected mode
TEST_F(StackNewTest, CreationInProtectedModeFails) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    EXPECT_THROW(Stack2 stack(vmem_unit, 1024), lvm::runtime_error);
}

// Test accessor creation fails in unprotected mode
TEST_F(StackNewTest, AccessorCreationInUnprotectedModeFails) {
    Stack2 stack(vmem_unit, 1024);
    // Still in unprotected mode
    EXPECT_THROW(stack.get_accessor(MemAccessMode::READ_WRITE), lvm::runtime_error);
}

// Test push and pop byte
TEST_F(StackNewTest, PushPopByte) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_byte(0x42);
    EXPECT_FALSE(accessor->is_empty());
    EXPECT_EQ(accessor->get_size(), 1);
    EXPECT_EQ(accessor->get_sp(), 1);
    
    byte_t value = accessor->pop_byte();
    EXPECT_EQ(value, 0x42);
    EXPECT_TRUE(accessor->is_empty());
    EXPECT_EQ(accessor->get_sp(), 0);
}

// Test push and pop word
TEST_F(StackNewTest, PushPopWord) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_word(0x1234);
    EXPECT_EQ(accessor->get_size(), 2);
    EXPECT_EQ(accessor->get_sp(), 2);
    
    word_t value = accessor->pop_word();
    EXPECT_EQ(value, 0x1234);
    EXPECT_EQ(accessor->get_sp(), 0);
}

// Test multiple pushes
TEST_F(StackNewTest, MultiplePushes) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_byte(0x10);
    accessor->push_byte(0x20);
    accessor->push_byte(0x30);
    
    EXPECT_EQ(accessor->get_size(), 3);
    EXPECT_EQ(accessor->pop_byte(), 0x30);
    EXPECT_EQ(accessor->pop_byte(), 0x20);
    EXPECT_EQ(accessor->pop_byte(), 0x10);
}

// Test stack grows upward from 0
TEST_F(StackNewTest, GrowsUpward) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    EXPECT_EQ(accessor->get_sp(), 0);
    accessor->push_byte(0x11);
    EXPECT_EQ(accessor->get_sp(), 1);
    accessor->push_byte(0x22);
    EXPECT_EQ(accessor->get_sp(), 2);
    accessor->push_byte(0x33);
    EXPECT_EQ(accessor->get_sp(), 3);
}

// Test peek operations
TEST_F(StackNewTest, PeekOperations) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_byte(0xAA);
    accessor->push_byte(0xBB);
    
    EXPECT_EQ(accessor->peek_byte(), 0xBB);
    EXPECT_EQ(accessor->get_size(), 2);  // Peek doesn't change size
}

// Test peek from base (absolute addressing from 0)
TEST_F(StackNewTest, PeekFromBase) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_byte(0x11);  // Address 0
    accessor->push_byte(0x22);  // Address 1
    accessor->push_byte(0x33);  // Address 2
    
    EXPECT_EQ(accessor->peek_byte_from_base(0), 0x11);
    EXPECT_EQ(accessor->peek_byte_from_base(1), 0x22);
    EXPECT_EQ(accessor->peek_byte_from_base(2), 0x33);
}

// Test frame pointer sits at -1 relative to frame
TEST_F(StackNewTest, FramePointerBehavior) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some values
    accessor->push_byte(0x10);  // Address 0
    accessor->push_byte(0x20);  // Address 1
    accessor->push_byte(0x30);  // Address 2
    
    // Set frame pointer
    accessor->set_frame_pointer(1);  // FP sits at address 1
    EXPECT_EQ(accessor->get_fp(), 1);
    
    // First frame position is FP+1 = 2, which contains 0x30
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 0x30);
}

// Test set frame to top
TEST_F(StackNewTest, SetFrameToTop) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_byte(0x10);
    accessor->push_byte(0x20);
    accessor->push_byte(0x30);
    // SP now at 3
    
    accessor->set_frame_to_top();
    // FP should be at SP-1 = 2
    EXPECT_EQ(accessor->get_fp(), 2);
    
    // Now push more values above the frame
    accessor->push_byte(0x40);  // Address 3
    accessor->push_byte(0x50);  // Address 4
    
    // Frame position 0 (address FP+1 = 3) should contain 0x40
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 0x40);
    EXPECT_EQ(accessor->peek_byte_from_frame(1), 0x50);
}

// Test overflow detection
TEST_F(StackNewTest, OverflowDetection) {
    Stack2 stack(vmem_unit, 10);  // Small capacity
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Fill the stack
    for (int i = 0; i < 10; ++i) {
        accessor->push_byte(i);
    }
    
    EXPECT_TRUE(accessor->is_full());
    EXPECT_THROW(accessor->push_byte(0xFF), lvm::runtime_error);
}

// Test underflow detection
TEST_F(StackNewTest, UnderflowDetection) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    EXPECT_TRUE(accessor->is_empty());
    EXPECT_THROW(accessor->pop_byte(), lvm::runtime_error);
}

// Test flush - should only flush current frame, preserving data below frame pointer
TEST_F(StackNewTest, FlushStack) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some data
    accessor->push_byte(0x11);
    accessor->push_byte(0x22);
    accessor->push_byte(0x33);
    accessor->push_byte(0x44);
    
    // Set frame pointer at position 1 (protecting 0x11 and 0x22 below it)
    accessor->set_frame_pointer(1);
    
    // Push more data in the new frame
    accessor->push_byte(0x55);
    accessor->push_byte(0x66);
    
    EXPECT_EQ(accessor->get_sp(), 6);
    EXPECT_EQ(accessor->get_fp(), 1);
    
    // Flush should only clear the current frame
    accessor->flush();
    
    EXPECT_TRUE(accessor->is_empty());  // Empty relative to current frame
    EXPECT_EQ(accessor->get_sp(), 2);   // SP moves back to fp_ + 1
    EXPECT_EQ(accessor->get_fp(), 1);   // FP unchanged
    
    // Data below frame should still be accessible via peek_from_base
    EXPECT_EQ(accessor->peek_byte_from_base(0), 0x11);
    EXPECT_EQ(accessor->peek_byte_from_base(1), 0x22);
}

// Test read-only accessor
TEST_F(StackNewTest, ReadOnlyAccessor) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    
    // Use read-write to set up
    {
        auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
        accessor->push_byte(0xAA);
        accessor->push_byte(0xBB);
    }
    
    // Use read-only accessor
    auto ro_accessor = stack.get_accessor(MemAccessMode::READ_ONLY);
    
    // Reading should work
    EXPECT_EQ(ro_accessor->peek_byte(), 0xBB);
    EXPECT_EQ(ro_accessor->peek_byte_from_base(0), 0xAA);
    
    // Writing should fail
    EXPECT_THROW(ro_accessor->push_byte(0xCC), lvm::runtime_error);
    EXPECT_THROW(ro_accessor->pop_byte(), lvm::runtime_error);
    EXPECT_THROW(ro_accessor->flush(), lvm::runtime_error);
}

// Test word operations with little-endian
TEST_F(StackNewTest, WordLittleEndian) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_word(0x1234);
    
    // Verify little-endian storage
    EXPECT_EQ(accessor->peek_byte_from_base(0), 0x34);  // Low byte
    EXPECT_EQ(accessor->peek_byte_from_base(1), 0x12);  // High byte
}

// Test peek word from frame
TEST_F(StackNewTest, PeekWordFromFrame) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_word(0xAAAA);  // Addresses 0-1
    accessor->push_word(0xBBBB);  // Addresses 2-3
    
    accessor->set_frame_pointer(1);  // FP at 1, frame starts at 2
    
    EXPECT_EQ(accessor->peek_word_from_frame(0), 0xBBBB);
}
