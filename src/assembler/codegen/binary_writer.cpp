#include "binary_writer.h"
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace lvm {

// Import assembler types
using assembler::CodeGraph;

BinaryWriter::BinaryWriter() {
}

void BinaryWriter::write_binary(const CodeGraph& graph, 
                                const std::string& filename,
                                const std::string& program_name) {
    // Generate binary data
    std::vector<uint8_t> binary_data = generate_binary(graph, program_name);
    
    // Write to file
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(binary_data.data()), binary_data.size());
    
    if (!file.good()) {
        throw std::runtime_error("Failed to write binary data to: " + filename);
    }
    
    file.close();
}

std::vector<uint8_t> BinaryWriter::generate_binary(const CodeGraph& graph,
                                                   const std::string& program_name) {
    std::vector<uint8_t> binary;
    
    // Calculate header size
    std::string machine_name = MACHINE_NAME;
    std::string truncated_program_name = program_name.substr(0, 32);
    
    uint16_t header_size = 2 +     // header size itself
                          4 +      // header version
                          1 + static_cast<uint16_t>(machine_name.size()) +  // machine name
                          4 +      // machine version
                          2 + static_cast<uint16_t>(truncated_program_name.size());   // program name
    
    // Write header size (2 bytes, little-endian)
    write_uint16(binary, header_size);
    
    // Write header version (4 bytes: major, minor, revision_high, revision_low)
    write_uint8(binary, HEADER_VERSION_MAJOR);
    write_uint8(binary, HEADER_VERSION_MINOR);
    write_uint16(binary, HEADER_VERSION_REVISION);
    
    // Write machine name size (1 byte)
    write_uint8(binary, static_cast<uint8_t>(machine_name.size()));
    
    // Write machine name
    write_string(binary, machine_name);
    
    // Write machine version (4 bytes)
    write_uint8(binary, MACHINE_VERSION_MAJOR);
    write_uint8(binary, MACHINE_VERSION_MINOR);
    write_uint16(binary, MACHINE_VERSION_REVISION);
    
    // Write program name size (2 bytes, little-endian)
    write_uint16(binary, static_cast<uint16_t>(truncated_program_name.size()));
    
    // Write program name
    write_string(binary, truncated_program_name);
    
    // Get data segment bytes
    std::vector<uint8_t> data_segment;
    for (const auto& data_block : graph.data_blocks()) {
        const std::vector<uint8_t>& block_bytes = data_block->data();
        data_segment.insert(data_segment.end(), block_bytes.begin(), block_bytes.end());
    }
    
    // Write data segment size (4 bytes, little-endian)
    write_uint32(binary, static_cast<uint32_t>(data_segment.size()));
    
    // Write data segment
    for (uint8_t b : data_segment) {
        binary.push_back(b);
    }
    
    // Get code segment bytes
    std::vector<uint8_t> code_segment;
    for (const auto& code_node : graph.code_nodes()) {
        // Skip labels (they have size 0)
        if (code_node->size() == 0) {
            continue;
        }
        
        // Cast to CodeInstructionNode to get encode() method
        auto* instruction = dynamic_cast<assembler::CodeInstructionNode*>(code_node.get());
        if (instruction) {
            std::vector<uint8_t> node_bytes = instruction->encode();
            code_segment.insert(code_segment.end(), node_bytes.begin(), node_bytes.end());
        }
    }
    
    // Write code segment size (4 bytes, little-endian)
    write_uint32(binary, static_cast<uint32_t>(code_segment.size()));
    
    // Write code segment
    for (uint8_t b : code_segment) {
        binary.push_back(b);
    }
    
    return binary;
}

void BinaryWriter::write_uint8(std::vector<uint8_t>& data, uint8_t value) {
    data.push_back(value);
}

void BinaryWriter::write_uint16(std::vector<uint8_t>& data, uint16_t value) {
    data.push_back(value & 0xFF);          // low byte
    data.push_back((value >> 8) & 0xFF);   // high byte
}

void BinaryWriter::write_uint32(std::vector<uint8_t>& data, uint32_t value) {
    data.push_back(value & 0xFF);           // byte 0
    data.push_back((value >> 8) & 0xFF);    // byte 1
    data.push_back((value >> 16) & 0xFF);   // byte 2
    data.push_back((value >> 24) & 0xFF);   // byte 3
}

void BinaryWriter::write_string(std::vector<uint8_t>& data, const std::string& str, size_t max_length) {
    size_t length = max_length > 0 ? std::min(str.size(), max_length) : str.size();
    for (size_t i = 0; i < length; ++i) {
        data.push_back(static_cast<uint8_t>(str[i]));
    }
}

} // namespace lvm
