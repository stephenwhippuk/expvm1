# Register and Flags - General-Purpose Storage

## Purpose

The Register class provides 16-bit general-purpose storage with optional flag management, supporting both full-word and split high/low byte access. The Flags class manages CPU status flags (ZERO, CARRY, SIGN, OVERFLOW) that control conditional operations and reflect arithmetic results.

## Architecture Overview

### Register Structure

```
16-bit Register Layout:
┌────────────────┬────────────────┐
│   High Byte    │   Low Byte     │
│   (bits 8-15)  │   (bits 0-7)   │
└────────────────┴────────────────┘
     get/set          get/set
   high_byte()      low_byte()
        ↓                ↓
    [8 bits]         [8 bits]
```

### Flags Register

```
Flags Byte (8-bit):
┌───┬───┬───┬───┬───┬───┬───┬───┐
│ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
└───┴───┴───┴─┬─┴─┬─┴─┬─┴─┬─┴───┘
              │   │   │   │
              V   S   C   Z
         OVERFLOW │   │ ZERO
              SIGN  CARRY
```

### Flag Semantics

| Flag | Bit | Mask | Meaning |
|------|-----|------|---------|
| **ZERO** | 0 | 0x01 | Set when result is zero |
| **CARRY** | 1 | 0x02 | Set on unsigned overflow or borrow |
| **SIGN** | 2 | 0x04 | Set when bit 15 of result is 1 (negative) |
| **OVERFLOW** | 3 | 0x08 | Set on signed overflow |

## Interface

### Register Class

```cpp
class Register {
    // Construction
    Register();                                    // No flags
    Register(std::shared_ptr<Flags> flags_ptr);   // With shared flags
    Register(const Register& other);              // Copy
    ~Register();
    
    // Full word access
    void set_value(word_t value);
    word_t get_value() const;
    void clear();
    
    // Byte-level access
    void set_high_byte(byte_t high);
    void set_low_byte(byte_t low);
    byte_t get_high_byte() const;
    byte_t get_low_byte() const;
    
    // Flag operations (only if flags provided)
    bool has_flags() const;
    bool set_flag(Flag flag);
    bool clear_flag(Flag flag);
    bool is_flag_set(Flag flag) const;
    
    // Increment/Decrement (sets flags if available)
    void inc();
    void dec();
};
```

### Flags Class

```cpp
class Flags {
    // Construction
    Flags();
    ~Flags();
    
    // Flag manipulation
    void set(Flag flag);
    void clear(Flag flag);
    bool is_set(Flag flag) const;
    void clear_all();
};

enum class Flag {
    ZERO     = 0x01,
    CARRY    = 0x02,
    SIGN     = 0x04,
    OVERFLOW = 0x08
};
```

## Usage Examples

### Example 1: Basic Register Operations

```cpp
#include "register.h"

// Create register without flags
Register bx;

// Set full value
bx.set_value(0x1234);

// Get full value
word_t value = bx.get_value();  // 0x1234

// Access individual bytes
byte_t high = bx.get_high_byte();  // 0x12
byte_t low = bx.get_low_byte();    // 0x34

// Modify individual bytes
bx.set_low_byte(0x56);
// bx = 0x1256

bx.set_high_byte(0x78);
// bx = 0x7856

// Clear register
bx.clear();
// bx = 0x0000
```

### Example 2: Register with Flags

```cpp
#include "register.h"
#include "flags.h"

// Create shared flags
auto flags = std::make_shared<Flags>();

// Create register with flags
Register ax(flags);

// Check if register has flags
if (ax.has_flags()) {
    // Can use flag operations
    ax.set_flag(Flag::ZERO);
    ax.set_flag(Flag::CARRY);
}

// Set value and manually update flags
ax.set_value(0);
if (ax.get_value() == 0) {
    ax.set_flag(Flag::ZERO);
}

// Check flag state
if (ax.is_flag_set(Flag::ZERO)) {
    // Zero flag is set
}

// Clear individual flag
ax.clear_flag(Flag::CARRY);
```

### Example 3: High/Low Byte Manipulation

```cpp
Register ax;

// Build word from bytes
ax.set_low_byte(0xCD);
ax.set_high_byte(0xAB);
// ax = 0xABCD

// Extract bytes
byte_t low = ax.get_low_byte();   // 0xCD
byte_t high = ax.get_high_byte(); // 0xAB

// Modify only low byte
ax.set_value(0x1234);
ax.set_low_byte(0xFF);
// ax = 0x12FF (high byte preserved)

// Modify only high byte
ax.set_value(0x1234);
ax.set_high_byte(0x00);
// ax = 0x0034 (low byte preserved)

// Swap bytes
word_t original = 0x1234;
ax.set_value(original);
byte_t temp_high = ax.get_high_byte();
byte_t temp_low = ax.get_low_byte();
ax.set_high_byte(temp_low);
ax.set_low_byte(temp_high);
// ax = 0x3412 (bytes swapped)
```

