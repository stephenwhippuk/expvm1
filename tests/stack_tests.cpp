#include <gtest/gtest.h>
#include "memsize.h"
#include "stack.h"
#include "memunit.h"
#include "accessMode.h"

using namespace lvm;

// Stack test fixture
class StackTest : public ::testing::Test {
protected:
    std::shared_ptr<Memory> memory;
    
    void SetUp() override {
        memory = std::make_shared<Memory>(256, 256); // page_size, pages
        memory->unprotected_mode(); // Required to create stack
    }
    
    std::unique_ptr<Stack> createStack(memsize_t size = 1024) {
        auto stack = std::make_unique<Stack>(memory, 0, 0x0000, size);
        memory->protected_mode(); // Switch to protected mode after creating stack
        return stack;
    }
};

// Test stack creation and initial state
TEST_F(StackTest, StackCreation) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    EXPECT_TRUE(accessor->is_empty());
    EXPECT_FALSE(accessor->is_full());
    EXPECT_EQ(accessor->get_size(), 0);
}

// Test push and pop byte operations
TEST_F(StackTest, PushPopByte) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push(0x42);
    EXPECT_FALSE(accessor->is_empty());
    EXPECT_EQ(accessor->peek_byte(), 0x42);
    
    byte_t value = accessor->pop();
    EXPECT_EQ(value, 0x42);
    EXPECT_TRUE(accessor->is_empty());
}

// Test push and pop word operations
TEST_F(StackTest, PushPopWord) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push_word(0xABCD);
    EXPECT_FALSE(accessor->is_empty());
    EXPECT_EQ(accessor->peek_word(), 0xABCD);
    
    word_t value = accessor->pop_word();
    EXPECT_EQ(value, 0xABCD);
    EXPECT_TRUE(accessor->is_empty());
}

// Test multiple push/pop operations
TEST_F(StackTest, MultiplePushPop) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push(0x11);
    accessor->push(0x22);
    accessor->push(0x33);
    
    EXPECT_EQ(accessor->pop(), 0x33);
    EXPECT_EQ(accessor->pop(), 0x22);
    EXPECT_EQ(accessor->pop(), 0x11);
    EXPECT_TRUE(accessor->is_empty());
}

// Test peek operations
TEST_F(StackTest, PeekOperations) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push(0x42);
    EXPECT_EQ(accessor->peek_byte(), 0x42);
    EXPECT_EQ(accessor->peek_byte(), 0x42); // Should still be 0x42
    
    accessor->pop();
    
    accessor->push_word(0x1234);
    EXPECT_EQ(accessor->peek_word(), 0x1234);
    EXPECT_EQ(accessor->peek_word(), 0x1234); // Should still be 0x1234
}

// Test stack underflow
TEST_F(StackTest, StackUnderflow) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    EXPECT_THROW(accessor->pop(), std::runtime_error);
}

// Test stack overflow
TEST_F(StackTest, StackOverflow) {
    auto stack = createStack(10); // Small stack
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Fill the stack
    for (int i = 0; i < 10; i++) {
        accessor->push(static_cast<byte_t>(i));
    }
    
    EXPECT_TRUE(accessor->is_full());
    EXPECT_THROW(accessor->push(0xFF), std::runtime_error);
}

// Test frame pointer operations
TEST_F(StackTest, FramePointerOperations) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some data
    accessor->push_word(0x1111);
    accessor->push_word(0x2222);
    
    // Set frame to top
    accessor->set_frame_to_top();
    
    // Push more data
    accessor->push_word(0x3333);
    
    // Frame register should point to where we set it
    addr_t frame_addr = accessor->get_frame_register();
    EXPECT_GT(frame_addr, 0);
}

// Test peek from base pointer
TEST_F(StackTest, PeekFromBase) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some values onto the stack
    accessor->push(0x11); // First push - goes to highest address
    accessor->push(0x22); // Second push
    accessor->push(0x33); // Third push - at stack top (lowest address of the three)
    
    // BP points to base (highest address = stack_size - 1)
    // Offset 0 from BP should give us the first pushed value (0x11)
    // Offset 1 from BP should give us the second pushed value (0x22)
    // Offset 2 from BP should give us the third pushed value (0x33)
    EXPECT_EQ(accessor->peek_byte_from_base(0), 0x11);
    EXPECT_EQ(accessor->peek_byte_from_base(1), 0x22);
    EXPECT_EQ(accessor->peek_byte_from_base(2), 0x33);
}

// Test peek word from base pointer
TEST_F(StackTest, PeekWordFromBase) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some words onto the stack
    accessor->push_word(0xABCD); // First word (word index 0)
    accessor->push_word(0x1234); // Second word (word index 1)
    
    // Offset N accesses the Nth word from the base
    // Offset 0 from BP should give us the first word (0xABCD)
    // Offset 1 from BP should give us the second word (0x1234)
    EXPECT_EQ(accessor->peek_word_from_base(0), 0xABCD);
    EXPECT_EQ(accessor->peek_word_from_base(1), 0x1234);
}

// Test read-only accessor mode
TEST_F(StackTest, ReadOnlyAccessor) {
    auto stack = createStack();
    auto rw_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some data with read-write accessor
    rw_accessor->push(0x42);
    rw_accessor->push_word(0xABCD);
    
    // Get a read-only accessor
    auto ro_accessor = stack->get_accessor(MemAccessMode::READ_ONLY);
    
    // Read operations should work
    EXPECT_EQ(ro_accessor->peek_word(), 0xABCD);
    EXPECT_FALSE(ro_accessor->is_empty());
    
    // Write operations should throw
    EXPECT_THROW(ro_accessor->pop(), std::runtime_error);
    EXPECT_THROW(ro_accessor->push(0xFF), std::runtime_error);
    EXPECT_THROW(ro_accessor->push_word(0xFFFF), std::runtime_error);
    EXPECT_THROW(ro_accessor->pop_word(), std::runtime_error);
}

// Test flush operation
TEST_F(StackTest, FlushOperation) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Push some data
    accessor->push(0x11);
    accessor->push(0x22);
    accessor->set_frame_to_top();
    accessor->push(0x33);
    accessor->push(0x44);
    
    // Flush should clear items after frame
    accessor->flush();
    
    // Stack should be at frame pointer
    EXPECT_TRUE(accessor->is_empty());
}

// Test mixed byte and word operations
TEST_F(StackTest, MixedOperations) {
    auto stack = createStack();
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->push(0x11);
    accessor->push_word(0x2233);
    accessor->push(0x44);
    
    EXPECT_EQ(accessor->pop(), 0x44);
    EXPECT_EQ(accessor->pop_word(), 0x2233);
    EXPECT_EQ(accessor->pop(), 0x11);
    EXPECT_TRUE(accessor->is_empty());
}
