#include <gtest/gtest.h>
#include "stack_new.h"
#include "vmemunit.h"
#include "errors.h"

using namespace lvm;

// Test fixture for subroutine-related stack operations
class StackSubroutineTest : public ::testing::Test {
protected:
    VMemUnit vmem_unit;
    
    void SetUp() override {
        // Tests start in unprotected mode
    }
};

// Test basic call setup - push flag and set frame
TEST_F(StackSubroutineTest, CallSetup) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Simulate call without return value
    accessor->push_byte(0);  // has_return_value flag = false
    accessor->set_frame_to_top();
    
    EXPECT_EQ(accessor->get_sp(), 1);
    EXPECT_EQ(accessor->get_fp(), 0);  // FP at -1 relative, so FP+1 = 1 (where flag is)
    EXPECT_TRUE(accessor->is_empty());  // Empty relative to frame
}

// Test call with return value flag
TEST_F(StackSubroutineTest, CallWithReturnValue) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Simulate call with return value
    accessor->push_byte(1);  // has_return_value flag = true
    accessor->set_frame_to_top();
    
    EXPECT_EQ(accessor->get_sp(), 1);
    EXPECT_EQ(accessor->get_fp(), 0);
    
    // Frame offset 0 accesses where FP points (the flag)
    byte_t flag = accessor->peek_byte_from_frame(0);
    EXPECT_EQ(flag, 1);
}

// Test subroutine pushes local variables
TEST_F(StackSubroutineTest, SubroutineLocalVariables) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Setup call frame
    accessor->push_byte(0);  // no return value
    accessor->set_frame_to_top();
    
    // Subroutine pushes local variables
    accessor->push_byte(0x11);
    accessor->push_byte(0x22);
    accessor->push_word(0x3344);
    
    EXPECT_EQ(accessor->get_sp(), 5);
    EXPECT_FALSE(accessor->is_empty());
    
    // Can peek locals relative to frame
    // Frame offset 0 is the flag, locals start at offset 1
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 0);     // flag
    EXPECT_EQ(accessor->peek_byte_from_frame(1), 0x11);  // first local
    EXPECT_EQ(accessor->peek_byte_from_frame(2), 0x22);  // second local
    EXPECT_EQ(accessor->peek_word_from_frame(3), 0x3344); // word local
}

// Test return without return value - flush clears locals
TEST_F(StackSubroutineTest, ReturnWithoutValue) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Setup call frame
    accessor->push_byte(0);  // no return value
    accessor->set_frame_to_top();
    
    // Subroutine pushes locals
    accessor->push_byte(0x11);
    accessor->push_byte(0x22);
    
    EXPECT_EQ(accessor->get_sp(), 3);
    
    // Check flag before return
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 0);
    
    // Return: flush clears locals (back to frame start which is fp+1)
    accessor->flush();
    
    EXPECT_TRUE(accessor->is_empty());
    EXPECT_EQ(accessor->get_sp(), 1);  // Back to fp+1 (just after flag)
}

// Test return with return value
TEST_F(StackSubroutineTest, ReturnWithValue) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Setup call frame
    accessor->push_byte(1);  // has return value
    accessor->set_frame_to_top();
    
    // Subroutine pushes locals
    accessor->push_byte(0x11);
    accessor->push_byte(0x22);
    
    // Subroutine pushes return value
    accessor->push_word(0xABCD);
    
    EXPECT_EQ(accessor->get_sp(), 5);
    
    // Check flag
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 1);
    
    // Return: pop return value first
    word_t return_value = accessor->pop_word();
    EXPECT_EQ(return_value, 0xABCD);
    
    // Then flush locals (back to fp+1)
    accessor->flush();
    
    EXPECT_EQ(accessor->get_sp(), 1);  // Back to fp+1 (just after flag)
}

