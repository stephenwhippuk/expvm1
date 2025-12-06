#include <gtest/gtest.h>
#include "vmemunit.h"
#include "stack_accessor.h"
#include "vaddr.h"
#include "errors.h"
#include <stdexcept>

using namespace lvm;

// Test fixture for StackAccessor tests
class StackAccessorTest : public ::testing::Test {
protected:
    VMemUnit vmem_unit;
    context_id_t context_id;
    
    void SetUp() override {
        // Create a context for stack (64KB)
        context_id = vmem_unit.create_context(64 * 1024);
    }
};

// Test stack accessor creation
TEST_F(StackAccessorTest, AccessorCreation) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    ASSERT_NE(ctx, nullptr);
    
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    ASSERT_NE(accessor, nullptr);
    EXPECT_EQ(accessor->get_context_id(), context_id);
    EXPECT_EQ(accessor->get_size(), 64 * 1024);
}

// Test accessor creation fails in unprotected mode
TEST_F(StackAccessorTest, AccessorCreationUnprotectedFails) {
    // Leave in unprotected mode
    const Context* ctx = vmem_unit.get_context(context_id);
    ASSERT_NE(ctx, nullptr);
    
    EXPECT_THROW(ctx->create_stack_accessor(vmem_unit), std::runtime_error);
}

// Test byte read/write
TEST_F(StackAccessorTest, ByteReadWrite) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Write at address 0x100
    accessor->write_byte(0x100, 0x42);
    
    // Read it back
    byte_t value = accessor->read_byte(0x100);
    EXPECT_EQ(value, 0x42);
}

// Test byte operations at different addresses
TEST_F(StackAccessorTest, ByteOperationsMultipleAddresses) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Write to different addresses
    accessor->write_byte(0x0000, 0xAA);
    accessor->write_byte(0x1000, 0xBB);
    accessor->write_byte(0x5000, 0xCC);
    
    // Read them back
    EXPECT_EQ(accessor->read_byte(0x0000), 0xAA);
    EXPECT_EQ(accessor->read_byte(0x1000), 0xBB);
    EXPECT_EQ(accessor->read_byte(0x5000), 0xCC);
}

// Test word read/write (little-endian)
TEST_F(StackAccessorTest, WordReadWrite) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    accessor->write_word(0x400, 0x1234);
    
    word_t value = accessor->read_word(0x400);
    EXPECT_EQ(value, 0x1234);
    
    // Verify little-endian storage
    EXPECT_EQ(accessor->read_byte(0x400), 0x34);  // Low byte
    EXPECT_EQ(accessor->read_byte(0x401), 0x12);  // High byte
}

// Test multiple word operations
TEST_F(StackAccessorTest, MultipleWordOperations) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    accessor->write_word(0x100, 0xABCD);
    accessor->write_word(0x200, 0x1234);
    accessor->write_word(0x300, 0x5678);
    
    EXPECT_EQ(accessor->read_word(0x100), 0xABCD);
    EXPECT_EQ(accessor->read_word(0x200), 0x1234);
    EXPECT_EQ(accessor->read_word(0x300), 0x5678);
}

// Test address bounds checking - byte
TEST_F(StackAccessorTest, ByteBoundsChecking) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Last valid address
    uint32_t size = accessor->get_size();
    accessor->write_byte(size - 1, 0x42);
    EXPECT_EQ(accessor->read_byte(size - 1), 0x42);
    
    // Beyond bounds
    EXPECT_THROW(accessor->write_byte(size, 0x42), std::runtime_error);
    EXPECT_THROW(accessor->read_byte(size), std::runtime_error);
}

// Test address bounds checking - word
TEST_F(StackAccessorTest, WordBoundsChecking) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    uint32_t size = accessor->get_size();
    
    // Last valid word address (size - 2)
    accessor->write_word(size - 2, 0x1234);
    EXPECT_EQ(accessor->read_word(size - 2), 0x1234);
    
    // Word at size - 1 would overflow (needs 2 bytes)
    EXPECT_THROW(accessor->write_word(size - 1, 0x1234), std::runtime_error);
    EXPECT_THROW(accessor->read_word(size - 1), std::runtime_error);
}

// Test pre-allocation ensures all memory available
TEST_F(StackAccessorTest, PreAllocationComplete) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Should be able to write to any address without allocation delay
    // Test addresses across different blocks
    accessor->write_byte(0, 0x01);
    accessor->write_byte(4095, 0x02);   // End of first block
    accessor->write_byte(4096, 0x03);   // Start of second block
    accessor->write_byte(8192, 0x04);   // Start of third block
    
    EXPECT_EQ(accessor->read_byte(0), 0x01);
    EXPECT_EQ(accessor->read_byte(4095), 0x02);
    EXPECT_EQ(accessor->read_byte(4096), 0x03);
    EXPECT_EQ(accessor->read_byte(8192), 0x04);
}

// Test default read returns zero (from pre-allocated memory)
TEST_F(StackAccessorTest, DefaultValueZero) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Unwritten memory should be zero-initialized
    EXPECT_EQ(accessor->read_byte(0x1000), 0);
    EXPECT_EQ(accessor->read_word(0x2000), 0);
}

// Test operations in protected mode work
TEST_F(StackAccessorTest, ProtectedModeOperations) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Operations should work in protected mode
    accessor->write_byte(0x100, 0x42);
    EXPECT_EQ(accessor->read_byte(0x100), 0x42);
    
    accessor->write_word(0x200, 0x1234);
    EXPECT_EQ(accessor->read_word(0x200), 0x1234);
}

// Test 32-bit addressing (direct, no page translation)
TEST_F(StackAccessorTest, DirectAddressing) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    // Use full 32-bit addresses within bounds
    accessor->write_byte(0x00000100, 0xAA);
    accessor->write_byte(0x00001000, 0xBB);
    accessor->write_byte(0x0000FFFF, 0xCC);
    
    EXPECT_EQ(accessor->read_byte(0x00000100), 0xAA);
    EXPECT_EQ(accessor->read_byte(0x00001000), 0xBB);
    EXPECT_EQ(accessor->read_byte(0x0000FFFF), 0xCC);
}

// Test small stack context
TEST_F(StackAccessorTest, SmallStackContext) {
    // Create a small 1KB stack
    context_id_t small_ctx = vmem_unit.create_context(1024);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    const Context* ctx = vmem_unit.get_context(small_ctx);
    auto accessor = ctx->create_stack_accessor(vmem_unit);
    
    EXPECT_EQ(accessor->get_size(), 1024);
    
    // Can access within bounds
    accessor->write_byte(0, 0x01);
    accessor->write_byte(1023, 0x02);
    
    EXPECT_EQ(accessor->read_byte(0), 0x01);
    EXPECT_EQ(accessor->read_byte(1023), 0x02);
    
    // Cannot access beyond bounds
    EXPECT_THROW(accessor->read_byte(1024), std::runtime_error);
}
