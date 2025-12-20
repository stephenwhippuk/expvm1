# Instruction Set Reference

## Overview

This document provides a complete reference for all 119 instructions supported by the Pendragon VM assembler. Each instruction is documented with its opcode, operands, behavior, flags affected, and usage examples.

## Instruction Format

Instructions consist of:
- **Opcode**: 1-byte operation code (0x00-0x7F)
- **Operands**: 0-2 operands, each can be:
  - **REG**: Register (1 byte encoded)
  - **VALUE**: Immediate value (1 or 2 bytes)
  - **ADDR**: Memory address (2 bytes)

## Register Encoding

| Register | Encoding |
|----------|----------|
| AX | 0x00 |
| BX | 0x01 |
| CX | 0x02 |
| DX | 0x03 |
| EX | 0x04 |

## Processor Flags

Instructions may affect these flags:
- **ZERO (Z)**: Set when result equals zero
- **CARRY (C)**: Set on arithmetic carry/borrow
- **SIGN (S)**: Set when result is negative (high bit = 1)
- **OVERFLOW (O)**: Set on signed arithmetic overflow

## Instruction Categories

1. [Control Flow](#control-flow)
2. [Data Movement](#data-movement)
3. [Stack Operations](#stack-operations)
4. [Memory Management](#memory-management)
5. [Arithmetic Operations](#arithmetic-operations)
6. [Logical Operations](#logical-operations)
7. [Bit Operations](#bit-operations)
8. [Comparison Operations](#comparison-operations)
9. [System Operations](#system-operations)

---

## Control Flow

### NOP - No Operation

**Opcode**: 0x00  
**Operands**: None  
**Flags**: None affected

Performs no operation. Execution continues with the next instruction.

**Usage**:
```assembly
CODE
    NOP             ; Does nothing
    LD AX, 10
```

**Use Cases**:
- Padding/alignment
- Placeholder for future code
- Timing adjustments

---

### HALT - Halt Execution

**Opcode**: 0x01  
**Operands**: None  
**Flags**: None affected

Stops program execution immediately.

**Usage**:
```assembly
CODE
    LD AX, 42
    HALT            ; Stop here
```

**Use Cases**:
- End of program
- Error termination
- Debug breakpoints

---

### JMP - Unconditional Jump

**Opcode**: 0x1E  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected

Sets the instruction pointer to the specified address.

**Syntax**: `JMP address`

**Usage**:
```assembly
CODE
start:
    LD AX, 10
    JMP end         ; Jump to 'end' label
    LD AX, 20       ; Skipped
end:
    HALT
```

**Use Cases**:
- Unconditional branching
- Loop continuation
- Function epilogue jumps

---

### JPZ / JZ - Jump if Zero

**Opcode**: 0x1F  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads ZERO flag)

Jumps to address if the ZERO flag is set.

**Syntax**: `JPZ address` or `JZ address`

**Usage**:
```assembly
CODE
    LD AX, 10
    SUB AX, 10      ; AX = 0, ZERO flag set
    JPZ is_zero     ; Jump taken
    HALT            ; Skipped
is_zero:
    LD AX, 1
    HALT
```

**Use Cases**:
- Equality testing
- Null pointer checks
- Loop termination

---

### JPNZ / JNZ - Jump if Not Zero

**Opcode**: 0x20  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads ZERO flag)

Jumps to address if the ZERO flag is NOT set.

**Syntax**: `JPNZ address` or `JNZ address`

**Usage**:
```assembly
CODE
    LD CX, 10
loop:
    ; ... loop body ...
    DEC CX          ; Decrement counter
    JPNZ loop       ; Continue if CX != 0
    HALT
```

**Use Cases**:
- Loop continuation
- Non-equality testing
- Non-null checks

---

### JPC - Jump if Carry

**Opcode**: 0x21  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads CARRY flag)

Jumps to address if the CARRY flag is set.

**Syntax**: `JPC address`

**Usage**:
```assembly
CODE
    LD AL, 255
    ADD AL, 1       ; Overflow sets CARRY
    JPC overflow    ; Jump taken
    HALT
overflow:
    ; Handle overflow
    HALT
```

---

### JPNC - Jump if Not Carry

**Opcode**: 0x22  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads CARRY flag)

Jumps to address if the CARRY flag is NOT set.

**Syntax**: `JPNC address`

---

### JPS - Jump if Sign

**Opcode**: 0x23  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads SIGN flag)

Jumps to address if the SIGN flag is set (result was negative).

**Syntax**: `JPS address`

**Usage**:
```assembly
CODE
    LD AX, 10
    SUB AX, 20      ; Result negative, SIGN set
    JPS negative    ; Jump taken
    HALT
negative:
    ; Handle negative result
    HALT
```

---

### JPNS - Jump if Not Sign

**Opcode**: 0x24  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads SIGN flag)

Jumps to address if the SIGN flag is NOT set (result was positive).

**Syntax**: `JPNS address`

---

### JPO - Jump if Overflow

**Opcode**: 0x25  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads OVERFLOW flag)

Jumps to address if the OVERFLOW flag is set.

**Syntax**: `JPO address`

---

### JPNO - Jump if Not Overflow

**Opcode**: 0x26  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected (reads OVERFLOW flag)

Jumps to address if the OVERFLOW flag is NOT set.

**Syntax**: `JPNO address`

---

### CALL - Call Subroutine

**Opcode**: 0x27  
**Operands**: ADDR (2 bytes), FLAG (1 bit)  
**Flags**: None affected

Pushes current instruction pointer and stack frame pointer to return stack, sets new stack frame, and jumps to subroutine address.

**Syntax**: `CALL address`

**Usage**:
```assembly
CODE
main:
    LD AX, 10
    CALL multiply_by_two
    HALT

multiply_by_two:
    MUL AX, 2
    RET
```

**Use Cases**:
- Function calls
- Subroutine invocation
- Code reuse

---

### RET - Return from Subroutine

**Opcode**: 0x28  
**Operands**: None  
**Flags**: None affected

Flushes stack, pops instruction pointer and stack frame pointer from return stack, and continues execution after the CALL.

**Syntax**: `RET`

**Usage**:
```assembly
CODE
function:
    LD AX, 42
    RET             ; Return to caller
```

---

## Data Movement

### LD - Load Value into Register

**Variant 1 - Immediate**: 0x02  
**Variant 2 - Register**: 0x03  
**Operands**: REG, VALUE (2 bytes) or REG, REG  
**Flags**: None affected

Loads a value or register contents into a 16-bit register.

**Syntax**: 
- `LD dest_reg, immediate`
- `LD dest_reg, source_reg`

**Usage**:
```assembly
CODE
    LD AX, 1000         ; Load immediate into AX
    LD BX, AX           ; Copy AX to BX
    LD CX, 0xFF00       ; Load hex value into CX
```

**Use Cases**:
- Initialize registers
- Copy values between registers
- Load constants

---

### SWP - Swap Registers

**Opcode**: 0x04  
**Operands**: REG, REG  
**Flags**: None affected

Swaps the contents of two registers.

**Syntax**: `SWP reg1, reg2`

**Usage**:
```assembly
CODE
    LD AX, 100
    LD BX, 200
    SWP AX, BX          ; AX=200, BX=100
```

---

### LDH - Load High Byte

**Variant 1 - Immediate**: 0x05  
**Variant 2 - Register**: 0x06  
**Operands**: REG, VALUE (1 byte) or REG, REG  
**Flags**: None affected

Loads a byte value into the high byte of a register (bits 15-8).

**Syntax**:
- `LDH dest_reg, immediate`
- `LDH dest_reg, source_reg`

**Usage**:
```assembly
CODE
    LD AX, 0x0000
    LDH AH, 0x12        ; AX becomes 0x1200
    LDH BH, AH          ; Copy AH to BH
```

**Use Cases**:
- Byte-level manipulation
- Packing two bytes into a word
- High-byte initialization

---

### LDL - Load Low Byte

**Variant 1 - Immediate**: 0x07  
**Variant 2 - Register**: 0x08  
**Operands**: REG, VALUE (1 byte) or REG, REG  
**Flags**: None affected

Loads a byte value into the low byte of a register (bits 7-0).

**Syntax**:
- `LDL dest_reg, immediate`
- `LDL dest_reg, source_reg`

**Usage**:
```assembly
CODE
    LD AX, 0x0000
    LDL AL, 0x34        ; AX becomes 0x0034
    LDL BL, AL          ; Copy AL to BL
```

---

### LDA - Load from Address / Load Address

**Variant 1 - Load from Address**: 0x09  
**Variant 2 - Store to Address**: 0x0D  
**Variant 3 - Indirect Load**: 0x72  
**Operands**: Various  
**Flags**: None affected

Multiple variants for loading/storing words from/to memory.

**Syntax**:
- `LD reg, [address]` - Load word from memory address
- `LD [address], reg` - Store word to memory address
- `LD reg, (address_reg)` - Load word from address in register

**Usage**:
```assembly
DATA
    value: DW [0x1234]

CODE
    LD AX, [value]      ; Load word from 'value'
    LD BX, 0x5678
    LD [value], BX      ; Store BX to 'value'
    
    LDA CX, value       ; Load address of 'value' into CX
    LD DX, (CX)         ; Load word from address in CX
```

---

### LDAB - Load Byte from Address

**Opcode**: 0x0A  
**Operands**: REG, ADDR (2 bytes)  
**Flags**: None affected

Loads a single byte from memory into a register (full 16-bit register affected).

**Syntax**: `LDAB reg, [address]`

**Usage**:
```assembly
DATA
    byte_val: DB [0x42]

CODE
    LDAB AX, [byte_val] ; AX = 0x0042 (byte zero-extended)
```

---

### LDAH - Load High Byte from Address

**Variant 1 - Load**: 0x0B  
**Variant 2 - Store**: 0x0E  
**Variant 3 - Indirect**: 0x73  
**Operands**: Various  
**Flags**: None affected

Loads or stores the high byte of a register from/to memory.

**Syntax**:
- `LD reg, [address]` (with high byte variant)
- `LD [address], reg` (store high byte)

---

### LDAL - Load Low Byte from Address

**Variant 1 - Load**: 0x0C  
**Variant 2 - Store**: 0x0F  
**Variant 3 - Indirect**: 0x74  
**Operands**: Various  
**Flags**: None affected

Loads or stores the low byte of a register from/to memory.

**Syntax**:
- `LD reg, [address]` (with low byte variant)
- `LD [address], reg` (store low byte)

---

## Stack Operations

### PUSH - Push Register to Stack

**Opcode**: 0x10  
**Operands**: REG  
**Flags**: None affected

Pushes a 16-bit register value onto the stack.

**Syntax**: `PUSH reg`

**Usage**:
```assembly
CODE
    LD AX, 100
    PUSH AX             ; Stack: [100]
    LD AX, 200
    PUSH AX             ; Stack: [100, 200]
```

**Use Cases**:
- Save register values
- Function parameters
- Temporary storage

---

### PUSHH - Push High Byte to Stack

**Opcode**: 0x11  
**Operands**: REG  
**Flags**: None affected

Pushes the high byte of a register onto the stack.

**Syntax**: `PUSHH reg`

---

### PUSHL - Push Low Byte to Stack

**Opcode**: 0x12  
**Operands**: REG  
**Flags**: None affected

Pushes the low byte of a register onto the stack.

**Syntax**: `PUSHL reg`

---

### POP - Pop from Stack to Register

**Opcode**: 0x13  
**Operands**: REG  
**Flags**: None affected

Pops a 16-bit value from the stack into a register.

**Syntax**: `POP reg`

**Usage**:
```assembly
CODE
    PUSH AX             ; Save AX
    LD AX, 50           ; Use AX for something else
    POP AX              ; Restore original AX
```

**Note**: Cannot pop past stack frame (SF).

---

### POPH - Pop High Byte from Stack

**Opcode**: 0x14  
**Operands**: REG  
**Flags**: None affected

Pops a byte from the stack into the high byte of a register.

**Syntax**: `POPH reg`

---

### POPL - Pop Low Byte from Stack

**Opcode**: 0x15  
**Operands**: REG  
**Flags**: None affected

Pops a byte from the stack into the low byte of a register.

**Syntax**: `POPL reg`

---

### PUSHW - Push Immediate Word

**Opcode**: 0x75  
**Operands**: VALUE (2 bytes)  
**Flags**: None affected

Pushes an immediate 16-bit value onto the stack.

**Syntax**: `PUSHW immediate`

**Usage**:
```assembly
CODE
    PUSHW 0x1234        ; Push constant
    PUSHW 42            ; Push decimal value
```

---

### PUSHB - Push Immediate Byte

**Opcode**: 0x76  
**Operands**: VALUE (1 byte)  
**Flags**: None affected

Pushes an immediate byte value onto the stack.

**Syntax**: `PUSHB immediate`

**Usage**:
```assembly
CODE
    PUSHB 0x42          ; Push byte constant
    PUSHB 10            ; Push decimal byte
```

---

### PEEK - Peek at Stack

**Opcode**: 0x16  
**Operands**: REG, OFFSET (2 bytes)  
**Flags**: None affected

Loads a word from the stack at (Stack Base - OFFSET) without popping.

**Syntax**: `PEEK reg, offset`

**Usage**:
```assembly
CODE
    PUSHW 100
    PUSHW 200
    PEEK AX, 0          ; AX = 200 (top of stack)
    PEEK BX, 2          ; BX = 100 (previous value)
```

---

### PEEKF - Peek at Stack Frame

**Opcode**: 0x17  
**Operands**: REG, OFFSET (2 bytes)  
**Flags**: None affected

Loads a word from the stack at (Stack Frame - OFFSET) without popping.

**Syntax**: `PEEKF reg, offset`

---

### PEEKB - Peek Byte at Stack

**Opcode**: 0x18  
**Operands**: REG, OFFSET (2 bytes)  
**Flags**: None affected

Loads a byte from the stack at (Stack Base - OFFSET) without popping.

**Syntax**: `PEEKB reg, offset`

---

### PEEKFB - Peek Byte at Stack Frame

**Opcode**: 0x19  
**Operands**: REG, OFFSET (2 bytes)  
**Flags**: None affected

Loads a byte from the stack at (Stack Frame - OFFSET) without popping.

**Syntax**: `PEEKFB reg, offset`

---

### FLSH - Flush Stack

**Opcode**: 0x1A  
**Operands**: None  
**Flags**: None affected

Clears the stack back to the current stack frame pointer.

**Syntax**: `FLSH`

**Usage**:
```assembly
CODE
    PUSHW 10
    PUSHW 20
    PUSHW 30
    FLSH                ; All three values removed
```

---

## Memory Management

Memory management instructions control paging and stack frame operations.

### PAGE - Set Memory Page

**Variant 1 - Immediate**: 0x1B  
**Variant 2 - Register**: 0x1C  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: None affected

Sets the current memory page for paged memory architectures. The PAGE instruction switches the active memory page, allowing access to data on different pages.

**Syntax**:
- `PAGE immediate` - Set page to literal value
- `PAGE reg` - Set page to value in register

**Binary Format**:
- Immediate variant: `[0x1B] [page_low] [page_high] [context_low] [context_high]` (5 bytes)
- Register variant: `[0x1C] [reg] [context_low] [context_high]` (4 bytes)

**Usage**:
```assembly
DATA
    PAGE graphics_data
    sprite: DB [0x01, 0x02, 0x03]
    
    PAGE sound_data
    sample: DW [1000, 2000, 3000]

CODE
    ; Manual page switching (usually unnecessary - assembler auto-injects)
    PAGE 1              ; Switch to page 1
    LDA AX, sprite      ; Access data on page 1
    
    PAGE 2              ; Switch to page 2  
    LDA BX, sample      ; Access data on page 2
    
    ; Using register for dynamic page selection
    LD CX, 1
    PAGE CX             ; Switch to page in CX register
```

**Automatic Injection**:

The assembler automatically injects PAGE instructions when necessary:

```assembly
DATA
    PAGE page1
    var1: DW [100]
    
    PAGE page2
    var2: DW [200]

CODE
    ; Assembler automatically injects: PAGE 1
    LDA AX, var1
    
    ; Assembler automatically injects: PAGE 2
    LDA BX, var2
    
    ; No PAGE instruction injected (already on page 2)
    LDA CX, var2
```

**Notes**:
- Page 0 is the default page for data without a `PAGE` directive
- Pages are 16-bit values (0-65535)
- Context parameter is typically 0 (reserved for future use)
- Switching pages does not affect register contents
- Manual PAGE instructions can override automatic injection if needed

---

### SETF - Set Stack Frame

**Opcode**: 0x1D  
**Operands**: ADDR (2 bytes)  
**Flags**: None affected

Sets the stack frame pointer to the specified address.

**Syntax**: `SETF address`

**Usage**:
```assembly
CODE
    SETF 0x1000         ; Set stack frame to 0x1000
```

---

## Arithmetic Operations

All arithmetic operations store results in AX and update flags.

### ADD - Add to AX

**Variant 1 - Immediate Word**: 0x29  
**Variant 2 - Register**: 0x2A  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S, O

Adds a word value or register to AX.

**Syntax**:
- `ADD AX, immediate`
- `ADD AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 100
    ADD AX, 50          ; AX = 150
    ADD AX, BX          ; AX = AX + BX
```

**Flags**:
- **ZERO**: Set if result is 0
- **CARRY**: Set if unsigned overflow
- **SIGN**: Set if result is negative (bit 15 = 1)
- **OVERFLOW**: Set if signed overflow

---

### ADB - Add Byte to AX

**Opcode**: 0x2B  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S, O

Adds a byte value to AX (byte is zero-extended to 16 bits).

**Syntax**: `ADB AX, immediate`

---

### ADH - Add High Bytes

**Opcode**: 0x2C  
**Operands**: REG  
**Flags**: Z, C, S, O

Adds the high byte of a register to the high byte of AX.

**Syntax**: `ADH AH, reg`

---

### ADL - Add Low Bytes

**Opcode**: 0x2D  
**Operands**: REG  
**Flags**: Z, C, S, O

Adds the low byte of a register to the low byte of AX.

**Syntax**: `ADL AL, reg`

---

### SUB - Subtract from AX

**Variant 1 - Immediate Word**: 0x2E  
**Variant 2 - Register**: 0x2F  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S, O

Subtracts a word value or register from AX.

**Syntax**:
- `SUB AX, immediate`
- `SUB AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 100
    SUB AX, 30          ; AX = 70
    SUB AX, BX          ; AX = AX - BX
```

---

### SBB - Subtract Byte from AX

**Opcode**: 0x30  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S, O

Subtracts a byte value from AX.

**Syntax**: `SBB AX, immediate`

---

### SBH - Subtract High Bytes

**Opcode**: 0x31  
**Operands**: REG  
**Flags**: Z, C, S, O

Subtracts the high byte of a register from the high byte of AX.

**Syntax**: `SBH AH, reg`

---

### SBL - Subtract Low Bytes

**Opcode**: 0x32  
**Operands**: REG  
**Flags**: Z, C, S, O

Subtracts the low byte of a register from the low byte of AX.

**Syntax**: `SBL AL, reg`

---

### MUL - Multiply AX

**Variant 1 - Immediate Word**: 0x33  
**Variant 2 - Register**: 0x34  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S, O

Multiplies AX by a word value or register.

**Syntax**:
- `MUL AX, immediate`
- `MUL AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 10
    MUL AX, 5           ; AX = 50
    MUL AX, BX          ; AX = AX * BX
```

**Note**: Result overflow wraps (only lower 16 bits retained).

---

### MLB - Multiply Byte by AX

**Opcode**: 0x35  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S, O

Multiplies AX by a byte value.

**Syntax**: `MLB AX, immediate`

---

### MLH - Multiply High Bytes

**Opcode**: 0x36  
**Operands**: REG  
**Flags**: Z, C, S, O

Multiplies the high byte of AX by the high byte of a register.

**Syntax**: `MLH AH, reg`

---

### MLL - Multiply Low Bytes

**Opcode**: 0x37  
**Operands**: REG  
**Flags**: Z, C, S, O

Multiplies the low byte of AX by the low byte of a register.

**Syntax**: `MLL AL, reg`

---

### DIV - Divide AX

**Variant 1 - Immediate Word**: 0x38  
**Variant 2 - Register**: 0x39  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S, O

Divides AX by a word value or register (integer division).

**Syntax**:
- `DIV AX, immediate`
- `DIV AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 100
    DIV AX, 3           ; AX = 33 (integer division)
    DIV AX, BX          ; AX = AX / BX
```

**Note**: Division by zero behavior is implementation-defined.

---

### DVB - Divide AX by Byte

**Opcode**: 0x3A  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S, O

Divides AX by a byte value.

**Syntax**: `DVB AX, immediate`

---

### DVH - Divide High Bytes

**Opcode**: 0x3B  
**Operands**: REG  
**Flags**: Z, C, S, O

Divides the high byte of AX by the high byte of a register.

**Syntax**: `DVH AH, reg`

---

### DVL - Divide Low Bytes

**Opcode**: 0x3C  
**Operands**: REG  
**Flags**: Z, C, S, O

Divides the low byte of AX by the low byte of a register.

**Syntax**: `DVL AL, reg`

---

### REM - Remainder (Modulo)

**Variant 1 - Immediate Word**: 0x3D  
**Variant 2 - Register**: 0x3E  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S, O

Computes the remainder of AX divided by a word value or register.

**Syntax**:
- `REM AX, immediate`
- `REM AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 100
    REM AX, 7           ; AX = 2 (100 % 7)
```

---

### RMB - Remainder Byte

**Opcode**: 0x3F  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S, O

Computes the remainder of AX divided by a byte value.

**Syntax**: `RMB AX, immediate`

---

### RMH - Remainder High Bytes

**Opcode**: 0x40  
**Operands**: REG  
**Flags**: Z, C, S, O

Computes the remainder of AH divided by the high byte of a register.

**Syntax**: `RMH AH, reg`

---

### RML - Remainder Low Bytes

**Opcode**: 0x41  
**Operands**: REG  
**Flags**: Z, C, S, O

Computes the remainder of AL divided by the low byte of a register.

**Syntax**: `RML AL, reg`

---

## Logical Operations

All logical operations store results in AX and update flags.

### AND - Logical AND

**Variant 1 - Immediate Word**: 0x42  
**Variant 2 - Register**: 0x43  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, S (C and O cleared)

Performs bitwise AND between AX and a word value or register.

**Syntax**:
- `AND AX, immediate`
- `AND AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0xFF0F
    AND AX, 0x0FFF      ; AX = 0x0F0F
    AND AX, BX          ; AX = AX & BX
```

**Use Cases**:
- Bit masking
- Clearing specific bits
- Testing bit patterns

---

### ANB - AND Byte with AX

**Opcode**: 0x44  
**Operands**: VALUE (1 byte)  
**Flags**: Z, S

Performs bitwise AND between the low byte of AX and a byte value.

**Syntax**: `ANB AL, immediate`

---

### ANH - AND High Bytes

**Opcode**: 0x45  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise AND between the high bytes of AX and a register.

**Syntax**: `ANH AH, reg`

---

### ANL - AND Low Bytes

**Opcode**: 0x46  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise AND between the low bytes of AX and a register.

**Syntax**: `ANL AL, reg`

---

### OR - Logical OR

**Variant 1 - Immediate Word**: 0x47  
**Variant 2 - Register**: 0x48  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, S (C and O cleared)

Performs bitwise OR between AX and a word value or register.

**Syntax**:
- `OR AX, immediate`
- `OR AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0xFF00
    OR AX, 0x00FF       ; AX = 0xFFFF
    OR AX, BX           ; AX = AX | BX
```

**Use Cases**:
- Setting specific bits
- Combining bit flags
- Merging values

---

### ORB - OR Byte with AX

**Opcode**: 0x49  
**Operands**: VALUE (1 byte)  
**Flags**: Z, S

Performs bitwise OR between the low byte of AX and a byte value.

**Syntax**: `ORB AL, immediate`

---

### ORH - OR High Bytes

**Opcode**: 0x4A  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise OR between the high bytes of AX and a register.

**Syntax**: `ORH AH, reg`

---

### ORL - OR Low Bytes

**Opcode**: 0x4B  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise OR between the low bytes of AX and a register.

**Syntax**: `ORL AL, reg`

---

### XOR - Logical XOR

**Variant 1 - Immediate Word**: 0x4C  
**Variant 2 - Register**: 0x4D  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, S (C and O cleared)

Performs bitwise XOR between AX and a word value or register.

**Syntax**:
- `XOR AX, immediate`
- `XOR AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0xFFFF
    XOR AX, 0xFFFF      ; AX = 0x0000 (quick clear)
    XOR AX, BX          ; AX = AX ^ BX
```

**Use Cases**:
- Toggling bits
- Quick register clear (XOR reg, reg)
- Encryption/checksums

---

### XOB - XOR Byte with AX

**Opcode**: 0x4E  
**Operands**: VALUE (1 byte)  
**Flags**: Z, S

Performs bitwise XOR between the low byte of AX and a byte value.

**Syntax**: `XOB AL, immediate`

---

### XOH - XOR High Bytes

**Opcode**: 0x4F  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise XOR between the high bytes of AX and a register.

**Syntax**: `XOH AH, reg`

---

### XOL - XOR Low Bytes

**Opcode**: 0x50  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise XOR between the low bytes of AX and a register.

**Syntax**: `XOL AL, reg`

---

### NOT - Logical NOT

**Variant 1 - Immediate Word**: 0x51  
**Variant 2 - Register**: 0x52  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, S (C and O cleared)

Performs bitwise NOT (one's complement) on a value or register, stores in AX.

**Syntax**:
- `NOT immediate` (result in AX)
- `NOT reg` (result in AX)

**Usage**:
```assembly
CODE
    LD AX, 0x00FF
    NOT AX              ; AX = 0xFF00
    NOT BX              ; AX = ~BX
```

---

### NOTB - NOT Byte

**Opcode**: 0x53  
**Operands**: VALUE (1 byte)  
**Flags**: Z, S

Performs bitwise NOT on a byte value, stores in AX.

**Syntax**: `NOTB immediate`

---

### NOTH - NOT High Byte

**Opcode**: 0x54  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise NOT on the high byte of a register, stores in AX.

**Syntax**: `NOTH reg`

---

### NOTL - NOT Low Byte

**Opcode**: 0x55  
**Operands**: REG  
**Flags**: Z, S

Performs bitwise NOT on the low byte of a register, stores in AX.

**Syntax**: `NOTL reg`

---

## Bit Operations

### SHL - Shift Left

**Variant 1 - Immediate Word**: 0x56  
**Variant 2 - Register**: 0x57  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S

Shifts AX left by the specified number of bit positions.

**Syntax**:
- `SHL AX, immediate`
- `SHL AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0x0001
    SHL AX, 4           ; AX = 0x0010 (multiply by 16)
```

**Note**: Bits shifted out are lost, zeros shifted in from right.

---

### SLB - Shift Left Byte

**Opcode**: 0x58  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S

Shifts AX left by a byte value.

**Syntax**: `SLB AX, immediate`

---

### SLH - Shift Left High Byte

**Opcode**: 0x59  
**Operands**: REG  
**Flags**: Z, C, S

Shifts the high byte of AX left.

**Syntax**: `SLH AH, reg`

---

### SLL - Shift Left Low Byte

**Opcode**: 0x5A  
**Operands**: REG  
**Flags**: Z, C, S

Shifts the low byte of AX left.

**Syntax**: `SLL AL, reg`

---

### SHR - Shift Right

**Variant 1 - Immediate Word**: 0x5B  
**Variant 2 - Register**: 0x5C  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: Z, C, S

Shifts AX right by the specified number of bit positions (logical shift).

**Syntax**:
- `SHR AX, immediate`
- `SHR AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0x0100
    SHR AX, 4           ; AX = 0x0010 (divide by 16)
```

**Note**: Zeros shifted in from left.

---

### SHRB - Shift Right Byte

**Opcode**: 0x5D  
**Operands**: VALUE (1 byte)  
**Flags**: Z, C, S

Shifts AX right by a byte value.

**Syntax**: `SHRB AX, immediate`

---

### SHRH - Shift Right High Byte

**Opcode**: 0x5E  
**Operands**: REG  
**Flags**: Z, C, S

Shifts the high byte of AX right.

**Syntax**: `SHRH AH, reg`

---

### SHRL - Shift Right Low Byte

**Opcode**: 0x5F  
**Operands**: REG  
**Flags**: Z, C, S

Shifts the low byte of AX right.

**Syntax**: `SHRL AL, reg`

---

### ROL - Rotate Left

**Variant 1 - Immediate Word**: 0x60  
**Variant 2 - Register**: 0x61  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: C

Rotates AX left by the specified number of bit positions.

**Syntax**:
- `ROL AX, immediate`
- `ROL AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0x8001
    ROL AX, 1           ; AX = 0x0003 (bit 15 wraps to bit 0)
```

**Note**: Bits rotated out from left wrap to right.

---

### ROLB - Rotate Left Byte

**Opcode**: 0x62  
**Operands**: VALUE (1 byte)  
**Flags**: C

Rotates AX left by a byte value.

**Syntax**: `ROLB AX, immediate`

---

### ROLH - Rotate Left High Byte

**Opcode**: 0x63  
**Operands**: REG  
**Flags**: C

Rotates the high byte of AX left.

**Syntax**: `ROLH AH, reg`

---

### ROLL - Rotate Left Low Byte

**Opcode**: 0x64  
**Operands**: REG  
**Flags**: C

Rotates the low byte of AX left.

**Syntax**: `ROLL AL, reg`

---

### ROR - Rotate Right

**Variant 1 - Immediate Word**: 0x65  
**Variant 2 - Register**: 0x66  
**Operands**: VALUE (2 bytes) or REG  
**Flags**: C

Rotates AX right by the specified number of bit positions.

**Syntax**:
- `ROR AX, immediate`
- `ROR AX, reg`

**Usage**:
```assembly
CODE
    LD AX, 0x0001
    ROR AX, 1           ; AX = 0x8000 (bit 0 wraps to bit 15)
```

**Note**: Bits rotated out from right wrap to left.

---

### RORB - Rotate Right Byte

**Opcode**: 0x67  
**Operands**: VALUE (1 byte)  
**Flags**: C

Rotates AX right by a byte value.

**Syntax**: `RORB AX, immediate`

---

### RORH - Rotate Right High Byte

**Opcode**: 0x68  
**Operands**: REG  
**Flags**: C

Rotates the high byte of AX right.

**Syntax**: `RORH AH, reg`

---

### RORL - Rotate Right Low Byte

**Opcode**: 0x69  
**Operands**: REG  
**Flags**: C

Rotates the low byte of AX right.

**Syntax**: `RORL AL, reg`

---

## Comparison Operations

### INC - Increment Register

**Opcode**: 0x6A  
**Operands**: REG  
**Flags**: Z, C, S, O (only if register is AX)

Increments a register by 1.

**Syntax**: `INC reg`

**Usage**:
```assembly
CODE
    LD CX, 0
    INC CX              ; CX = 1
    INC CX              ; CX = 2
```

**Use Cases**:
- Loop counter increment
- Pointer advancement
- Simple arithmetic

---

### DEC - Decrement Register

**Opcode**: 0x6B  
**Operands**: REG  
**Flags**: Z, C, S, O (only if register is AX)

Decrements a register by 1.

**Syntax**: `DEC reg`

**Usage**:
```assembly
CODE
    LD CX, 10
loop:
    ; ... loop body ...
    DEC CX              ; CX = CX - 1
    JPNZ loop           ; Continue if CX != 0
```

---

### CMP - Compare Registers/Values

**Variant 1 - Register vs Register**: 0x6C  
**Variant 2 - Register vs Immediate**: 0x6D  
**Operands**: REG, REG or REG, VALUE (2 bytes)  
**Flags**: All flags

Compares two 16-bit values and sets AX based on result:
- AX = -1 (0xFFFF) if first < second
- AX = 0 if first == second  
- AX = 1 if first > second

**Syntax**:
- `CMP reg1, reg2`
- `CMP reg, immediate`

**Usage**:
```assembly
CODE
    LD AX, 10
    LD BX, 20
    CMP AX, BX          ; AX = -1 (10 < 20)
    
    LD CX, 50
    CMP CX, 50          ; AX = 0 (50 == 50)
    
    LD DX, 100
    CMP DX, 50          ; AX = 1 (100 > 50)
```

**Use Cases**:
- Conditional branching
- Sorting algorithms
- Range checking

---

### CPH - Compare High Bytes

**Variant 1 - Register vs Register**: 0x6E  
**Variant 2 - Register vs Immediate**: 0x6F  
**Operands**: REG, REG or REG, VALUE (1 byte)  
**Flags**: All flags

Compares the high bytes of registers and sets AX based on result.

**Syntax**:
- `CPH reg1, reg2`
- `CPH reg, immediate`

---

### CPL - Compare Low Bytes

**Variant 1 - Register vs Register**: 0x70  
**Variant 2 - Register vs Immediate**: 0x71  
**Operands**: REG, REG or REG, VALUE (1 byte)  
**Flags**: All flags

Compares the low bytes of registers and sets AX based on result.

**Syntax**:
- `CPL reg1, reg2`
- `CPL reg, immediate`

---

## System Operations

### SYS / SYSCALL - System Call

**Opcode**: 0x7F  
**Operands**: FUNC (2 bytes)  
**Flags**: Implementation-defined

Invokes a system function. Function behavior depends on implementation.

**Syntax**: `SYS function_code` or `SYSCALL function_code`

**Usage**:
```assembly
CODE
    LD AX, 1            ; Function parameter
    SYS 0x0001          ; Call system function 1
```

**Note**: Available system functions are implementation-specific.

---

## Complete Instruction Summary Table

| Opcode | Mnemonic | Category | Operands | Flags |
|--------|----------|----------|----------|-------|
| 0x00 | NOP | Control | - | - |
| 0x01 | HALT | Control | - | - |
| 0x02 | LD | Data | REG, VALUE | - |
| 0x03 | LD | Data | REG, REG | - |
| 0x04 | SWP | Data | REG, REG | - |
| 0x05 | LDH | Data | REG, VALUE | - |
| 0x06 | LDH | Data | REG, REG | - |
| 0x07 | LDL | Data | REG, VALUE | - |
| 0x08 | LDL | Data | REG, REG | - |
| 0x09 | LDA | Data | REG, ADDR | - |
| 0x0A | LDAB | Data | REG, ADDR | - |
| 0x0B | LDAH | Data | REG, ADDR | - |
| 0x0C | LDAL | Data | REG, ADDR | - |
| 0x0D | LDA | Data | ADDR, REG | - |
| 0x0E | LDAH | Data | ADDR, REG | - |
| 0x0F | LDAL | Data | ADDR, REG | - |
| 0x10 | PUSH | Stack | REG | - |
| 0x11 | PUSHH | Stack | REG | - |
| 0x12 | PUSHL | Stack | REG | - |
| 0x13 | POP | Stack | REG | - |
| 0x14 | POPH | Stack | REG | - |
| 0x15 | POPL | Stack | REG | - |
| 0x16 | PEEK | Stack | REG, OFFSET | - |
| 0x17 | PEEKF | Stack | REG, OFFSET | - |
| 0x18 | PEEKB | Stack | REG, OFFSET | - |
| 0x19 | PEEKFB | Stack | REG, OFFSET | - |
| 0x1A | FLSH | Stack | - | - |
| 0x1B | PAGE | Memory | VALUE | - |
| 0x1C | PAGE | Memory | REG | - |
| 0x1D | SETF | Memory | ADDR | - |
| 0x1E | JMP | Control | ADDR | - |
| 0x1F | JPZ/JZ | Control | ADDR | - |
| 0x20 | JPNZ/JNZ | Control | ADDR | - |
| 0x21 | JPC | Control | ADDR | - |
| 0x22 | JPNC | Control | ADDR | - |
| 0x23 | JPS | Control | ADDR | - |
| 0x24 | JPNS | Control | ADDR | - |
| 0x25 | JPO | Control | ADDR | - |
| 0x26 | JPNO | Control | ADDR | - |
| 0x27 | CALL | Control | ADDR | - |
| 0x28 | RET | Control | - | - |
| 0x29-0x2D | ADD variants | Arithmetic | Various | Z,C,S,O |
| 0x2E-0x32 | SUB variants | Arithmetic | Various | Z,C,S,O |
| 0x33-0x37 | MUL variants | Arithmetic | Various | Z,C,S,O |
| 0x38-0x3C | DIV variants | Arithmetic | Various | Z,C,S,O |
| 0x3D-0x41 | REM variants | Arithmetic | Various | Z,C,S,O |
| 0x42-0x46 | AND variants | Logical | Various | Z,S |
| 0x47-0x4B | OR variants | Logical | Various | Z,S |
| 0x4C-0x50 | XOR variants | Logical | Various | Z,S |
| 0x51-0x55 | NOT variants | Logical | Various | Z,S |
| 0x56-0x5A | SHL variants | Bit | Various | Z,C,S |
| 0x5B-0x5F | SHR variants | Bit | Various | Z,C,S |
| 0x60-0x64 | ROL variants | Bit | Various | C |
| 0x65-0x69 | ROR variants | Bit | Various | C |
| 0x6A | INC | Comparison | REG | Z,C,S,O* |
| 0x6B | DEC | Comparison | REG | Z,C,S,O* |
| 0x6C-0x71 | CMP/CPH/CPL | Comparison | Various | All |
| 0x72-0x74 | LDA variants | Data | REG, REG | - |
| 0x75 | PUSHW | Stack | VALUE | - |
| 0x76 | PUSHB | Stack | VALUE | - |
| 0x7F | SYS | System | FUNC | * |

*Flags affected conditionally

---

## Programming Examples

### Example 1: Simple Loop

```assembly
CODE
    LD CX, 10           ; Loop 10 times
    LD AX, 0            ; Accumulator
loop:
    ADD AX, 1           ; Increment accumulator
    DEC CX              ; Decrement counter
    JPNZ loop           ; Continue if CX != 0
    HALT                ; AX = 10
```

### Example 2: Array Sum

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    count: DW [5]
    result: DW [0]

CODE
    LD AX, 0            ; Sum accumulator
    LDA BX, array       ; Array pointer
    LD CX, [count]      ; Element count

sum_loop:
    LDAB DX, (BX)       ; Load byte from array
    ADD AX, DX          ; Add to sum
    INC BX              ; Next element
    DEC CX              ; Decrement count
    JPNZ sum_loop       ; Continue if more elements
    
    LD [result], AX     ; Store result (150)
    HALT
```

### Example 3: Bit Manipulation

```assembly
CODE
    ; Set bits 4-7
    LD AX, 0x0000
    OR AX, 0x00F0       ; AX = 0x00F0
    
    ; Clear bits 0-3
    AND AX, 0xFFF0      ; AX = 0x00F0
    
    ; Toggle bit 8
    XOR AX, 0x0100      ; AX = 0x01F0
    
    ; Test bit 8
    LD BX, AX
    AND BX, 0x0100      ; BX = 0x0100 (bit is set)
    CMP BX, 0
    JPNZ bit_is_set
    
bit_is_set:
    HALT
```

### Example 4: Function Call

```assembly
DATA
    value: DW [10]

CODE
main:
    LD AX, [value]
    CALL square         ; Call square function
    LD [value], AX      ; Store result (100)
    HALT

square:
    MUL AX, AX          ; AX = AX * AX
    RET
```

### Example 5: Conditional Logic

```assembly
DATA
    temperature: DW [75]
    status: DW [0]

CODE
    LD AX, [temperature]
    
    ; Check if temperature > 100
    CMP AX, 100
    JPZ too_hot
    
    ; Check if temperature < 50
    CMP AX, 50
    JPNZ normal
    
too_cold:
    LD BX, 1            ; Status: cold
    JMP store_status
    
too_hot:
    LD BX, 2            ; Status: hot
    JMP store_status
    
normal:
    LD BX, 0            ; Status: normal
    
store_status:
    LD [status], BX
    HALT
```

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Assembly language syntax
- **[Register Reference](Registers.md)**: Register details and usage
- **[Expressions](Expressions.md)**: Address expressions
- **[Examples](Examples.md)**: Complete program examples
