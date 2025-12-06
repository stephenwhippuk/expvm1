# Helpers - Utility Functions

## Purpose

The Helpers module provides utility functions for byte manipulation, word construction, and address calculation. These functions ensure consistent little-endian byte ordering throughout the Pendragon VM.

## Architecture Overview

### Core Concepts

- **Little-Endian Convention**: Low byte comes first in memory/instruction stream
- **Type Safety**: Explicit casting to prevent type confusion
- **Address Construction**: Combine two bytes into 16-bit addresses
- **Word Construction**: Build 16-bit words from byte pairs

### Byte Ordering

```
Little-Endian Word Layout:
Memory/Stream:  [low_byte] [high_byte]
                    ↓          ↓
Word Value:     0xHHLL
                  ││
                  │└─ Low byte  (bits 0-7)
                  └── High byte (bits 8-15)

Example:
Bytes: [0x34] [0x12]
Word:  0x1234
```

## Interface

### Helper Functions

```cpp
namespace lvm {
    // Combine bytes into 16-bit address
    addr_t combine_bytes_to_address(byte_t high, byte_t low);
    
    // Combine bytes into 16-bit word
    word_t combine_bytes_to_word(byte_t high, byte_t low);
}
```

### Type Definitions

```cpp
// From memsize.h
typedef uint8_t  byte_t;   // 8-bit byte
typedef uint16_t word_t;   // 16-bit word
typedef uint16_t addr_t;   // 16-bit address (code/data)
typedef uint32_t addr32_t; // 32-bit address (stack)
```

## Usage Examples

### Example 1: Basic Word Construction

```cpp
#include "helpers.h"

// Build word from individual bytes
byte_t low_byte = 0x34;
byte_t high_byte = 0x12;

word_t value = combine_bytes_to_word(high_byte, low_byte);
// value = 0x1234

// Note: Parameters are (high, low) but little-endian is low-first in memory
// The function handles this correctly
```

### Example 2: Address Calculation from Instruction Stream

```cpp
#include "helpers.h"

// Instruction stream (little-endian)
std::vector<byte_t> program = {
    0x36,        // JMP opcode
    0x20, 0x10,  // Target address: 0x1020 (little-endian: 0x20, 0x10)
    // ...
};

// Parse address from instruction
byte_t addr_low = program[1];   // 0x20
byte_t addr_high = program[2];  // 0x10

addr_t jump_target = combine_bytes_to_address(addr_high, addr_low);
// jump_target = 0x1020

// Or directly from sequential access
addr_t target = combine_bytes_to_address(program[2], program[1]);
// target = 0x1020
```

### Example 3: Immediate Value Loading

```cpp
// Instruction: LD AX, 0x5678
// Bytecode: [0x00] [0x01] [0x78] [0x56]
//           opcode  reg   low   high

std::vector<byte_t> instruction = {0x00, 0x01, 0x78, 0x56};

byte_t opcode = instruction[0];  // 0x00
byte_t reg = instruction[1];     // 0x01 (AX)
byte_t imm_low = instruction[2];  // 0x78
byte_t imm_high = instruction[3]; // 0x56

word_t immediate = combine_bytes_to_word(imm_high, imm_low);
// immediate = 0x5678

// Load into register
register_ax.set_value(immediate);
```

### Example 4: Memory Address Construction

```cpp
#include "helpers.h"

// Store instruction: STA 0x2000, AX
// Bytecode: [0x0E] [0x00] [0x20] [0x01]
//           opcode  low    high   reg

std::vector<byte_t> store_instr = {0x0E, 0x00, 0x20, 0x01};

byte_t addr_low = store_instr[1];   // 0x00
byte_t addr_high = store_instr[2];  // 0x20

addr_t memory_address = combine_bytes_to_address(addr_high, addr_low);
// memory_address = 0x2000

// Use address for memory operation
memory_accessor->write_word(memory_address, register_value);
```

### Example 5: Parsing Multi-Byte Parameters

