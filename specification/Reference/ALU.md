# ALU - Arithmetic Logic Unit

## Purpose

The ALU performs all arithmetic and logical operations for the Pendragon VM, operating on the accumulator register and automatically setting flags based on operation results. It supports 16-bit word and 8-bit byte operations with overflow and carry detection.

## Interface Design

### IALU Interface

Alu implements the `IALU` pure virtual interface with all 26 operations:

```cpp
class IALU {
public:
    // Arithmetic operations (16-bit)
    virtual void add(word_t value) = 0;
    virtual void sub(word_t value) = 0;
    virtual void mul(word_t value) = 0;
    virtual void div(word_t value) = 0;
    virtual void rem(word_t value) = 0;
    
    // Arithmetic operations (8-bit)
    virtual void add_byte(byte_t value) = 0;
    virtual void sub_byte(byte_t value) = 0;
    virtual void mul_byte(byte_t value) = 0;
    virtual void div_byte(byte_t value) = 0;
    virtual void rem_byte(byte_t value) = 0;
    
    // Logical operations (16-bit)
    virtual void bit_and(word_t value) = 0;
    virtual void bit_or(word_t value) = 0;
    virtual void bit_xor(word_t value) = 0;
    virtual void bit_not() = 0;
    
    // Logical operations (8-bit)
    virtual void bit_and_byte(byte_t value) = 0;
    virtual void bit_or_byte(byte_t value) = 0;
    virtual void bit_xor_byte(byte_t value) = 0;
    
    // Shift and rotate
    virtual void shl(word_t count) = 0;
    virtual void shr(word_t count) = 0;
    virtual void rol(word_t count) = 0;
    virtual void ror(word_t count) = 0;
    
    // Comparison
    virtual void cmp(word_t value) = 0;
    virtual void cmp_byte(byte_t value) = 0;
    
    virtual ~IALU() = default;
};
```

**Key Points:**
- All 26 operation methods are pure virtual
- CPU owns the ALU (not injected) because ALU needs access to AX register
- CPU creates `std::unique_ptr<Alu>` internally during construction
- Unlike other subsystems, ALU is not injected because it's tightly coupled to the register file

## Architecture Overview

### Core Concepts

- **Accumulator-Based**: All operations modify the accumulator register (typically AX)
- **Automatic Flag Management**: Sets ZERO, SIGN, CARRY, and OVERFLOW flags
- **Dual Width**: Supports both 16-bit word and 8-bit byte operations
- **Bitwise Implementation**: Critical operations use bitwise logic for accuracy

### Operation Categories

1. **Arithmetic**: ADD, SUB, MUL, DIV, REM
2. **Logical**: AND, OR, XOR, NOT
3. **Shift/Rotate**: SHL, SHR, ROL, ROR
4. **Comparison**: CMP (sets flags without modifying accumulator)

### Flag Semantics

```
ZERO     (Z) : Set if result is 0
SIGN     (S) : Set if result's high bit is 1 (negative in signed arithmetic)
CARRY    (C) : Set on unsigned overflow (add) or borrow (subtract)
OVERFLOW (V) : Set on signed overflow (result doesn't fit in signed range)
```

## Interface

### Alu Class

```cpp
class Alu : public IALU {
public:
    // Construction (requires register with flags)
    Alu(std::shared_ptr<Register> acc);
    Alu(const Alu& other);
    ~Alu();
    
    // IALU interface implementation - Arithmetic operations (16-bit)
    void add(word_t value) override;
    void sub(word_t value) override;
    void mul(word_t value) override;
    void div(word_t value) override;
    void rem(word_t value) override;
    
    // Arithmetic operations (8-bit)
    void add_byte(byte_t value) override;
    void sub_byte(byte_t value) override;
    void mul_byte(byte_t value) override;
    void div_byte(byte_t value) override;
    void rem_byte(byte_t value) override;
    
    // Logical operations (16-bit)
    void bit_and(word_t value) override;
    void bit_or(word_t value) override;
    void bit_xor(word_t value) override;
    void bit_not() override;
    
    // Logical operations (8-bit)
    void bit_and_byte(byte_t value) override;
    void bit_or_byte(byte_t value) override;
    void bit_xor_byte(byte_t value) override;
    
    // Shift and rotate (16-bit only)
    void shl(word_t count) override;  // Shift left
    void shr(word_t count) override;  // Shift right
    void rol(word_t count) override;  // Rotate left
    void ror(word_t count) override;  // Rotate right
    
    // Comparison (sets flags, doesn't modify accumulator)
    void cmp(word_t value) override;
    void cmp_byte(byte_t value) override;
    
private:
    std::shared_ptr<Register> accumulator;
    void calculate_flags(word_t result, word_t a, word_t b, char operation);
};
```

## Usage Examples

### Example 1: Basic Arithmetic Operations

