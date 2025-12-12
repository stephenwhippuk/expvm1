#pragma once

#include "../ir/code_graph.h"
#include <string>
#include <vector>
#include <cstdint>

namespace lvm {

// Import assembler types into this namespace
using assembler::CodeGraph;

/**
 * BinaryWriter - Generates binary files in Pendragon VM format
 * 
 * Writes binary files following the format specification:
 * - Header with version, machine name/version, program name
 * - Data segment (size + bytes)
 * - Code segment (size + bytes)
 */
class BinaryWriter {
public:
    BinaryWriter();
    
    /**
     * Write a binary file from a code graph
     * 
     * @param graph The code graph to write
     * @param filename Path to write the binary file to
     * @param program_name Name of the program (32 chars max)
     * @throws runtime_error if file cannot be written
     */
    void write_binary(const CodeGraph& graph, 
                     const std::string& filename,
                     const std::string& program_name = "Program");
    
    /**
     * Generate binary data as a byte vector (for testing)
     * 
     * @param graph The code graph to write
     * @param program_name Name of the program
     * @return Binary data as byte vector
     */
    std::vector<uint8_t> generate_binary(const CodeGraph& graph,
                                         const std::string& program_name = "Program");

private:
    // Write helper functions
    void write_uint8(std::vector<uint8_t>& data, uint8_t value);
    void write_uint16(std::vector<uint8_t>& data, uint16_t value);
    void write_uint32(std::vector<uint8_t>& data, uint32_t value);
    void write_string(std::vector<uint8_t>& data, const std::string& str, size_t max_length = 0);
    
    // Header version: 1.0.0
    static constexpr uint8_t HEADER_VERSION_MAJOR = 1;
    static constexpr uint8_t HEADER_VERSION_MINOR = 0;
    static constexpr uint16_t HEADER_VERSION_REVISION = 0;
    
    // Machine info
    static constexpr const char* MACHINE_NAME = "Pendragon";
    static constexpr uint8_t MACHINE_VERSION_MAJOR = 1;
    static constexpr uint8_t MACHINE_VERSION_MINOR = 0;
    static constexpr uint16_t MACHINE_VERSION_REVISION = 0;
};

} // namespace lvm
