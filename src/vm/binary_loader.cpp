#include "binary_loader.h"
#include "errors.h"
#include <fstream>
#include <sstream>
#include <cstring>

using namespace lvm;

// Expected machine name and version for Pendragon VM
static const std::string EXPECTED_MACHINE_NAME = "Pendragon";
static const BinaryVersion EXPECTED_MACHINE_VERSION(1, 0, 0);
static const BinaryVersion SUPPORTED_HEADER_VERSION(1, 0, 0);

std::string BinaryVersion::to_string() const {
    std::ostringstream oss;
    oss << static_cast<int>(major) << "." 
        << static_cast<int>(minor) << "." 
        << revision;
    return oss.str();
}

BinaryLoader::BinaryLoader() {
}

std::string BinaryLoader::get_expected_machine_name() {
    return EXPECTED_MACHINE_NAME;
}

BinaryVersion BinaryLoader::get_expected_machine_version() {
    return EXPECTED_MACHINE_VERSION;
}

BinaryProgram BinaryLoader::load_file(const std::string& filename) {
    // Open file in binary mode
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw runtime_error("Failed to open binary file: " + filename);
    }
    
    // Get file size
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read entire file into buffer
    std::vector<byte_t> buffer(file_size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        throw runtime_error("Failed to read binary file: " + filename);
    }
    
    file.close();
    
    return load_from_bytes(buffer);
}

BinaryProgram BinaryLoader::load_from_bytes(const std::vector<byte_t>& data) {
    if (data.size() < 6) {  // Minimum header size
        throw runtime_error("Binary data too small to be valid");
    }
    
    BinaryProgram program;
    size_t offset = 0;
    
    // Parse header
    program.header = parse_header(data.data(), data.size(), offset);
    
    // Validate header
    validate_header(program.header);
    
    // Parse program segments
    parse_program_segments(data.data(), data.size(), offset, program);
    
    return program;
}

BinaryHeader BinaryLoader::parse_header(const byte_t* data, size_t data_size, size_t& offset) {
    BinaryHeader header;
    
    // Read header size (2 bytes)
    if (offset + 2 > data_size) {
        throw runtime_error("Unexpected end of data reading header size");
    }
    header.header_size = read_uint16(data, offset);
    offset += 2;
    
    // Read header version (4 bytes)
    if (offset + 4 > data_size) {
        throw runtime_error("Unexpected end of data reading header version");
    }
    header.header_version.major = data[offset++];
    header.header_version.minor = data[offset++];
    header.header_version.revision = read_uint16(data, offset);
    offset += 2;
    
    // Read machine name size (1 byte)
    if (offset + 1 > data_size) {
        throw runtime_error("Unexpected end of data reading machine name size");
    }
    uint8_t machine_name_size = data[offset++];
    
    // Read machine name
    if (offset + machine_name_size > data_size) {
        throw runtime_error("Unexpected end of data reading machine name");
    }
    header.machine_name = read_string(data, offset, machine_name_size);
    offset += machine_name_size;
    
    // Read machine version (4 bytes)
    if (offset + 4 > data_size) {
        throw runtime_error("Unexpected end of data reading machine version");
    }
    header.machine_version.major = data[offset++];
    header.machine_version.minor = data[offset++];
    header.machine_version.revision = read_uint16(data, offset);
    offset += 2;
    
    // Read program name size (2 bytes)
    if (offset + 2 > data_size) {
        throw runtime_error("Unexpected end of data reading program name size");
    }
    uint16_t program_name_size = read_uint16(data, offset);
    offset += 2;
    
    // Read program name
    if (offset + program_name_size > data_size) {
        throw runtime_error("Unexpected end of data reading program name");
    }
    header.program_name = read_string(data, offset, program_name_size);
    offset += program_name_size;
    
    return header;
}

void BinaryLoader::parse_program_segments(const byte_t* data, size_t data_size, size_t& offset, BinaryProgram& program) {
    // Read data segment size (4 bytes)
    if (offset + 4 > data_size) {
        throw runtime_error("Unexpected end of data reading data segment size");
    }
    uint32_t data_segment_size = read_uint32(data, offset);
    offset += 4;
    
    // Read data segment
    if (offset + data_segment_size > data_size) {
        throw runtime_error("Unexpected end of data reading data segment");
    }
    program.data_segment.resize(data_segment_size);
    std::memcpy(program.data_segment.data(), data + offset, data_segment_size);
    offset += data_segment_size;
    
    // Read code segment size (4 bytes)
    if (offset + 4 > data_size) {
        throw runtime_error("Unexpected end of data reading code segment size");
    }
    uint32_t code_segment_size = read_uint32(data, offset);
    offset += 4;
    
    // Read code segment
    if (offset + code_segment_size > data_size) {
        throw runtime_error("Unexpected end of data reading code segment");
    }
    program.code_segment.resize(code_segment_size);
    std::memcpy(program.code_segment.data(), data + offset, code_segment_size);
    offset += code_segment_size;
}

void BinaryLoader::validate_header(const BinaryHeader& header) {
    // Validate header version
    if (header.header_version != SUPPORTED_HEADER_VERSION) {
        throw runtime_error(
            "Unsupported binary format version: " + header.header_version.to_string() +
            " (expected " + SUPPORTED_HEADER_VERSION.to_string() + ")"
        );
    }
    
    // Validate machine name
    if (header.machine_name != EXPECTED_MACHINE_NAME) {
        throw runtime_error(
            "Binary is for machine '" + header.machine_name + 
            "' but this is '" + EXPECTED_MACHINE_NAME + "'"
        );
    }
    
    // Validate machine version (must match exactly for now)
    if (header.machine_version != EXPECTED_MACHINE_VERSION) {
        throw runtime_error(
            "Binary requires machine version " + header.machine_version.to_string() +
            " but this is version " + EXPECTED_MACHINE_VERSION.to_string()
        );
    }
}

uint16_t BinaryLoader::read_uint16(const byte_t* data, size_t offset) const {
    // Little-endian read
    return static_cast<uint16_t>(data[offset]) | 
           (static_cast<uint16_t>(data[offset + 1]) << 8);
}

uint32_t BinaryLoader::read_uint32(const byte_t* data, size_t offset) const {
    // Little-endian read
    return static_cast<uint32_t>(data[offset]) | 
           (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) |
           (static_cast<uint32_t>(data[offset + 3]) << 24);
}

std::string BinaryLoader::read_string(const byte_t* data, size_t offset, size_t length) const {
    return std::string(reinterpret_cast<const char*>(data + offset), length);
}