```cpp
#include "alu.h"
#include "register.h"

// Create flags and accumulator
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);

// Create ALU
Alu alu(ax);

// Addition
ax->set_value(100);
alu.add(50);
// ax = 150, flags updated

// Subtraction
ax->set_value(200);
alu.sub(75);
// ax = 125, flags updated

// Multiplication
ax->set_value(12);
alu.mul(5);
// ax = 60, flags updated

// Division
ax->set_value(100);
alu.div(4);
// ax = 25, flags updated

// Remainder
ax->set_value(17);
alu.rem(5);
// ax = 2, flags updated
```

### Example 2: Overflow and Carry Detection

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// Unsigned overflow (carry)
ax->set_value(0xFFFF);  // Maximum 16-bit value
alu.add(1);
// ax = 0x0000 (wraps)
// CARRY flag SET (unsigned overflow)
// ZERO flag SET (result is 0)

// Signed overflow
ax->set_value(0x7FFF);  // Maximum positive signed 16-bit
alu.add(1);
// ax = 0x8000
// OVERFLOW flag SET (signed overflow)
// SIGN flag SET (result appears negative)

// Borrow detection
ax->set_value(5);
alu.sub(10);
// ax = 0xFFFB (wraps to -5 in two's complement)
// CARRY flag SET (borrow occurred)
// SIGN flag SET (result is negative)
```

### Example 3: Logical Operations

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// AND operation (masking)
ax->set_value(0b1111000011110000);
alu.bit_and(0b0000111100001111);
// ax = 0b0000000000000000
// ZERO flag SET

// OR operation (combining bits)
ax->set_value(0b1100000000000000);
alu.bit_or(0b0011000000000000);
// ax = 0b1111000000000000
// SIGN flag SET (high bit is 1)

// XOR operation (toggling)
ax->set_value(0b1010101010101010);
alu.bit_xor(0b1111111111111111);
// ax = 0b0101010101010101

// NOT operation (inversion)
ax->set_value(0b1111000011110000);
alu.bit_not();
// ax = 0b0000111100001111
```

### Example 4: Byte-Sized Operations

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// Byte addition (operates on low byte only)
ax->set_value(0x1234);
alu.add_byte(0x10);
// ax = 0x1244 (only low byte modified)

// Byte with overflow
ax->set_value(0x12FF);
alu.add_byte(0x01);
// ax = 0x1200 (low byte wraps)
// CARRY flag SET

// Byte logical AND
ax->set_value(0x12F0);
alu.bit_and_byte(0x0F);
// ax = 0x1200 (low byte masked)

// Byte comparison
ax->set_value(0x1210);
alu.cmp_byte(0x10);
// ax = 0x1210 (unchanged)
// ZERO flag SET (low bytes equal)
```

### Example 5: Shift Operations

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// Shift left (multiply by 2^n)
ax->set_value(0b0000000000001010);  // 10
alu.shl(2);
// ax = 0b0000000000101000 (40)

// Shift right (divide by 2^n)
ax->set_value(0b0000000000101000);  // 40
alu.shr(2);
// ax = 0b0000000000001010 (10)

// Shift left with overflow
ax->set_value(0b1000000000000001);
alu.shl(1);
// ax = 0b0000000000000010
// Bits shifted off the left are lost

// Shift right (loses precision)
ax->set_value(15);
alu.shr(1);
// ax = 7 (15 / 2 = 7.5, truncated)
```

### Example 6: Rotate Operations

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// Rotate left (bits wrap around)
ax->set_value(0b1000000000000001);
alu.rol(1);
// ax = 0b0000000000000011 (high bit wraps to low)

// Rotate right
ax->set_value(0b1000000000000001);
alu.ror(1);
// ax = 0b1100000000000000 (low bit wraps to high)

// Multiple rotations
ax->set_value(0b0000000011111111);
alu.rol(4);
// ax = 0b0000111111110000 (rotated 4 positions left)

// Rotate by full width (no change)
ax->set_value(0x1234);
alu.rol(16);
// ax = 0x1234 (rotated full circle)
```

### Example 7: Comparison Without Modification

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
Alu alu(ax);

// Compare equal
ax->set_value(42);
alu.cmp(42);
// ax = 42 (UNCHANGED)
// ZERO flag SET (equal)
// CARRY flag CLEAR
// SIGN flag CLEAR

// Compare less than
ax->set_value(10);
alu.cmp(20);
// ax = 10 (UNCHANGED)
// CARRY flag SET (10 < 20 means borrow would occur)
// SIGN flag SET (result of 10 - 20 is negative)

// Compare greater than
ax->set_value(50);
alu.cmp(30);
// ax = 50 (UNCHANGED)
// CARRY flag CLEAR (no borrow)
// SIGN flag CLEAR (result positive)

// Use flags for conditional branching
if (flags->is_set(Flag::ZERO)) {
    // Equal
}
if (flags->is_set(Flag::CARRY)) {
    // Less than
}
if (!flags->is_set(Flag::CARRY) && !flags->is_set(Flag::ZERO)) {
    // Greater than
}
```

### Example 8: Complex Expression Evaluation