// Test nested calls - multiple frames
TEST_F(StackSubroutineTest, NestedCalls) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // First call
    accessor->push_byte(0);  // no return value
    int32_t frame1_fp = accessor->get_sp() - 1;
    accessor->set_frame_to_top();
    
    accessor->push_byte(0x11);  // local var
    EXPECT_EQ(accessor->get_sp(), 2);
    EXPECT_EQ(accessor->peek_byte_from_frame(1), 0x11);  // Local at offset 1
    
    // Nested call
    accessor->push_byte(1);  // has return value
    int32_t frame2_fp = accessor->get_sp() - 1;
    accessor->set_frame_to_top();
    
    accessor->push_byte(0x22);  // nested local
    accessor->push_word(0x9999);  // return value
    
    EXPECT_EQ(accessor->get_sp(), 6);
    EXPECT_EQ(accessor->get_fp(), frame2_fp);
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 1);  // Flag
    EXPECT_EQ(accessor->peek_byte_from_frame(1), 0x22);  // Nested local
    
    // Return from nested - pop return value
    word_t ret_val = accessor->pop_word();
    EXPECT_EQ(ret_val, 0x9999);
    
    // Flush nested frame (back to fp+1)
    accessor->flush();
    EXPECT_EQ(accessor->get_sp(), 3);  // Back to just after nested flag
    
    // Restore frame1
    accessor->set_frame_pointer(frame1_fp);
    accessor->pop_byte();  // Pop nested call flag
    
    EXPECT_EQ(accessor->get_sp(), 2);
    EXPECT_EQ(accessor->get_fp(), frame1_fp);
    
    // Can still access first frame's local
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 0);  // Flag
    EXPECT_EQ(accessor->peek_byte_from_frame(1), 0x11);  // Local
}

// Test frame protects data below it
TEST_F(StackSubroutineTest, FrameProtectsData) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some data before call
    accessor->push_byte(0xAA);
    accessor->push_byte(0xBB);
    
    // Setup call frame at current position
    accessor->push_byte(0);  // no return value
    accessor->set_frame_to_top();
    
    // Now frame protects the 0xAA and 0xBB
    EXPECT_EQ(accessor->get_fp(), 2);
    
    // Subroutine can't pop below frame
    EXPECT_TRUE(accessor->is_empty());  // Empty relative to frame
    EXPECT_THROW(accessor->pop_byte(), lvm::runtime_error);
    
    // But can peek from base to see protected data
    EXPECT_EQ(accessor->peek_byte_from_base(0), 0xAA);
    EXPECT_EQ(accessor->peek_byte_from_base(1), 0xBB);
    EXPECT_EQ(accessor->peek_byte_from_base(2), 0);  // The flag
}

// Test complete call/return sequence
TEST_F(StackSubroutineTest, CompleteCallReturnSequence) {
    Stack2 stack(vmem_unit, 1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);
    
    // Caller pushes arguments
    accessor->push_word(0x1234);  // arg1
    accessor->push_word(0x5678);  // arg2
    
    // Save frame pointer for later restoration
    int32_t caller_fp = accessor->get_fp();
    EXPECT_EQ(caller_fp, -1);  // Initial frame
    
    // Call: push flag and set new frame
    accessor->push_byte(1);  // has return value
    accessor->set_frame_to_top();
    
    int32_t callee_fp = accessor->get_fp();
    EXPECT_EQ(callee_fp, 4);  // After 2 words + flag
    
    // Subroutine can access arguments from previous frame via base
    EXPECT_EQ(accessor->peek_word_from_base(0), 0x1234);
    EXPECT_EQ(accessor->peek_word_from_base(2), 0x5678);
    EXPECT_EQ(accessor->peek_byte_from_frame(0), 1);  // Flag at frame offset 0
    
    // Subroutine does work, pushes locals
    accessor->push_byte(0xAA);
    accessor->push_byte(0xBB);
    
    // Subroutine computes and pushes return value
    accessor->push_word(0xABCD);
    
    EXPECT_EQ(accessor->get_sp(), 9);  // 4 (flag) + 1 (0xAA) + 1 (0xBB) + 2 (return word) + 1 (fp+1)
    
    // Return: extract return value
    word_t ret_val = accessor->pop_word();
    EXPECT_EQ(ret_val, 0xABCD);
    
    // Flush locals
    accessor->flush();
    EXPECT_EQ(accessor->get_sp(), 5);  // Back to flag position
    
    // Restore caller's frame
    accessor->set_frame_pointer(caller_fp);
    
    // Pop the call flag
    accessor->pop_byte();
    EXPECT_EQ(accessor->get_sp(), 4);
    
    // Push return value back for caller
    accessor->push_word(ret_val);
    EXPECT_EQ(accessor->get_sp(), 6);
    
    // Caller pops return value
    word_t result = accessor->pop_word();
    EXPECT_EQ(result, 0xABCD);
    
    // Caller cleans up arguments
    accessor->pop_word();  // arg2
    accessor->pop_word();  // arg1
    
    EXPECT_EQ(accessor->get_sp(), 0);
    EXPECT_EQ(accessor->get_fp(), -1);
}
