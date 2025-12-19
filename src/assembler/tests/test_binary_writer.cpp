#include <gtest/gtest.h>
#include "../codegen/binary_writer.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic/semantic_analyzer.h"
#include "../ir/code_graph_builder.h"
#include "../codegen/address_resolver.h"

using namespace lvm;
using namespace lvm::assembler;

// Helper to build complete binary from source
std::vector<uint8_t> assemble_to_binary(const std::string& source, const std::string& program_name = "TestProg") {
    // Pass 1: Lexer + Parser
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    // Pass 2: Semantic analysis
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    // Pass 3: Build code graph
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    // Pass 4: Resolve addresses
    AddressResolver resolver(table, *graph);
    resolver.resolve();
    
    // Pass 5: Generate binary
    BinaryWriter writer;
    return writer.generate_binary(*graph, program_name);
}

TEST(BinaryWriterTest, EmptyProgram) {
    std::string source = R"(
        DATA
        CODE
    )";
    
    auto binary = assemble_to_binary(source, "Empty");
    
    // Should have header + empty segments
    ASSERT_GT(binary.size(), 20);  // At least header size
    
    // Check header size (first 2 bytes, little-endian)
    uint16_t header_size = binary[0] | (binary[1] << 8);
    EXPECT_GT(header_size, 0);
    
    // Check header version
    EXPECT_EQ(binary[2], 1);  // major
    EXPECT_EQ(binary[3], 0);  // minor
    EXPECT_EQ(binary[4], 0);  // revision high
    EXPECT_EQ(binary[5], 0);  // revision low
}

TEST(BinaryWriterTest, DataSegment) {
    std::string source = "DATA\nmydata: DB [0x42]\nCODE\nHALT\n";
    
    auto binary = assemble_to_binary(source, "DataTest");
    
    // Parse header to find data segment
    size_t offset = 0;
    
    // Skip header size
    uint16_t header_size = binary[offset] | (binary[offset + 1] << 8);
    std::cout << "Header size: " << header_size << std::endl;
    offset = header_size;
    
    // Read data segment size (4 bytes, little-endian)
    uint32_t data_size = binary[offset] | (binary[offset + 1] << 8) |
                        (binary[offset + 2] << 16) | (binary[offset + 3] << 24);
    offset += 4;
    
    // Check data segment
    EXPECT_EQ(data_size, 3);  // 1 byte data + 2 byte size prefix
    // First two bytes are size prefix (little-endian: 0x01 0x00)
    EXPECT_EQ(binary[offset], 0x01);  // Size low byte
    EXPECT_EQ(binary[offset + 1], 0x00);  // Size high byte
    EXPECT_EQ(binary[offset + 2], 0x42);  // Actual data
}

TEST(BinaryWriterTest, CodeSegment) {
    std::string source = R"(
        DATA
        CODE
            HALT
            PUSHB R0, 0x10
    )";
    
    auto binary = assemble_to_binary(source, "CodeTest");
    
    // Parse header
    size_t offset = 0;
    uint16_t header_size = binary[offset] | (binary[offset + 1] << 8);
    offset = header_size;
    
    // Skip data segment
    uint32_t data_size = binary[offset] | (binary[offset + 1] << 8) |
                        (binary[offset + 2] << 16) | (binary[offset + 3] << 24);
    offset += 4 + data_size;
    
    // Read code segment size
    uint32_t code_size = binary[offset] | (binary[offset + 1] << 8) |
                        (binary[offset + 2] << 16) | (binary[offset + 3] << 24);
    offset += 4;
    
    // Check code segment
    // HALT (1 byte) + PUSHB R0, 0x10 (likely 6 bytes with register encoding)
    EXPECT_GT(code_size, 0);  // Has some code
    EXPECT_EQ(binary[offset], 0x01);      // HALT opcode
}

