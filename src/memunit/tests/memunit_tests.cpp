#include <gtest/gtest.h>
#include "vmemunit.h"
#include "context.h"
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
    EXPECT_EQ(memunit.get_mode(), VMemUnit::Mode::UNPROTECTED);
    EXPECT_TRUE(memunit.is_unprotected());
    EXPECT_FALSE(memunit.is_protected());
}

// Test mode switching
TEST_F(VMemUnitTest, ModeSwitch) {
    memunit.set_mode(VMemUnit::Mode::PROTECTED);
    EXPECT_EQ(memunit.get_mode(), VMemUnit::Mode::PROTECTED);
    EXPECT_TRUE(memunit.is_protected());
    EXPECT_FALSE(memunit.is_unprotected());
    
    memunit.set_mode(VMemUnit::Mode::UNPROTECTED);
    EXPECT_EQ(memunit.get_mode(), VMemUnit::Mode::UNPROTECTED);
}

// Test context creation in unprotected mode
TEST_F(VMemUnitTest, CreateContextUnprotected) {
    context_id_t id = memunit.create_context(1024);
    EXPECT_EQ(id, 0);
    
    const Context* ctx = memunit.get_context(id);
    ASSERT_NE(ctx, nullptr);
    EXPECT_EQ(ctx->get_id(), 0);
    EXPECT_EQ(ctx->get_base_address(), 0);
    EXPECT_EQ(ctx->get_size(), 1024);
}

// Test context creation fails in protected mode
TEST_F(VMemUnitTest, CreateContextProtectedFails) {
    memunit.set_mode(VMemUnit::Mode::PROTECTED);
    EXPECT_THROW(memunit.create_context(1024), std::runtime_error);
}

// Test creating multiple contexts
TEST_F(VMemUnitTest, CreateMultipleContexts) {
    context_id_t id1 = memunit.create_context(1024);
    context_id_t id2 = memunit.create_context(2048);
    context_id_t id3 = memunit.create_context(512);
    
    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(id3, 2);
    
    const Context* ctx1 = memunit.get_context(id1);
    const Context* ctx2 = memunit.get_context(id2);
    const Context* ctx3 = memunit.get_context(id3);
    
    ASSERT_NE(ctx1, nullptr);
    ASSERT_NE(ctx2, nullptr);
    ASSERT_NE(ctx3, nullptr);
    
    // Contexts should be allocated contiguously in virtual space
    EXPECT_EQ(ctx1->get_base_address(), 0);
    EXPECT_EQ(ctx2->get_base_address(), 1024);
    EXPECT_EQ(ctx3->get_base_address(), 1024 + 2048);
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
    memunit.set_mode(VMemUnit::Mode::PROTECTED);
    
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

// Test finding context for address
TEST_F(VMemUnitTest, FindContextForAddress) {
    context_id_t id1 = memunit.create_context(1000);
    context_id_t id2 = memunit.create_context(2000);
    
    // Address in first context
    const Context* ctx = memunit.find_context_for_address(500);
    ASSERT_NE(ctx, nullptr);
    EXPECT_EQ(ctx->get_id(), id1);
    
    // Address in second context
    ctx = memunit.find_context_for_address(1500);
    ASSERT_NE(ctx, nullptr);
    EXPECT_EQ(ctx->get_id(), id2);
    
    // Address outside any context
    ctx = memunit.find_context_for_address(10000);
    EXPECT_EQ(ctx, nullptr);
}

// Test Context class directly
TEST(ContextTest, Creation) {
    Context ctx(1, 0x1000, 0x2000);
    EXPECT_EQ(ctx.get_id(), 1);
    EXPECT_EQ(ctx.get_base_address(), 0x1000);
    EXPECT_EQ(ctx.get_size(), 0x2000);
    EXPECT_EQ(ctx.get_end_address(), 0x3000);
}

TEST(ContextTest, Contains) {
    Context ctx(0, 0x1000, 0x1000);
    
    EXPECT_TRUE(ctx.contains(0x1000));  // Start
    EXPECT_TRUE(ctx.contains(0x1500));  // Middle
    EXPECT_TRUE(ctx.contains(0x1FFF));  // Just before end
    EXPECT_FALSE(ctx.contains(0x2000)); // End (exclusive)
    EXPECT_FALSE(ctx.contains(0x0FFF)); // Before start
}

TEST(ContextTest, InvalidAddress) {
    // Address exceeding 40-bit space
    vaddr_t invalid_addr = (1ULL << 40);
    EXPECT_THROW(Context(0, invalid_addr, 1024), std::invalid_argument);
}

TEST(ContextTest, Overflow) {
    // Context that would overflow 40-bit space
    vaddr_t near_limit = VADDR_MASK - 100;
    EXPECT_THROW(Context(0, near_limit, 200), std::invalid_argument);
}

// Test vaddr validation
TEST(VAddrTest, Validation) {
    EXPECT_TRUE(is_valid_vaddr(0));
    EXPECT_TRUE(is_valid_vaddr(0xFFFFFFFFFF));  // Max 40-bit
    EXPECT_FALSE(is_valid_vaddr(0x10000000000)); // 41-bit
    EXPECT_FALSE(is_valid_vaddr(0xFFFFFFFFFFFFFFFF)); // 64-bit
}
