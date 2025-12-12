#include <gtest/gtest.h>
#include "binary_loader.h"
#include "errors.h"
#include <vector>
#include <cstring>

using namespace lvm;

// Helper function to create a test binary in memory
std::vector<byte_t> create_test_binary(
    const std::string& machine_name = "Pendragon",
    uint8_t machine_major = 1, uint8_t machine_minor = 0, uint16_t machine_rev = 0,
    const std::string& program_name = "TestProgram",
    const std::vector<byte_t>& data_segment = {},
    const std::vector<byte_t>& code_segment = {})
{
    std::vector<byte_t> binary;
    
    // Calculate header size
    uint16_t header_size = 2 +     // header size itself
                          4 +      // header version
                          1 + static_cast<uint16_t>(machine_name.size()) +  // machine name
                          4 +      // machine version
                          2 + static_cast<uint16_t>(program_name.size());   // program name
    
    // Write header size (2 bytes, little-endian)
    binary.push_back(header_size & 0xFF);
    binary.push_back((header_size >> 8) & 0xFF);
    
    // Write header version (4 bytes: major, minor, revision_high, revision_low)
    binary.push_back(1);  // major
    binary.push_back(0);  // minor
    binary.push_back(0);  // revision high byte
    binary.push_back(0);  // revision low byte
    
    // Write machine name size (1 byte)
    binary.push_back(static_cast<byte_t>(machine_name.size()));
    
    // Write machine name
    for (char c : machine_name) {
        binary.push_back(static_cast<byte_t>(c));
    }
    
    // Write machine version (4 bytes)
    binary.push_back(machine_major);
    binary.push_back(machine_minor);
    binary.push_back((machine_rev >> 8) & 0xFF);
    binary.push_back(machine_rev & 0xFF);
    
    // Write program name size (2 bytes, little-endian)
    uint16_t prog_name_size = static_cast<uint16_t>(program_name.size());
    binary.push_back(prog_name_size & 0xFF);
    binary.push_back((prog_name_size >> 8) & 0xFF);
    
    // Write program name
    for (char c : program_name) {
        binary.push_back(static_cast<byte_t>(c));
    }
    
    // Write data segment size (4 bytes, little-endian)
    uint32_t data_size = static_cast<uint32_t>(data_segment.size());
    binary.push_back(data_size & 0xFF);
    binary.push_back((data_size >> 8) & 0xFF);
    binary.push_back((data_size >> 16) & 0xFF);
    binary.push_back((data_size >> 24) & 0xFF);
    
    // Write data segment
    for (byte_t b : data_segment) {
        binary.push_back(b);
    }
    
    // Write code segment size (4 bytes, little-endian)
    uint32_t code_size = static_cast<uint32_t>(code_segment.size());
    binary.push_back(code_size & 0xFF);
    binary.push_back((code_size >> 8) & 0xFF);
    binary.push_back((code_size >> 16) & 0xFF);
    binary.push_back((code_size >> 24) & 0xFF);
    
    // Write code segment
    for (byte_t b : code_segment) {
        binary.push_back(b);
    }
    
    return binary;
}

TEST(BinaryLoaderTest, LoadValidBinary) {
    BinaryLoader loader;
    
    std::vector<byte_t> code = {0x01, 0x02, 0x03, 0x04};
    std::vector<byte_t> data = {0x0A, 0x0B};
    
    auto binary = create_test_binary("Pendragon", 1, 0, 0, "TestProg", data, code);
    
    BinaryProgram program = loader.load_from_bytes(binary);
    
    EXPECT_EQ(program.header.machine_name, "Pendragon");
    EXPECT_EQ(program.header.machine_version.major, 1);
    EXPECT_EQ(program.header.machine_version.minor, 0);
    EXPECT_EQ(program.header.machine_version.revision, 0);
    EXPECT_EQ(program.header.program_name, "TestProg");
    
    EXPECT_EQ(program.data_segment.size(), 2);
    EXPECT_EQ(program.data_segment[0], 0x0A);
    EXPECT_EQ(program.data_segment[1], 0x0B);
    
    EXPECT_EQ(program.code_segment.size(), 4);
    EXPECT_EQ(program.code_segment[0], 0x01);
    EXPECT_EQ(program.code_segment[1], 0x02);
    EXPECT_EQ(program.code_segment[2], 0x03);
    EXPECT_EQ(program.code_segment[3], 0x04);
}