### Example 4: Increment and Decrement

```cpp
auto flags = std::make_shared<Flags>();
Register ax(flags);

// Increment
ax.set_value(10);
ax.inc();
// ax = 11, flags updated

// Increment with overflow
ax.set_value(0xFFFF);
ax.inc();
// ax = 0x0000
// ZERO flag SET
// CARRY flag SET (wrapped)

// Decrement
ax.set_value(10);
ax.dec();
// ax = 9, flags updated

// Decrement with underflow
ax.set_value(0);
ax.dec();
// ax = 0xFFFF
// CARRY flag SET (borrowed)
// SIGN flag SET (appears negative)
```

### Example 5: Shared Flags Across Registers

```cpp
// Multiple registers sharing same flags
auto shared_flags = std::make_shared<Flags>();

Register ax(shared_flags);
Register bx(shared_flags);

// Set value in AX
ax.set_value(0);

// Set zero flag via AX
ax.set_flag(Flag::ZERO);

// BX sees same flags
if (bx.is_flag_set(Flag::ZERO)) {
    // This is true - flags are shared
}

// Clear flag via BX
bx.clear_flag(Flag::ZERO);

// AX sees the change
if (!ax.is_flag_set(Flag::ZERO)) {
    // This is true - flags are synchronized
}
```

### Example 6: Flags Direct Manipulation

```cpp
#include "flags.h"

Flags status_flags;

// Set multiple flags
status_flags.set(Flag::ZERO);
status_flags.set(Flag::CARRY);

// Check individual flags
bool is_zero = status_flags.is_set(Flag::ZERO);      // true
bool is_carry = status_flags.is_set(Flag::CARRY);    // true
bool is_sign = status_flags.is_set(Flag::SIGN);      // false

// Clear individual flag
status_flags.clear(Flag::CARRY);

// Check after clear
is_carry = status_flags.is_set(Flag::CARRY);  // false
is_zero = status_flags.is_set(Flag::ZERO);    // true (still set)

// Clear all flags
status_flags.clear_all();

// All flags now clear
is_zero = status_flags.is_set(Flag::ZERO);    // false
```

### Example 7: Conditional Logic Using Flags

```cpp
auto flags = std::make_shared<Flags>();
Register ax(flags);

// Simulate comparison: AX - 10
word_t ax_value = ax.get_value();
word_t compare_value = 10;

// Set flags based on comparison
flags->clear_all();

if (ax_value == compare_value) {
    flags->set(Flag::ZERO);
}

if (ax_value < compare_value) {
    flags->set(Flag::CARRY);  // Borrow would occur
}

word_t result = ax_value - compare_value;
if (result & 0x8000) {
    flags->set(Flag::SIGN);  // Result negative
}

// Use flags for branching
if (flags->is_set(Flag::ZERO)) {
    // Equal path
}
else if (flags->is_set(Flag::CARRY)) {
    // Less than path
}
else {
    // Greater than path
}
```

### Example 8: Register Copy and Independence

```cpp
auto flags1 = std::make_shared<Flags>();
Register ax(flags1);
ax.set_value(0x1234);

// Copy constructor
Register cx(ax);
cx.set_value(0x5678);

// Original unchanged
assert(ax.get_value() == 0x1234);
assert(cx.get_value() == 0x5678);

// Flags are also copied (reference to same flags object)
ax.set_flag(Flag::ZERO);
if (cx.is_flag_set(Flag::ZERO)) {
    // This is true - they share the same flags object
}

// Create independent register
auto flags2 = std::make_shared<Flags>();
Register dx(flags2);

ax.set_flag(Flag::CARRY);
// dx does NOT see this flag (different flags object)
```

### Example 9: Using Registers as Address Holders

```cpp
Register si;  // Source Index

// Store memory address
addr_t memory_address = 0x2000;
si.set_value(memory_address);

// Extract address for use
addr_t address = si.get_value();

// Use with offset
addr_t base = si.get_value();
addr_t offset = 10;
addr_t target = base + offset;  // 0x200A

// Increment address
si.inc();
// si = 0x2001
```

### Example 10: Byte-Level Data Packing