TEST(BinaryWriterTest, ProgramName) {
    std::string source = R"(
        DATA
        CODE
            HALT
    )";
    
    auto binary = assemble_to_binary(source, "MyProgram");
    
    // Parse header to find program name
    size_t offset = 0;
    
    // Skip header size
    offset += 2;
    
    // Skip header version
    offset += 4;
    
    // Skip machine name
    uint8_t machine_name_size = binary[offset++];
    offset += machine_name_size;
    
    // Skip machine version
    offset += 4;
    
    // Read program name size
    uint16_t prog_name_size = binary[offset] | (binary[offset + 1] << 8);
    offset += 2;
    
    // Read program name
    std::string program_name;
    for (size_t i = 0; i < prog_name_size; ++i) {
        program_name += static_cast<char>(binary[offset + i]);
    }
    
    EXPECT_EQ(program_name, "MyProgram");
}

TEST(BinaryWriterTest, CompleteProgram) {
    std::string source = "DATA\nmessage: DB [0x48, 0x65, 0x6C, 0x6C, 0x6F]\nvalue: DW [0x1234]\nCODE\nstart:\nPUSHW R0, message\nPUSHW R1, 5\nSYSCALL 0x01\nHALT\n";
    
    auto binary = assemble_to_binary(source, "HelloWorld");
    
    // Should have complete binary structure
    ASSERT_GT(binary.size(), 50);
    
    // Verify magic/version bytes
    EXPECT_EQ(binary[2], 1);  // header version major
    EXPECT_EQ(binary[3], 0);  // header version minor
    
    // Parse to verify data segment
    uint16_t header_size = binary[0] | (binary[1] << 8);
    size_t offset = header_size;
    
    uint32_t data_size = binary[offset] | (binary[offset + 1] << 8) |
                        (binary[offset + 2] << 16) | (binary[offset + 3] << 24);
    offset += 4;
    
    // Data segment: 5 bytes (message) + 2 bytes (value) + 4 bytes (size prefixes) = 11 bytes
    EXPECT_EQ(data_size, 11);
    // First block: message with size prefix
    EXPECT_EQ(binary[offset], 0x05);      // Size prefix low byte (5 bytes)
    EXPECT_EQ(binary[offset + 1], 0x00);  // Size prefix high byte
    EXPECT_EQ(binary[offset + 2], 0x48);  // 'H' - first message byte
    // Second block: value with size prefix
    EXPECT_EQ(binary[offset + 7], 0x02);  // Size prefix low byte (2 bytes)
    EXPECT_EQ(binary[offset + 8], 0x00);  // Size prefix high byte
    EXPECT_EQ(binary[offset + 9], 0x34);  // value low byte
    EXPECT_EQ(binary[offset + 10], 0x12); // value high byte
    
    // Verify code segment exists
    offset += data_size;
    uint32_t code_size = binary[offset] | (binary[offset + 1] << 8) |
                        (binary[offset + 2] << 16) | (binary[offset + 3] << 24);
    EXPECT_GT(code_size, 0);
}

TEST(BinaryWriterTest, LongProgramName) {
    std::string source = R"(
        DATA
        CODE
            HALT
    )";
    
    // Name longer than 32 chars should be truncated
    std::string long_name = "ThisIsAVeryLongProgramNameThatExceedsThirtyTwoCharacters";
    auto binary = assemble_to_binary(source, long_name);
    
    // Parse to find program name
    size_t offset = 2 + 4;  // skip header size + header version
    
    uint8_t machine_name_size = binary[offset++];
    offset += machine_name_size + 4;  // skip machine name + version
    
    uint16_t prog_name_size = binary[offset] | (binary[offset + 1] << 8);
    
    // Should be truncated to 32
    EXPECT_LE(prog_name_size, 32);
}

TEST(BinaryWriterTest, MachineNameAndVersion) {
    std::string source = R"(
        DATA
        CODE
            HALT
    )";
    
    auto binary = assemble_to_binary(source);
    
    // Check machine name
    size_t offset = 2 + 4;  // skip header size + header version
    
    uint8_t machine_name_size = binary[offset++];
    std::string machine_name;
    for (size_t i = 0; i < machine_name_size; ++i) {
        machine_name += static_cast<char>(binary[offset++]);
    }
    
    EXPECT_EQ(machine_name, "Pendragon");
    
    // Check machine version
    EXPECT_EQ(binary[offset], 1);      // major
    EXPECT_EQ(binary[offset + 1], 0);  // minor
    EXPECT_EQ(binary[offset + 2], 0);  // revision high
    EXPECT_EQ(binary[offset + 3], 0);  // revision low
}
