#include <iostream>
#include <fstream>
#include <vector>
#include "memsize.h"

using namespace lvm;

/**
 * Creates the Hello World binary for Pendragon VM
 * 
 * This program pushes "Hello, World!" onto the stack and calls
 * the PRINT_LINE_FROM_STACK system call (0x0011)
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
    
    // Write data segment (empty)
    write_uint32(file, 0);
    
    // Write code segment
    // Hello World program that uses PRINT_LINE_FROM_STACK system call
    std::vector<byte_t> code = {
        // Push "Hello, World!" in reverse order (13 characters)
        0x76, 0x21,        // PUSHB '!'
        0x76, 0x64,        // PUSHB 'd'
        0x76, 0x6C,        // PUSHB 'l'
        0x76, 0x72,        // PUSHB 'r'
        0x76, 0x6F,        // PUSHB 'o'
        0x76, 0x57,        // PUSHB 'W'
        0x76, 0x20,        // PUSHB ' '
        0x76, 0x2C,        // PUSHB ','
        0x76, 0x6F,        // PUSHB 'o'
        0x76, 0x6C,        // PUSHB 'l'
        0x76, 0x6C,        // PUSHB 'l'
        0x76, 0x65,        // PUSHB 'e'
        0x76, 0x48,        // PUSHB 'H'
        
        // Push character count (13 = 0x000D)
        0x75, 0x0D, 0x00,  // PUSHW 13
        
        // Call PRINT_LINE_FROM_STACK (0x0011)
        0x7F, 0x11, 0x00,  // SYSCALL 0x0011
        
        // Halt
        0x01               // HALT
    };
    
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
    std::cout << "\nProgram will print 'Hello, World!' and halt." << std::endl;
    
    return 0;
}