```cpp
Register ax;

// Pack two status bytes into one register
byte_t status1 = 0x42;
byte_t status2 = 0x88;

ax.set_high_byte(status1);
ax.set_low_byte(status2);
// ax = 0x4288

// Later, unpack
byte_t retrieved_status1 = ax.get_high_byte();  // 0x42
byte_t retrieved_status2 = ax.get_low_byte();   // 0x88

// Use for character pair storage
char char1 = 'A';
char char2 = 'B';
ax.set_high_byte(static_cast<byte_t>(char1));
ax.set_low_byte(static_cast<byte_t>(char2));
// ax contains "AB" in packed form
```

## Key Design Patterns

### Optional Flags Pattern

```cpp
// Register without flags (general purpose)
Register temp_reg;
temp_reg.set_value(100);
// No flag updates, lighter weight

// Register with flags (accumulator or status tracking)
auto flags = std::make_shared<Flags>();
Register accumulator(flags);
accumulator.inc();
// Flags automatically updated
```

### Shared Flags for CPU

```cpp
// Typical CPU setup
auto cpu_flags = std::make_shared<Flags>();

Register AX(cpu_flags);
Register BX(cpu_flags);
Register CX(cpu_flags);
Register DX(cpu_flags);
Register EX(cpu_flags);

// All registers share same flag state
// ALU operations on any register update same flags
// Conditional operations check same flags
```

### Byte Access for 8-bit Operations

```cpp
Register ax;

// Load 16-bit value
ax.set_value(0x1234);

// Operate on low byte only
byte_t low = ax.get_low_byte();  // 0x34
low += 10;                        // 0x3E
ax.set_low_byte(low);
// ax = 0x123E (high byte preserved)

// Operate on high byte only
byte_t high = ax.get_high_byte();  // 0x12
high -= 1;                          // 0x11
ax.set_high_byte(high);
// ax = 0x113E (low byte preserved)
```

## Increment/Decrement Behavior

The `inc()` and `dec()` methods automatically update flags if the register has them:

```cpp
auto flags = std::make_shared<Flags>();
Register ax(flags);

ax.set_value(0xFFFF);
ax.inc();
// ax = 0x0000
// Flags: ZERO = SET, CARRY = SET, SIGN = CLEAR

ax.set_value(0);
ax.dec();
// ax = 0xFFFF  
// Flags: ZERO = CLEAR, CARRY = SET, SIGN = SET

ax.set_value(0x7FFF);  // Max positive signed
ax.inc();
// ax = 0x8000
// Flags: OVERFLOW = SET, SIGN = SET
```

## Error Handling

The Register and Flags classes have minimal error handling:
- No bounds checking on `set_value()` - wraps naturally at 16-bit boundary
- No exceptions thrown for normal operations
- Flag operations on registers without flags are no-ops (return false)

## Performance Considerations

- **Value Access**: O(1) - Direct memory access
- **Byte Access**: O(1) - Bitwise operations
- **Flag Operations**: O(1) - Bitwise mask operations
- **Increment/Decrement**: O(1) - Simple arithmetic plus flag calculation

## Memory Layout

```cpp
sizeof(Register) = 20 bytes (typical):
  - word_t value         : 2 bytes
  - Flags& flags         : 8 bytes (reference)
  - bool flags_provided  : 1 byte
  - padding              : ~9 bytes

sizeof(Flags) = 1+ bytes:
  - byte_t flags         : 1 byte
  - padding/overhead     : varies
```

## Thread Safety

Neither Register nor Flags classes are thread-safe. If multiple threads need to access the same register or flags, external synchronization is required.

## Construction Patterns

```cpp
// No flags (lightweight)
Register general_purpose;

// With flags (for accumulator)
auto flags = std::make_shared<Flags>();
Register accumulator(flags);

// Copy (shares flags reference)
Register copy(accumulator);

// Create flags separately
Flags my_flags;
my_flags.set(Flag::ZERO);
```

## Pendragon VM Register Set

Standard Pendragon VM registers (5 general-purpose + special):

```cpp
auto cpu_flags = std::make_shared<Flags>();

Register AX(cpu_flags);  // Accumulator (primary for ALU)
Register BX(cpu_flags);  // Base
Register CX(cpu_flags);  // Counter  
Register DX(cpu_flags);  // Data
Register EX(cpu_flags);  // Extended

// Special registers (managed by other units)
// IR - Instruction Register (program counter) - in InstructionUnit
// SP - Stack Pointer - in Stack2
// FP - Frame Pointer - in Stack2
```