```cpp
// CPU instruction decoder
byte_t opcode = fetch_byte();

// Get parameter count
int param_bytes = get_additional_bytes(opcode);

std::vector<byte_t> params;
for (int i = 0; i < param_bytes; ++i) {
    params.push_back(fetch_byte());
}

// For instructions with 16-bit immediate
if (param_bytes == 2) {
    // Parameters are stored low-byte first
    word_t immediate = combine_bytes_to_word(params[1], params[0]);
    // immediate = (params[1] << 8) | params[0]
}

// For instructions with address
if (opcode == OPCODE_JMP_ADDR) {
    // params[0] = low byte, params[1] = high byte
    addr_t target = combine_bytes_to_address(params[1], params[0]);
    jump_to(target);
}
```

### Example 6: Reading Little-Endian Words from Memory

```cpp
// Memory contains: [0x34, 0x12, 0x78, 0x56, ...]
//                   addr   addr+1 addr+2 addr+3

PagedMemoryAccessor accessor;
addr_t base_addr = 0x1000;

// Read first word
byte_t byte0 = accessor.read_byte(base_addr);      // 0x34
byte_t byte1 = accessor.read_byte(base_addr + 1);  // 0x12

word_t word1 = combine_bytes_to_word(byte1, byte0);
// word1 = 0x1234

// Read second word
byte_t byte2 = accessor.read_byte(base_addr + 2);  // 0x78
byte_t byte3 = accessor.read_byte(base_addr + 3);  // 0x56

word_t word2 = combine_bytes_to_word(byte3, byte2);
// word2 = 0x5678
```

### Example 7: Constructing Jump Tables

```cpp
// Jump table in memory (little-endian addresses)
std::vector<byte_t> jump_table = {
    0x00, 0x10,  // Entry 0: 0x1000
    0x20, 0x10,  // Entry 1: 0x1020
    0x40, 0x10,  // Entry 2: 0x1040
    0x60, 0x10,  // Entry 3: 0x1060
};

// Get address for entry N
int entry = 2;
int offset = entry * 2;  // Each entry is 2 bytes

byte_t low = jump_table[offset];
byte_t high = jump_table[offset + 1];

addr_t target = combine_bytes_to_address(high, low);
// target = 0x1040
```

### Example 8: Split and Recombine

```cpp
// Start with a word
word_t original = 0xABCD;

// Split into bytes
byte_t high = (original >> 8) & 0xFF;  // 0xAB
byte_t low = original & 0xFF;          // 0xCD

// Store in little-endian order
std::vector<byte_t> buffer = {low, high};  // [0xCD, 0xAB]

// Later, reconstruct
word_t reconstructed = combine_bytes_to_word(buffer[1], buffer[0]);
// reconstructed = 0xABCD
assert(reconstructed == original);
```

### Example 9: Endianness Conversion

```cpp
// Convert big-endian data to little-endian
byte_t big_endian_data[] = {0x12, 0x34};  // Big-endian: high byte first

// Read as little-endian (swap bytes)
word_t value = combine_bytes_to_word(big_endian_data[0], big_endian_data[1]);
// value = 0x1234 (correct)

// If we had accidentally used them in wrong order:
word_t wrong = combine_bytes_to_word(big_endian_data[1], big_endian_data[0]);
// wrong = 0x3412 (byte-swapped)
```

### Example 10: Address Arithmetic with Helpers

```cpp
// Calculate target address from base + offset
addr_t base = 0x2000;
word_t offset = 0x0100;

addr_t target = base + offset;  // 0x2100

// Split target into bytes for transmission
byte_t target_low = target & 0xFF;        // 0x00
byte_t target_high = (target >> 8) & 0xFF; // 0x21

// Reconstruct at destination
addr_t received_addr = combine_bytes_to_address(target_high, target_low);
// received_addr = 0x2100
assert(received_addr == target);
```

## Key Design Patterns

### Little-Endian Parameter Order

**In memory/instruction stream** (left to right):
```
[low_byte] [high_byte]
```

**In function calls**:
```cpp
combine_bytes_to_word(high_byte, low_byte)  // Parameters: high first, low second
```

This convention matches the natural reading order when parsing:
```cpp
byte_t low = fetch_byte();   // First byte in stream
byte_t high = fetch_byte();  // Second byte in stream
word_t value = combine_bytes_to_word(high, low);
```

