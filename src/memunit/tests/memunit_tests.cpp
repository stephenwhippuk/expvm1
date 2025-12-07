#include <gtest/gtest.h>
#include "vmemunit.h"
#include "context.h"
#include "paged_memory_accessor.h"
#include "vaddr.h"
#include "errors.h"
#include <stdexcept>

using namespace lvm;

// Test fixture for VMemUnit tests
class VMemUnitTest : public ::testing::Test {
protected:
    lvm::VMemUnit memunit;
};

// Test initial state
TEST_F(VMemUnitTest, InitialState) {
    EXPECT_EQ(memunit.get_mode(), IVMemUnit::Mode::UNPROTECTED);
    EXPECT_TRUE(memunit.is_unprotected());
    EXPECT_FALSE(memunit.is_protected());
}

// Test mode switching
TEST_F(VMemUnitTest, ModeSwitch) {
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    EXPECT_EQ(memunit.get_mode(), IVMemUnit::Mode::PROTECTED);
    EXPECT_TRUE(memunit.is_protected());
    EXPECT_FALSE(memunit.is_unprotected());
    
    memunit.set_mode(IVMemUnit::Mode::UNPROTECTED);
    EXPECT_EQ(memunit.get_mode(), IVMemUnit::Mode::UNPROTECTED);
}

// Test context creation in unprotected mode (integration test using accessor)
TEST_F(VMemUnitTest, CreateContextUnprotected) {
    context_id_t id = memunit.create_context(1024);
    EXPECT_EQ(id, 0);
    
    auto ctx = memunit.get_context(id);
    ASSERT_NE(ctx, nullptr);
    
    // Verify context works by creating an accessor in PROTECTED mode
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    ASSERT_NE(accessor, nullptr);
    
    // Write and read through accessor to verify context is properly initialized
    accessor->set_page(0);
    accessor->write_byte(0, 0x42);
    EXPECT_EQ(accessor->read_byte(0), 0x42);
}

// Test context creation fails in protected mode
TEST_F(VMemUnitTest, CreateContextProtectedFails) {
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    EXPECT_THROW(memunit.create_context(1024), std::runtime_error);
}

// Test creating multiple contexts (integration test using accessors)
TEST_F(VMemUnitTest, CreateMultipleContexts) {
    context_id_t id1 = memunit.create_context(1024);
    context_id_t id2 = memunit.create_context(2048);
    context_id_t id3 = memunit.create_context(512);
    
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(id3, 2);
    
    auto ctx1 = memunit.get_context(id1);
    auto ctx2 = memunit.get_context(id2);
    auto ctx3 = memunit.get_context(id3);
    
    ASSERT_NE(ctx1, nullptr);
    ASSERT_NE(ctx2, nullptr);
    ASSERT_NE(ctx3, nullptr);
    
    // Verify contexts are independent by writing different values through accessors
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    
    auto acc1 = ctx1->create_paged_accessor(MemAccessMode::READ_WRITE);
    auto acc2 = ctx2->create_paged_accessor(MemAccessMode::READ_WRITE);
    auto acc3 = ctx3->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    acc1->set_page(0);
    acc2->set_page(0);
    acc3->set_page(0);
    
    acc1->write_byte(10, 0xAA);
    acc2->write_byte(10, 0xBB);
    acc3->write_byte(10, 0xCC);
    
    EXPECT_EQ(acc1->read_byte(10), 0xAA);
    EXPECT_EQ(acc2->read_byte(10), 0xBB);
    EXPECT_EQ(acc3->read_byte(10), 0xCC);
}

// Test destroying context
TEST_F(VMemUnitTest, DestroyContext) {
    context_id_t id = memunit.create_context(1024);
    ASSERT_NE(memunit.get_context(id), nullptr);
    
    memunit.destroy_context(id);
    EXPECT_EQ(memunit.get_context(id), nullptr);
}

// Test destroying context fails in protected mode
TEST_F(VMemUnitTest, DestroyContextProtectedFails) {
    context_id_t id = memunit.create_context(1024);
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    
    EXPECT_THROW(memunit.destroy_context(id), std::runtime_error);
}

// Test destroying non-existent context
TEST_F(VMemUnitTest, DestroyNonExistentContext) {
    EXPECT_THROW(memunit.destroy_context(999), std::invalid_argument);
}

// Test getting non-existent context
TEST_F(VMemUnitTest, GetNonExistentContext) {
    EXPECT_EQ(memunit.get_context(999), nullptr);
}

// Test creating context with zero size
TEST_F(VMemUnitTest, CreateContextZeroSize) {
    EXPECT_THROW(memunit.create_context(0), std::invalid_argument);
}

// Test finding context for address (integration test using accessors)
TEST_F(VMemUnitTest, FindContextForAddress) {
    context_id_t id1 = memunit.create_context(1000);
    context_id_t id2 = memunit.create_context(2000);
    
    // Write unique values to each context so we can identify them
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    
    auto ctx1 = memunit.get_context(id1);
    auto ctx2 = memunit.get_context(id2);
    
    auto acc1 = ctx1->create_paged_accessor(MemAccessMode::READ_WRITE);
    auto acc2 = ctx2->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    acc1->set_page(0);
    acc2->set_page(0);
    
    acc1->write_byte(0, 0x11);
    acc2->write_byte(0, 0x22);
    
    // Find context for address in first context and verify by reading through accessor
    auto ctx = memunit.find_context_for_address(500);
    ASSERT_NE(ctx, nullptr);
    auto test_acc = ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
    test_acc->set_page(0);
    EXPECT_EQ(test_acc->read_byte(0), 0x11);
    
    // Find context for address in second context
    ctx = memunit.find_context_for_address(1500);
    ASSERT_NE(ctx, nullptr);
    test_acc = ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
    test_acc->set_page(0);
    EXPECT_EQ(test_acc->read_byte(0), 0x22);
    
    // Address outside any context
    ctx = memunit.find_context_for_address(10000);
    EXPECT_EQ(ctx, nullptr);
}

// Integration tests for Context behavior through accessors
TEST_F(VMemUnitTest, ContextCreationAndAccess) {
    // Create context and verify it's usable through accessors
    context_id_t id = memunit.create_context(0x2000);
    auto ctx = memunit.get_context(id);
    ASSERT_NE(ctx, nullptr);
    
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Test writing across page boundary
    accessor->set_page(0);
    accessor->write_word(250, 0x1234);
    EXPECT_EQ(accessor->read_word(250), 0x1234);
}

TEST_F(VMemUnitTest, ContextBoundaryValidation) {
    // Create a small context to test boundary checking
    context_id_t id = memunit.create_context(256);
    auto ctx = memunit.get_context(id);
    
    memunit.set_mode(IVMemUnit::Mode::PROTECTED);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_page(0);
    // Write at valid offset
    accessor->write_byte(255, 0xFF);
    EXPECT_EQ(accessor->read_byte(255), 0xFF);
    
    // Attempting to access beyond context size should fail
    // (PagedMemoryAccessor validates against context size)
    accessor->set_page(1);
    EXPECT_THROW(accessor->write_byte(0, 0x00), std::runtime_error);
}

// Test vaddr validation
TEST(VAddrTest, Validation) {
    EXPECT_TRUE(is_valid_vaddr(0));
    EXPECT_TRUE(is_valid_vaddr(0xFFFFFFFFFF));  // Max 40-bit
    EXPECT_FALSE(is_valid_vaddr(0x10000000000)); // 41-bit
    EXPECT_FALSE(is_valid_vaddr(0xFFFFFFFFFFFFFFFF)); // 64-bit
}