TEST(BinaryLoaderTest, LoadEmptySegments) {
    BinaryLoader loader;
    
    auto binary = create_test_binary("Pendragon", 1, 0, 0, "Empty", {}, {});
    
    BinaryProgram program = loader.load_from_bytes(binary);
    
    EXPECT_EQ(program.data_segment.size(), 0);
    EXPECT_EQ(program.code_segment.size(), 0);
}

TEST(BinaryLoaderTest, InvalidMachineName) {
    BinaryLoader loader;
    
    auto binary = create_test_binary("WrongMachine", 1, 0, 0, "Test", {}, {});
    
    EXPECT_THROW({
        loader.load_from_bytes(binary);
    }, runtime_error);
}

TEST(BinaryLoaderTest, InvalidMachineVersion) {
    BinaryLoader loader;
    
    auto binary = create_test_binary("Pendragon", 2, 0, 0, "Test", {}, {});
    
    EXPECT_THROW({
        loader.load_from_bytes(binary);
    }, runtime_error);
}

TEST(BinaryLoaderTest, InvalidHeaderVersion) {
    BinaryLoader loader;
    
    std::vector<byte_t> binary;
    
    // Header size
    binary.push_back(0x0E);
    binary.push_back(0x00);
    
    // Invalid header version (2.0.0 instead of 1.0.0)
    binary.push_back(2);  // major
    binary.push_back(0);  // minor
    binary.push_back(0);  // revision high
    binary.push_back(0);  // revision low
    
    // Rest of the binary...
    binary.push_back(9);  // machine name size
    for (char c : "Pendragon") {
        binary.push_back(static_cast<byte_t>(c));
    }
    
    EXPECT_THROW({
        loader.load_from_bytes(binary);
    }, runtime_error);
}

TEST(BinaryLoaderTest, TruncatedBinary) {
    BinaryLoader loader;
    
    std::vector<byte_t> binary = {0x00, 0x00};  // Just header size, nothing else
    
    EXPECT_THROW({
        loader.load_from_bytes(binary);
    }, runtime_error);
}

TEST(BinaryLoaderTest, BinaryTooSmall) {
    BinaryLoader loader;
    
    std::vector<byte_t> binary = {0x00};  // Too small
    
    EXPECT_THROW({
        loader.load_from_bytes(binary);
    }, runtime_error);
}

TEST(BinaryLoaderTest, VersionToString) {
    BinaryVersion v(1, 2, 345);
    EXPECT_EQ(v.to_string(), "1.2.345");
}

TEST(BinaryLoaderTest, VersionComparison) {
    BinaryVersion v1(1, 0, 0);
    BinaryVersion v2(1, 0, 0);
    BinaryVersion v3(1, 0, 1);
    
    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 != v2);
    EXPECT_TRUE(v1 != v3);
}

TEST(BinaryLoaderTest, LongProgramName) {
    BinaryLoader loader;
    
    std::string long_name(500, 'A');  // 500 character name
    auto binary = create_test_binary("Pendragon", 1, 0, 0, long_name, {}, {});
    
    BinaryProgram program = loader.load_from_bytes(binary);
    
    EXPECT_EQ(program.header.program_name, long_name);
}

TEST(BinaryLoaderTest, LargeSegments) {
    BinaryLoader loader;
    
    std::vector<byte_t> large_code(10000, 0x42);
    std::vector<byte_t> large_data(5000, 0x99);
    
    auto binary = create_test_binary("Pendragon", 1, 0, 0, "Large", large_data, large_code);
    
    BinaryProgram program = loader.load_from_bytes(binary);
    
    EXPECT_EQ(program.code_segment.size(), 10000);
    EXPECT_EQ(program.data_segment.size(), 5000);
    EXPECT_EQ(program.code_segment[0], 0x42);
    EXPECT_EQ(program.data_segment[0], 0x99);
}
