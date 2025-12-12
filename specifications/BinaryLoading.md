# Binary File Loading Implementation

## Overview

The binary file loading feature allows the Pendragon VM to load compiled programs from disk. Programs are stored in a standardized binary format (Version 1.0.0) that includes header metadata and separate data/code segments.

## Components

### 1. BinaryLoader Class

**Location:** `src/vm/include/binary_loader.h`, `src/vm/binary_loader.cpp`

The `BinaryLoader` class is responsible for:
- Reading binary files from disk
- Parsing the binary format header
- Validating version compatibility
- Extracting data and code segments
- Error handling for invalid files

**Key Methods:**
- `load_file(const std::string& filename)` - Load from file on disk
- `load_from_bytes(const std::vector<byte_t>& data)` - Load from memory (for testing)
- `get_expected_machine_name()` - Returns "Pendragon"
- `get_expected_machine_version()` - Returns version 1.0.0

### 2. Binary Format Structures

**BinaryVersion:**
- Represents semantic version (major.minor.revision)
- Supports equality comparison
- Provides string conversion for error messages

**BinaryHeader:**
- `header_size` - Total size of header in bytes
- `header_version` - Format version (currently 1.0.0)
- `machine_name` - Target machine name (must be "Pendragon")
- `machine_version` - Required VM version (must be 1.0.0)
- `program_name` - Human-readable program name

**BinaryProgram:**
- Complete representation of loaded program
- Contains header plus data and code segments

### 3. VM Integration

**Location:** `src/vm/vm.cpp`

The `vm::load_program()` method has been updated to:
1. Instantiate a `BinaryLoader`
2. Call `load_file()` to parse the binary
3. Validate compatibility
4. Load code segment into CPU
5. Provide descriptive error messages

**Current Implementation:**
```cpp
void vm::load_program(char* fileName, addr_t load_address) {
    BinaryLoader loader;
    
    try {
        BinaryProgram program = loader.load_file(fileName);
        // Load code segment into CPU
        cpu_instance->load_program(program.code_segment);
    } catch (const runtime_error& e) {
        throw runtime_error("Failed to load program '" + 
                          std::string(fileName) + "': " + e.what());
    }
}
```

**Note:** Data segment loading is not yet implemented (marked with TODO).

## Binary Format Specification

The binary format follows the specification in `specifications/binary-format.md`:

### Header Section
```
Offset | Size | Field             | Description
-------|------|-------------------|----------------------------------
0      | 2    | Header Size       | Total header size in bytes
2      | 4    | Header Version    | Format version (major.minor.revision)
6      | 1    | Machine Name Size | Length of machine name
7      | n    | Machine Name      | Target machine name (e.g., "Pendragon")
7+n    | 4    | Machine Version   | Required VM version
11+n   | 2    | Program Name Size | Length of program name
13+n   | m    | Program Name      | Human-readable program name
```

### Program Section
```
Offset     | Size | Field            | Description
-----------|------|------------------|----------------------------------
13+n+m     | 4    | Data Segment Size| Size of data segment in bytes
17+n+m     | d    | Data Segment     | Initialized data
17+n+m+d   | 4    | Code Segment Size| Size of code segment in bytes
21+n+m+d   | c    | Code Segment     | Executable bytecode
```

**All multi-byte values are stored in little-endian format.**

## Validation

The loader performs the following validations:

1. **File Size:** Minimum 6 bytes for basic header
2. **Header Version:** Must be exactly 1.0.0
3. **Machine Name:** Must be "Pendragon"
4. **Machine Version:** Must be exactly 1.0.0
5. **Segment Sizes:** Must not exceed remaining file size
6. **Data Integrity:** All advertised bytes must be present

**Error Handling:**
- File not found: `runtime_error` with file name
- Invalid format: `runtime_error` with specific issue
- Version mismatch: `runtime_error` with expected vs actual versions
- Truncated data: `runtime_error` indicating incomplete read

