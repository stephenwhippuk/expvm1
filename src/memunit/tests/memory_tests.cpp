#include <gtest/gtest.h>
#include "memory2.h"
#include "errors.h"

using namespace lvm;

class Memory2Test : public ::testing::Test {
protected:
    static constexpr memsize_t TEST_SIZE = 1024;
};

// Basic creation and size test
TEST_F(Memory2Test, Creation) {
    Memory2 mem(TEST_SIZE);
    EXPECT_EQ(mem.size(), TEST_SIZE);
}

// Test zero initialization
TEST_F(Memory2Test, ZeroInitialized) {
    Memory2 mem(TEST_SIZE);
    
    // Check that memory is initialized to zero
    for (addr_t addr = 0; addr < 100; ++addr) {
        EXPECT_EQ(mem.read_byte(addr), 0);
    }
}

// Test byte read/write
TEST_F(Memory2Test, ByteReadWrite) {
    Memory2 mem(TEST_SIZE);
    
    mem.write_byte(0, 0x42);
    EXPECT_EQ(mem.read_byte(0), 0x42);
    
    mem.write_byte(100, 0xAB);
    EXPECT_EQ(mem.read_byte(100), 0xAB);
}

// Test word read/write
TEST_F(Memory2Test, WordReadWrite) {
    Memory2 mem(TEST_SIZE);
    
    mem.write_word(0, 0x1234);
    EXPECT_EQ(mem.read_word(0), 0x1234);
    
    mem.write_word(100, 0xABCD);
    EXPECT_EQ(mem.read_word(100), 0xABCD);
}

// Test word endianness (little-endian)
TEST_F(Memory2Test, WordEndianness) {
    Memory2 mem(TEST_SIZE);
    
    mem.write_word(0, 0xABCD);
    
    // Little-endian: low byte first
    EXPECT_EQ(mem.read_byte(0), 0xCD);  // Low byte
    EXPECT_EQ(mem.read_byte(1), 0xAB);  // High byte
}

// Test bulk read/write
TEST_F(Memory2Test, BulkReadWrite) {
    Memory2 mem(TEST_SIZE);
    
    byte_t write_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    mem.write_bulk(10, write_data, 5);
    
    byte_t read_data[5] = {0};
    mem.read_bulk(10, read_data, 5);
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(read_data[i], write_data[i]);
    }
}

// Test clear region
TEST_F(Memory2Test, ClearRegion) {
    Memory2 mem(TEST_SIZE);
    
    // Write some data
    for (addr_t addr = 10; addr < 20; ++addr) {
        mem.write_byte(addr, 0xFF);
    }
    
    // Clear a portion
    mem.clear(10, 10);
    
    // Check it's cleared
    for (addr_t addr = 10; addr < 20; ++addr) {
        EXPECT_EQ(mem.read_byte(addr), 0);
    }
}

// Test clear all
TEST_F(Memory2Test, ClearAll) {
    Memory2 mem(TEST_SIZE);
    
    // Write some data
    mem.write_byte(0, 0xFF);
    mem.write_byte(100, 0xFF);
    mem.write_byte(TEST_SIZE - 1, 0xFF);
    
    // Clear all
    mem.clear_all();
    
    // Check it's all cleared
    EXPECT_EQ(mem.read_byte(0), 0);
    EXPECT_EQ(mem.read_byte(100), 0);
    EXPECT_EQ(mem.read_byte(TEST_SIZE - 1), 0);
}

// Test boundary conditions
TEST_F(Memory2Test, BoundaryAccess) {
    Memory2 mem(TEST_SIZE);
    
    // First byte
    mem.write_byte(0, 0x11);
    EXPECT_EQ(mem.read_byte(0), 0x11);
    
    // Last byte
    mem.write_byte(TEST_SIZE - 1, 0x22);
    EXPECT_EQ(mem.read_byte(TEST_SIZE - 1), 0x22);
}

// Test out of bounds read
TEST_F(Memory2Test, OutOfBoundsRead) {
    Memory2 mem(TEST_SIZE);
    
    EXPECT_THROW(mem.read_byte(TEST_SIZE), lvm::runtime_error);
    EXPECT_THROW(mem.read_word(TEST_SIZE - 1), lvm::runtime_error); // Word needs 2 bytes
}

// Test out of bounds write
TEST_F(Memory2Test, OutOfBoundsWrite) {
    Memory2 mem(TEST_SIZE);
    
    EXPECT_THROW(mem.write_byte(TEST_SIZE, 0x00), lvm::runtime_error);
    EXPECT_THROW(mem.write_word(TEST_SIZE - 1, 0x0000), lvm::runtime_error);
}

// Test zero size creation
TEST_F(Memory2Test, ZeroSizeCreation) {
    EXPECT_THROW(Memory2(0), lvm::runtime_error);
}

// Test independence of different instances
TEST_F(Memory2Test, Independence) {
    Memory2 mem1(100);
    Memory2 mem2(100);
    
    mem1.write_byte(10, 0x11);
    mem2.write_byte(10, 0x22);
    
    EXPECT_EQ(mem1.read_byte(10), 0x11);
    EXPECT_EQ(mem2.read_byte(10), 0x22);
}