### Type Safety Pattern

```cpp
word_t combine_bytes_to_word(byte_t high, byte_t low) {
    return static_cast<word_t>(
        (static_cast<word_t>(high) << 8) | static_cast<word_t>(low)
    );
}
```

Key points:
1. Cast `high` to `word_t` before shift (prevents overflow)
2. Cast `low` to `word_t` before OR (consistent types)
3. Return type is explicit `word_t`

### Symmetric Address/Word Functions

Both `combine_bytes_to_address()` and `combine_bytes_to_word()` have identical implementations but different return types:

```cpp
addr_t combine_bytes_to_address(byte_t high, byte_t low);  // Returns addr_t (uint16_t)
word_t combine_bytes_to_word(byte_t high, byte_t low);     // Returns word_t (uint16_t)
```

Use `combine_bytes_to_address()` for clarity when working with addresses, `combine_bytes_to_word()` for data values.

## Implementation Details

### Bitwise Construction

```cpp
// Given: high = 0x12, low = 0x34
word_t result = combine_bytes_to_word(0x12, 0x34);

// Step by step:
// 1. Cast high to word_t:        0x0012
// 2. Shift left 8 bits:          0x1200
// 3. Cast low to word_t:         0x0034
// 4. Bitwise OR:                 0x1200 | 0x0034 = 0x1234
// 5. Return:                     0x1234
```

### Why Explicit Casts?

Without proper casting, byte shift operations can produce unexpected results:

```cpp
// WRONG: byte_t shifted directly
byte_t high = 0x12;
word_t bad_result = (high << 8) | low;  // high is still byte_t, shift may overflow

// CORRECT: cast before shift
word_t good_result = (static_cast<word_t>(high) << 8) | low;  // Safe
```

## Common Patterns in Pendragon VM

### Instruction Decoding Pattern

```cpp
// Fetch opcode
byte_t opcode = read_byte_at_IR();
advance_IR(1);

// Fetch parameters (if any)
int param_count = get_additional_bytes(opcode);
std::vector<byte_t> params;
for (int i = 0; i < param_count; ++i) {
    params.push_back(read_byte_at_IR());
    advance_IR(1);
}

// Decode parameters
if (param_count == 2) {
    // Little-endian: params[0] is low, params[1] is high
    word_t value = combine_bytes_to_word(params[1], params[0]);
}
```

### Memory Write Pattern

```cpp
// Write word to memory (little-endian)
void write_word(addr_t address, word_t value) {
    byte_t low = value & 0xFF;
    byte_t high = (value >> 8) & 0xFF;
    
    write_byte(address, low);       // Low byte first
    write_byte(address + 1, high);  // High byte second
}

// Read word from memory
word_t read_word(addr_t address) {
    byte_t low = read_byte(address);
    byte_t high = read_byte(address + 1);
    
    return combine_bytes_to_word(high, low);
}
```

## Error Handling

The helper functions perform no error checking:
- No validation of byte values (all 8-bit values are valid)
- No overflow checking (results fit naturally in 16-bit types)
- No null pointer checks (primitive types)

This is intentional for performance - callers are responsible for data validity.

## Performance Considerations

- **Combine Operations**: O(1) - Two shifts and one OR
- **No Branching**: Deterministic execution time
- **Inline Candidates**: Simple functions suitable for compiler inlining
- **No Allocations**: Stack-only operations

## Thread Safety

Helper functions are thread-safe:
- No shared state
- No side effects  
- Pure functions (same inputs → same outputs)
- Can be called concurrently from multiple threads

## Usage Guidelines

1. **Always use helpers for multi-byte values**: Don't manually combine bytes
2. **Parameter order**: Remember (high, low) even though memory is low-first
3. **Type clarity**: Use `combine_bytes_to_address()` for addresses
4. **Consistency**: Apply little-endian convention throughout codebase

## Future Extensions

Potential additions to helpers module:
- `split_word_to_bytes(word_t value)` → returns `{low, high}` pair
- `reverse_bytes(word_t value)` → endian swap
- `sign_extend_byte(byte_t value)` → extend to signed word
- `truncate_to_byte(word_t value)` → extract low byte with explicit semantics