## Testing

### Unit Tests

**Location:** `src/vm/tests/binary_loader_tests.cpp`

Comprehensive test suite covering:
- Valid binary loading with data and code segments
- Empty segments
- Invalid machine names
- Invalid machine versions
- Invalid header versions
- Truncated binaries
- Version string formatting
- Version comparison operators
- Long program names (500+ characters)
- Large segments (10KB+ code, 5KB+ data)

**Test Results:** All 11 binary loader tests pass (104 total tests)

## Tools

### create_binary

**Location:** `tools/create_binary.cpp`

Simple utility to create valid Pendragon VM binary files.

**Usage:**
```bash
./create_binary <output.bin>
```

**Creates:**
- Header with Pendragon 1.0.0 machine info
- Program name "HelloWorld"
- Empty data segment
- Single-byte code segment (HALT instruction: 0x00)

**Example:**
```bash
$ ./create_binary test_program.bin
Created binary file: test_program.bin
  Machine: Pendragon v1.0.0
  Program: HelloWorld
  Data segment: 0 bytes
  Code segment: 1 bytes
```

## Future Enhancements

1. **Data Segment Loading:** Currently not implemented. Need to:
   - Create/manage data context in VM
   - Load data segment at specified address
   - Provide accessor for programs to read initialized data

2. **Version Compatibility:** Current implementation requires exact version match. Could support:
   - Backward compatibility (load older binaries)
   - Major version checking (allow minor/revision differences)

3. **Metadata Extensions:** Could add to header:
   - Entry point address
   - Required memory sizes
   - Required system calls
   - Debug symbols

4. **File Format Validation:** Could add:
   - Checksums for data integrity
   - Magic number at file start
   - Signature verification

5. **Symbol Table:** For debugging:
   - Function names and addresses
   - Variable names and locations
   - Source line mapping

## Dependencies

### Internal
- `memsize.h` - Type definitions (byte_t, addr_t, etc.)
- `errors.h` - Runtime error handling
- `vm.h` - VM interface
- `cpu.h` - CPU load_program() method

### External
- `<fstream>` - File I/O
- `<vector>` - Dynamic arrays
- `<string>` - String handling
- `<cstring>` - memcpy for byte copying
- `<cstdint>` - Fixed-width integer types

## Build Integration

### CMake Changes

**src/vm/CMakeLists.txt:**
```cmake
add_library(lvm_vm STATIC
    vm.cpp
    binary_loader.cpp  # Added
)

# Add tests subdirectory
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

**CMakeLists.txt (root):**
```cmake
# Binary creation tool
add_executable(create_binary tools/create_binary.cpp)
target_link_libraries(create_binary PRIVATE lvm_helpers)
```

### Build Artifacts

After building, the following executables are created:
- `build/lvm` - Main VM executable
- `build/create_binary` - Binary file creation utility
- `build/src/vm/tests/lvm_vm_tests` - Binary loader test suite

## Usage Example

### Creating a Binary

```cpp
// Use create_binary tool or programmatically create
std::ofstream file("program.bin", std::ios::binary);
// ... write header and segments according to format ...
file.close();
```

### Loading into VM

```cpp
#include "vm.h"

lvm::vm virtual_machine(1024, 4096);  // stack=1KB, code=4KB

try {
    virtual_machine.load_program("program.bin", 0);
    virtual_machine.run();
} catch (const lvm::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Status

✅ **Completed:**
- BinaryLoader class with full parsing
- Header validation (version, machine name/version)
- Segment extraction (data and code)
- Error handling and reporting
- Unit test suite (11 tests)
- Integration with VM
- create_binary tool
- Documentation

⏳ **Pending:**
- Data segment loading into memory context
- Integration tests with full VM execution
- Additional tools (disassembler, binary inspector)

## Version History

- **1.0.0** - Initial implementation
  - Basic binary format support
  - Header validation
  - Code segment loading
  - Unit tests
  - create_binary tool
