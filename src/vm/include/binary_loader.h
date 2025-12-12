#pragma once

#include "memsize.h"
#include <vector>
#include <string>
#include <cstdint>

namespace lvm {

    /**
     * Binary file format header structure (Version 1.0.0)
     * 
     * Header layout:
     * - Header size: 2 bytes
     * - Header version: 4 bytes (major, minor, revision_high, revision_low)
     * - Machine name size: 1 byte
     * - Machine name: variable bytes
     * - Machine version: 4 bytes (major, minor, revision_high, revision_low)
     * - Program name size: 2 bytes
     * - Program name: variable bytes
     * 
     * Program layout:
     * - Data segment size: 4 bytes
     * - Data segment: variable bytes
     * - Code segment size: 4 bytes
     * - Code segment: variable bytes
     */
    struct BinaryVersion {
        uint8_t major;
        uint8_t minor;
        uint16_t revision;
        
        BinaryVersion(uint8_t maj = 0, uint8_t min = 0, uint16_t rev = 0)
            : major(maj), minor(min), revision(rev) {}
            
        bool operator==(const BinaryVersion& other) const {
            return major == other.major && minor == other.minor && revision == other.revision;
        }
        
        bool operator!=(const BinaryVersion& other) const {
            return !(*this == other);
        }
        
        std::string to_string() const;
    };
    
    struct BinaryHeader {
        uint16_t header_size;
        BinaryVersion header_version;
        std::string machine_name;
        BinaryVersion machine_version;
        std::string program_name;
    };
    
    struct BinaryProgram {
        BinaryHeader header;
        std::vector<byte_t> data_segment;
        std::vector<byte_t> code_segment;
    };

    /**
     * BinaryLoader - Loads and parses Pendragon VM binary files
     * 
     * Reads binary files in the Pendragon VM format and validates:
     * - File format version compatibility
     * - Machine name and version
     * - Segment sizes and integrity
     */
    class BinaryLoader {
    public:
        BinaryLoader();
        
        /**
         * Load a binary file from disk
         * 
         * @param filename Path to the binary file
         * @return BinaryProgram structure containing header and segments
         * @throws runtime_error if file cannot be read or format is invalid
         */
        BinaryProgram load_file(const std::string& filename);
        
        /**
         * Load a binary from a byte vector (for testing)
         * 
         * @param data Raw binary data
         * @return BinaryProgram structure containing header and segments
         * @throws runtime_error if format is invalid
         */
        BinaryProgram load_from_bytes(const std::vector<byte_t>& data);
        
        /**
         * Get expected machine name for this VM
         */
        static std::string get_expected_machine_name();
        
        /**
         * Get expected machine version for this VM
         */
        static BinaryVersion get_expected_machine_version();
        
    private:
        BinaryHeader parse_header(const byte_t* data, size_t data_size, size_t& offset);
        void parse_program_segments(const byte_t* data, size_t data_size, size_t& offset, BinaryProgram& program);
        
        uint16_t read_uint16(const byte_t* data, size_t offset) const;
        uint32_t read_uint32(const byte_t* data, size_t offset) const;
        std::string read_string(const byte_t* data, size_t offset, size_t length) const;
        
        void validate_header(const BinaryHeader& header);
    };

} // namespace lvm
