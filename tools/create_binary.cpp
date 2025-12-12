#include <iostream>
#include <fstream>
#include <vector>
#include "memsize.h"

using namespace lvm;

/**
 * Simple utility to create a Pendragon VM binary file
 * 
 * Creates a minimal binary with:
 * - Header (Pendragon 1.0.0)
 * - Small data segment
 * - Simple code segment (HALT instruction)
 */

void write_uint16(std::ofstream& file, uint16_t value) {
    file.put(value & 0xFF);
    file.put((value >> 8) & 0xFF);
}

void write_uint32(std::ofstream& file, uint32_t value) {
    file.put(value & 0xFF);
    file.put((value >> 8) & 0xFF);
    file.put((value >> 16) & 0xFF);
    file.put((value >> 24) & 0xFF);
}

void write_string(std::ofstream& file, const std::string& str) {
    for (char c : str) {
        file.put(c);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output.bin>" << std::endl;
        return 1;
    }
    
    std::ofstream file(argv[1], std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create file: " << argv[1] << std::endl;
        return 1;
    }
    
    std::string machine_name = "Pendragon";
    std::string program_name = "HelloWorld";
    
    // Calculate header size
    uint16_t header_size = 2 +  // header size itself
                          4 +   // header version
                          1 + machine_name.size() +  // machine name
                          4 +   // machine version
                          2 + program_name.size();   // program name
    
    // Write header size
    write_uint16(file, header_size);
    
    // Write header version (1.0.0)
    file.put(1);  // major
    file.put(0);  // minor
    file.put(0);  // revision high byte
    file.put(0);  // revision low byte
    
    // Write machine name
    file.put(static_cast<byte_t>(machine_name.size()));
    write_string(file, machine_name);
    
    // Write machine version (1.0.0)
    file.put(1);  // major
    file.put(0);  // minor
    file.put(0);  // revision high byte
    file.put(0);  // revision low byte
    
    // Write program name
    write_uint16(file, program_name.size());
    write_string(file, program_name);
    
    // Write data segment (empty for now)
    write_uint32(file, 0);
    
    // Write code segment (just HALT instruction: 0x00)
    std::vector<byte_t> code = {0x00};  // HALT
    write_uint32(file, code.size());
    for (byte_t b : code) {
        file.put(b);
    }
    
    file.close();
    
    std::cout << "Created binary file: " << argv[1] << std::endl;
    std::cout << "  Machine: " << machine_name << " v1.0.0" << std::endl;
    std::cout << "  Program: " << program_name << std::endl;
    std::cout << "  Data segment: 0 bytes" << std::endl;
    std::cout << "  Code segment: " << code.size() << " bytes" << std::endl;
    
    return 0;
}