```cpp
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);
auto bx = std::make_shared<Register>(flags);
Alu alu(ax);

// Compute: result = (a + b) * 2 - 5
word_t a = 10;
word_t b = 15;

ax->set_value(a);
alu.add(b);              // ax = 25
alu.shl(1);              // ax = 50 (multiply by 2)
alu.sub(5);              // ax = 45

// Compute: result = (a & 0xFF) | (b << 8)
ax->set_value(a);
alu.bit_and(0x00FF);     // ax = 0x000A
bx->set_value(b);
// Would need to manually shift bx and then OR
// (Or use separate instructions in VM)

// Check if power of 2
ax->set_value(32);
word_t value = ax->get_value();
word_t check = value - 1;
alu.bit_and(check);
// ax = 0 if value was power of 2
// ZERO flag SET if power of 2
```

## Key Design Patterns

### Bitwise Addition Implementation

```cpp
void Alu::add(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = 0;
    word_t carry = 0;
    
    // Bit-by-bit addition with carry propagation
    for (int i = 0; i < 16; i++) {
        word_t bit_a = (a >> i) & 1;
        word_t bit_b = (b >> i) & 1;
        word_t sum_bit = bit_a ^ bit_b ^ carry;
        carry = (bit_a & bit_b) | (bit_a & carry) | (bit_b & carry);
        result |= (sum_bit << i);
    }
    
    accumulator->set_value(result);
    calculate_flags(result, a, b, '+');
}
```

### Flag Calculation

```cpp
void Alu::calculate_flags(word_t result, word_t a, word_t b, char operation) {
    // Clear all flags
    accumulator->clear_flag(Flag::ZERO);
    accumulator->clear_flag(Flag::CARRY);
    accumulator->clear_flag(Flag::SIGN);
    accumulator->clear_flag(Flag::OVERFLOW);
    
    // ZERO: Result is exactly 0
    if (result == 0) {
        accumulator->set_flag(Flag::ZERO);
    }
    
    // SIGN: High bit set (bit 15)
    if (result & 0x8000) {
        accumulator->set_flag(Flag::SIGN);
    }
    
    // Operation-specific flags
    switch (operation) {
        case '+':
            // CARRY: Unsigned overflow
            if (result < a || result < b) {
                accumulator->set_flag(Flag::CARRY);
            }
            // OVERFLOW: Signed overflow
            if (((a ^ result) & (b ^ result) & 0x8000) != 0) {
                accumulator->set_flag(Flag::OVERFLOW);
            }
            break;
            
        case '-':
            // CARRY: Borrow occurred
            if (a < b) {
                accumulator->set_flag(Flag::CARRY);
            }
            // OVERFLOW: Signed overflow
            if (((a ^ b) & (a ^ result) & 0x8000) != 0) {
                accumulator->set_flag(Flag::OVERFLOW);
            }
            break;
    }
}
```

## Error Handling

Common exceptions:
- `runtime_error("Accumulator register must have associated flags for ALU operations")` - Register lacks flags
- `runtime_error("Division by zero")` - Attempted division or remainder by zero
- `runtime_error("Shift count out of range")` - Shift/rotate count > 16

## Performance Considerations

- **Bitwise Addition**: O(16) - Iterates through 16 bits with carry
- **Simple Operations**: O(1) - Direct register manipulation
- **Multiplication/Division**: O(1) - Uses native CPU operations (implementation may vary)
- **Shift/Rotate**: O(n) where n is shift count
- **Flag Calculation**: O(1) - Constant-time checks

## Flag Decision Table

| Operation | ZERO | SIGN | CARRY | OVERFLOW |
|-----------|------|------|-------|----------|
| **ADD** | result == 0 | bit 15 set | unsigned overflow | signed overflow |
| **SUB** | result == 0 | bit 15 set | borrow (a < b) | signed overflow |
| **MUL** | result == 0 | bit 15 set | - | - |
| **DIV** | result == 0 | bit 15 set | - | - |
| **AND** | result == 0 | bit 15 set | - | - |
| **OR** | result == 0 | bit 15 set | - | - |
| **XOR** | result == 0 | bit 15 set | - | - |
| **NOT** | result == 0 | bit 15 set | - | - |
| **CMP** | a == b | (a-b) bit 15 | a < b | signed overflow |

## Register Requirements

The ALU requires its accumulator register to have associated flags:

```cpp
// Valid construction
auto flags = std::make_shared<Flags>();
auto ax = std::make_shared<Register>(flags);  // Has flags
Alu alu(ax);  // OK

// Invalid construction
auto bx = std::make_shared<Register>(nullptr);  // No flags
Alu alu(bx);  // THROWS: "must have associated flags"
```

## Byte Operations Behavior

Byte operations only affect the low 8 bits of the accumulator:

```cpp
ax->set_value(0x1234);
alu.add_byte(0x10);
// ax = 0x1244 (high byte 0x12 preserved)

ax->set_value(0x12FF);
alu.add_byte(0x01);
// ax = 0x1200 (low byte wraps, high byte preserved)
```

Flags are set based on the 8-bit result, not the full 16-bit register value.
