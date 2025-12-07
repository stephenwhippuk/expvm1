#include <gtest/gtest.h>
#include "vmemunit.h"
#include "paged_memory_accessor.h"
#include "vaddr.h"
#include "accessMode.h"
#include "errors.h"
#include <stdexcept>

using namespace lvm;

// Test fixture for PagedMemoryAccessor tests
class PagedMemoryAccessorTest : public ::testing::Test {
protected:
    VMemUnit vmem_unit;
    context_id_t context_id;
    
    void SetUp() override {
        // Create a context (4MB size for testing)
        context_id = vmem_unit.create_context(4 * 1024 * 1024);
    }
};

// Test accessor creation in protected mode
TEST_F(PagedMemoryAccessorTest, AccessorCreation) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    ASSERT_NE(ctx, nullptr);
    
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    ASSERT_NE(accessor, nullptr);
    EXPECT_EQ(accessor->get_context_id(), context_id);
}

// Test accessor creation fails in unprotected mode
TEST_F(PagedMemoryAccessorTest, AccessorCreationUnprotectedFails) {
    // Leave in unprotected mode
    auto ctx = vmem_unit.get_context(context_id);
    ASSERT_NE(ctx, nullptr);
    
    EXPECT_THROW(ctx->create_paged_accessor(MemAccessMode::READ_WRITE), 
                 std::runtime_error);
}

// Test page switching via accessor
TEST_F(PagedMemoryAccessorTest, PageSwitching) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Test page management through accessor
    EXPECT_EQ(accessor->get_page(), 0);  // Initial page is 0
    accessor->set_page(0x1234);
    EXPECT_EQ(accessor->get_page(), 0x1234);
    
    accessor->set_page(0xABCD);
    EXPECT_EQ(accessor->get_page(), 0xABCD);
}

// Test byte read/write with on-demand allocation
TEST_F(PagedMemoryAccessorTest, ByteReadWrite) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Write to page 0, offset 0x100
    accessor->write_byte(0x100, 0x42);
    
    // Read it back
    byte_t value = accessor->read_byte(0x100);
    EXPECT_EQ(value, 0x42);
}

// Test byte operations across pages
TEST_F(PagedMemoryAccessorTest, ByteOperationsAcrossPages) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Write to page 0
    accessor->set_page(0);
    accessor->write_byte(0x200, 0xAA);
    
    // Write to page 1
    accessor->set_page(1);
    accessor->write_byte(0x300, 0xBB);
    
    // Read from page 0
    accessor->set_page(0);
    EXPECT_EQ(accessor->read_byte(0x200), 0xAA);
    
    // Read from page 1
    accessor->set_page(1);
    EXPECT_EQ(accessor->read_byte(0x300), 0xBB);
}

// Test word read/write (little-endian)
TEST_F(PagedMemoryAccessorTest, WordReadWrite) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    accessor->write_word(0x400, 0x1234);
    
    word_t value = accessor->read_word(0x400);
    EXPECT_EQ(value, 0x1234);
    
    // Verify little-endian storage
    EXPECT_EQ(accessor->read_byte(0x400), 0x34);  // Low byte
    EXPECT_EQ(accessor->read_byte(0x401), 0x12);  // High byte
}

// Test bulk write and read
TEST_F(PagedMemoryAccessorTest, BulkOperations) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    
    // Write data
    std::vector<byte_t> data = {0x10, 0x20, 0x30, 0x40, 0x50};
    accessor->bulk_write(0x500, data);
    
    // Read it back
    std::vector<byte_t> buffer;
    accessor->bulk_read(0x500, buffer, 5);
    
    ASSERT_EQ(buffer.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(buffer[i], data[i]);
    }
}

// Test read-only accessor
TEST_F(PagedMemoryAccessorTest, ReadOnlyAccessor) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
    
    
    // Write should fail
    EXPECT_THROW(accessor->write_byte(0x100, 0x42), lvm::runtime_error);
    EXPECT_THROW(accessor->write_word(0x100, 0x1234), lvm::runtime_error);
    
    std::vector<byte_t> data = {0x10, 0x20};
    EXPECT_THROW(accessor->bulk_write(0x100, data), lvm::runtime_error);
}

// Test operations in protected mode work (accessor obtained in protected mode)
TEST_F(PagedMemoryAccessorTest, ProtectedModeOperations) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Operations should work in protected mode
    accessor->write_byte(0x100, 0x42);
    EXPECT_EQ(accessor->read_byte(0x100), 0x42);
}

// Test address validation
TEST_F(PagedMemoryAccessorTest, AddressOutOfBounds) {
    // Create small context (1 page = 64KB)
    context_id_t small_ctx = vmem_unit.create_context(0x10000);
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(small_ctx);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Page 0 is valid
    accessor->set_page(0);
    accessor->write_byte(0xFFFF, 0x42);  // Last byte in page 0
    
    // Page 1 would exceed context size
    accessor->set_page(1);
    EXPECT_THROW(accessor->write_byte(0x0000, 0x42), std::runtime_error);
}

// Test default read returns zero
TEST_F(PagedMemoryAccessorTest, DefaultReadReturnsZero) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    
    // Read from unallocated memory should return 0
    EXPECT_EQ(accessor->read_byte(0x1000), 0);
    EXPECT_EQ(accessor->read_word(0x2000), 0);
}

// Test page+address to 32-bit address translation
TEST_F(PagedMemoryAccessorTest, PageAddressTranslation) {
    vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
    auto ctx = vmem_unit.get_context(context_id);
    auto accessor = ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    // Page 0x0001, offset 0x2345 = 0x00012345
    accessor->set_page(0x0001);
    accessor->write_byte(0x2345, 0xAB);
    
    // Page 0x0010, offset 0x0000 = 0x00100000
    accessor->set_page(0x0010);
    accessor->write_byte(0x0000, 0xCD);
    
    // Verify they're independent addresses
    accessor->set_page(0x0001);
    EXPECT_EQ(accessor->read_byte(0x2345), 0xAB);
    
    accessor->set_page(0x0010);
    EXPECT_EQ(accessor->read_byte(0x0000), 0xCD);
}
