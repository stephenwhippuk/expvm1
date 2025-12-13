# Register Reference

## Overview

The Pendragon VM provides five 16-bit general-purpose registers, each of which can be accessed as a full 16-bit word or as two separate 8-bit bytes (high and low). This architecture allows for efficient manipulation of both word-sized and byte-sized data.

## Register Architecture

### Memory Layout

Each 16-bit register is divided into two 8-bit sub-registers:

```
     15                    8  7                     0
     ┌──────────────────────┬──────────────────────┐
 AX  │         AH           │         AL           │
     └──────────────────────┴──────────────────────┘
     
     ┌──────────────────────┬──────────────────────┐
 BX  │         BH           │         BL           │
     └──────────────────────┴──────────────────────┘
     
     ┌──────────────────────┬──────────────────────┐
 CX  │         CH           │         CL           │
     └──────────────────────┴──────────────────────┘
     
     ┌──────────────────────┬──────────────────────┐
 DX  │         DH           │         DL           │
     └──────────────────────┴──────────────────────┘
     
     ┌──────────────────────┬──────────────────────┐
 EX  │         EH           │         EL           │
     └──────────────────────┴──────────────────────┘
```

### Sub-Register Access

- **High Byte (H)**: Bits 15-8 of the parent register
- **Low Byte (L)**: Bits 7-0 of the parent register

Modifications to sub-registers affect the parent register and vice versa:

```assembly
CODE
    LD AX, 0x1234       ; AX = 0x1234, AH = 0x12, AL = 0x34
    LD AH, 0xFF         ; AX = 0xFF34, AH = 0xFF, AL = 0x34
    LD AL, 0x00         ; AX = 0xFF00, AH = 0xFF, AL = 0x00
```

## 16-Bit Registers

### AX - Accumulator Register

**Primary Use**: Arithmetic and logical operations

**Description**: The accumulator register is the primary register for arithmetic, logical, and data manipulation operations. Many instructions implicitly use AX as a source or destination operand.

**Common Uses**:
- Arithmetic calculations (addition, subtraction, multiplication, division)
- Logical operations (AND, OR, XOR, NOT)
- Result storage
- General-purpose data manipulation

**Examples**:
```assembly
CODE
    ; Arithmetic
    LD AX, 100
    ADD AX, 50          ; AX = 150
    MUL AX, 2           ; AX = 300
    
    ; Logical operations
    LD AX, 0xFF00
    AND AX, 0x00FF      ; AX = 0x0000
    
    ; Data loading
    LD AX, [value]      ; Load from memory
    LD [result], AX     ; Store to memory
```

### BX - Base Register

**Primary Use**: Base pointer for memory addressing

**Description**: The base register is commonly used to hold base addresses for memory operations, particularly when accessing arrays or data structures.

**Common Uses**:
- Array base addresses
- Pointer storage
- Index calculations
- General-purpose operations

**Examples**:
```assembly
DATA
    array: DB [10, 20, 30, 40, 50]

CODE
    ; Array access
    LDA BX, array       ; BX = address of array
    LD AL, (BX)         ; Load first element (indirect addressing)
    
    ; Offset calculations
    LD BX, 100
    ADD BX, 50          ; BX = 150 (base + offset)
```

### CX - Counter Register

**Primary Use**: Loop counters and iteration control

**Description**: The counter register is traditionally used for loop counting and repetitive operations. It's the natural choice for any operation that requires counting or iteration.

**Common Uses**:
- Loop counters
- Iteration counts
- Repetition control
- String/array length tracking

**Examples**:
```assembly
CODE
    ; Simple loop
    LD CX, 10           ; 10 iterations
loop:
    ; ... loop body ...
    DEC CX              ; Decrement counter
    JPNZ loop           ; Continue if CX != 0
    
    ; Array processing
    LD CX, 5            ; Array has 5 elements
    LDA BX, array       ; BX = array base
process_loop:
    LD AL, (BX)         ; Load element
    ; ... process AL ...
    INC BX              ; Next element
    DEC CX              ; Decrement count
    JPNZ process_loop
```

### DX - Data Register

**Primary Use**: Data storage and I/O operations

**Description**: The data register is used for general data storage and is often paired with AX for operations that require multiple data registers.

**Common Uses**:
- Temporary data storage
- Secondary operand storage
- I/O port addressing
- Multi-register operations

**Examples**:
```assembly
CODE
    ; Swapping values
    LD AX, [value1]
    LD DX, [value2]
    LD [value1], DX
    LD [value2], AX
    
    ; Multi-register arithmetic
    LD AX, 100
    LD DX, 200
    ADD AX, DX          ; AX = 300
```

### EX - Extended Register

**Primary Use**: Extended operations and additional general-purpose storage

**Description**: The extended register provides additional general-purpose register space for complex operations or when all other registers are in use.

**Common Uses**:
- Additional temporary storage
- Preservation of values during complex operations
- Extended calculations
- Function call parameter passing

**Examples**:
```assembly
CODE
    ; Preserving values
    LD EX, AX           ; Save AX in EX
    ; ... use AX for something else ...
    LD AX, EX           ; Restore AX
    
    ; Complex calculations
    LD AX, 10
    LD BX, 20
    LD CX, 30
    LD DX, 40
    LD EX, 50           ; All five registers in use
```

## 8-Bit Sub-Registers

### High Byte Registers (AH, BH, CH, DH, EH)

**Access**: Upper 8 bits (bits 15-8) of the parent register

**Use Cases**:
- Byte-sized arithmetic on high byte
- Packing two independent byte values in one register
- Character/string operations on high byte
- Bit manipulation of upper bits

**Examples**:
```assembly
CODE
    ; High byte operations
    LD AX, 0x1234
    LD AH, 0xFF         ; AX becomes 0xFF34
    ADD AH, 1           ; AX becomes 0x0034 (overflow wraps)
    
    ; Packing two values
    LD AH, 0x12         ; Store first byte in high
    LD AL, 0x34         ; Store second byte in low
    ; AX now contains 0x1234
```

### Low Byte Registers (AL, BL, CL, DL, EL)

**Access**: Lower 8 bits (bits 7-0) of the parent register

**Use Cases**:
- Byte-sized arithmetic on low byte
- Character operations
- Small numeric values (0-255)
- Efficient byte manipulation

**Examples**:
```assembly
CODE
    ; Low byte operations
    LD AX, 0x1234
    LD AL, 0xFF         ; AX becomes 0x12FF
    ADD AL, 1           ; AX becomes 0x1200 (overflow wraps)
    
    ; Character handling
    LD AL, 'A'          ; Load ASCII character (0x41)
    ADD AL, 32          ; Convert to lowercase ('a' = 0x61)
```

### Sub-Register Naming Convention

- **H suffix**: High byte (e.g., AH = high byte of AX)
- **L suffix**: Low byte (e.g., AL = low byte of AX)
- Parent register name + suffix

**Complete List**:
```
16-bit    High (8-bit)    Low (8-bit)
------    ------------    -----------
  AX          AH              AL
  BX          BH              BL
  CX          CH              CL
  DX          DH              DL
  EX          EH              EL
```

## Register Encoding

Registers are encoded in machine code using 4-bit values:

```
Register    Encoding
--------    --------
AX          0x0
BX          0x1
CX          0x2
DX          0x3
EX          0x4
```

Sub-registers use the same encoding as their parent register, with the instruction opcode indicating whether the operation is on the full 16-bit register or an 8-bit sub-register.

## Register Usage Conventions

### Best Practices

1. **Use AX for Primary Operations**
   - Main arithmetic and logical operations
   - Return values from calculations
   - Primary data manipulation

2. **Use BX for Addressing**
   - Base pointers for arrays
   - Memory address storage
   - Data structure pointers

3. **Use CX for Counting**
   - Loop counters
   - String lengths
   - Array sizes

4. **Use DX for Secondary Data**
   - Paired operations with AX
   - Temporary storage
   - I/O operations

5. **Use EX for Additional Storage**
   - When other registers are occupied
   - Saving/restoring register values
   - Extended calculations

### Register Preservation

When calling subroutines or performing complex operations, consider preserving register values:

```assembly
CODE
my_function:
    ; Save registers
    PUSH AX
    PUSH BX
    PUSH CX
    
    ; Function body
    LD AX, 100
    LD BX, 200
    ADD AX, BX
    
    ; Restore registers
    POP CX
    POP BX
    POP AX
    RET
```

## Instruction Compatibility

### Full Register Instructions

Most instructions can operate on any 16-bit register:

```assembly
CODE
    LD AX, 100          ; Load into AX
    LD BX, 200          ; Load into BX
    LD CX, 300          ; Load into CX
    LD DX, 400          ; Load into DX
    LD EX, 500          ; Load into EX
    
    ADD AX, BX          ; Add any two registers
    SWP CX, DX          ; Swap any two registers
```

### Sub-Register Instructions

Instructions with byte suffixes (B, H, L) operate on 8-bit sub-registers:

```assembly
CODE
    ; Byte operations
    ADB AH, 10          ; Add byte to high byte of AX
    SBL AL, 5           ; Subtract byte from low byte of AX
    
    ; Load high/low variants
    LDH AH, 0x12        ; Load high byte
    LDL BL, 0x34        ; Load low byte
```

### Register-Specific Instructions

Some instructions have specific register requirements. Refer to the [Instruction Set Reference](InstructionSet.md) for details.

## Examples

### Example 1: Byte Packing/Unpacking

```assembly
DATA
    packed: DW [0x1234]

CODE
    ; Unpack a word into separate bytes
    LD AX, [packed]     ; AX = 0x1234
    LD BL, AH           ; BL = 0x12 (high byte)
    LD CL, AL           ; CL = 0x34 (low byte)
    
    ; Pack two bytes into a word
    LD DH, 0xAB
    LD DL, 0xCD
    LD [packed], DX     ; Write 0xABCD to memory
```

### Example 2: Array Iteration

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    sum: DW [0]

CODE
    ; Sum array elements
    LD AX, 0            ; AX = accumulator (sum)
    LDA BX, array       ; BX = array base pointer
    LD CX, 5            ; CX = element count
    
sum_loop:
    LD DL, (BX)         ; Load array element into DL
    ADD AL, DL          ; Add to sum (byte addition)
    INC BX              ; Move to next element
    DEC CX              ; Decrement counter
    JPNZ sum_loop       ; Continue if CX != 0
    
    LD [sum], AX        ; Store result
```

### Example 3: Multi-Register Calculation

```assembly
CODE
    ; Calculate: result = (a + b) * (c - d)
    LD AX, [a]
    ADD AX, [b]         ; AX = a + b
    
    LD DX, [c]
    SUB DX, [d]         ; DX = c - d
    
    MUL AX, DX          ; AX = (a + b) * (c - d)
    LD [result], AX
```

### Example 4: Character Processing

```assembly
DATA
    input: DB "Hello"
    output: DB [0, 0, 0, 0, 0]

CODE
    ; Convert string to uppercase
    LDA BX, input       ; Source pointer
    LDA DX, output      ; Destination pointer
    LD CX, 5            ; Character count
    
convert_loop:
    LD AL, (BX)         ; Load character
    
    ; Check if lowercase (a-z: 0x61-0x7A)
    CMP AL, 0x61
    JPC skip            ; If AL < 'a', skip
    CMP AL, 0x7A
    JPNC skip           ; If AL > 'z', skip
    
    ; Convert to uppercase (subtract 32)
    SUB AL, 32
    
skip:
    LD (DX), AL         ; Store character
    INC BX              ; Next source
    INC DX              ; Next destination
    DEC CX              ; Decrement counter
    JPNZ convert_loop
    
    HALT
```

### Example 5: Register Swapping

```assembly
CODE
    ; Swap AX and BX without temporary memory
    LD AX, 100
    LD BX, 200
    
    ; Method 1: Using SWP instruction
    SWP AX, BX          ; AX = 200, BX = 100
    
    ; Method 2: Using arithmetic (if SWP not available)
    LD AX, 100
    LD BX, 200
    ADD AX, BX          ; AX = 300
    SUB BX, AX          ; BX = -100 (wraps to 100 as unsigned)
    ADD BX, AX          ; BX = 200
    SUB AX, BX          ; AX = 100
```

## Register State and Flags

Register operations affect processor flags:

- **ZERO flag**: Set when result is zero
- **CARRY flag**: Set on arithmetic overflow/underflow
- **SIGN flag**: Set when result is negative (bit 15 = 1 for 16-bit, bit 7 = 1 for 8-bit)
- **OVERFLOW flag**: Set on signed arithmetic overflow

Example:
```assembly
CODE
    LD AL, 255
    ADD AL, 1           ; AL = 0, ZERO flag set, CARRY flag set
    
    LD AX, 0x8000
    ADD AX, 1           ; AX = 0x8001, SIGN flag set (bit 15 = 1)
```

## Common Pitfalls

### 1. Sub-Register Independence Assumption

**Incorrect Assumption**: Sub-registers are independent of parent register

```assembly
CODE
    LD AX, 0x0000       ; Clear AX
    LD AH, 0x12         ; AX is now 0x1200, NOT just AH affected!
```

### 2. Overflow in Sub-Register Operations

**Pitfall**: Overflow in 8-bit operations affects parent register unexpectedly

```assembly
CODE
    LD AX, 0x00FF       ; AH = 0x00, AL = 0xFF
    ADD AL, 1           ; AL overflows to 0x00
                        ; AX is now 0x0000 (entire AX affected)
                        ; CARRY flag is set
```

### 3. Register Clobbering in Loops

**Pitfall**: Forgetting which registers are modified in nested operations

```assembly
CODE
    LD CX, 10           ; Outer loop counter
outer:
    LD CX, 5            ; BUG! Overwrites outer loop counter
    ; ... inner loop ...
    DEC CX
    JPNZ outer          ; Will loop incorrectly
```

**Solution**: Use different registers or save/restore:
```assembly
CODE
    LD CX, 10           ; Outer loop counter
outer:
    PUSH CX             ; Save outer counter
    LD CX, 5            ; Inner loop counter
    ; ... inner loop ...
    POP CX              ; Restore outer counter
    DEC CX
    JPNZ outer
```

### 4. Signed vs Unsigned Confusion

**Pitfall**: Treating signed and unsigned values incorrectly

```assembly
CODE
    LD AL, 0xFF         ; Could be 255 (unsigned) or -1 (signed)
    CMP AL, 0           ; Comparison treats as unsigned (255 > 0)
```

## Register Quick Reference

| Register | Size | Purpose | Sub-Regs | Encoding |
|----------|------|---------|----------|----------|
| AX | 16-bit | Accumulator | AH, AL | 0x0 |
| BX | 16-bit | Base pointer | BH, BL | 0x1 |
| CX | 16-bit | Counter | CH, CL | 0x2 |
| DX | 16-bit | Data | DH, DL | 0x3 |
| EX | 16-bit | Extended | EH, EL | 0x4 |

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Assembly language syntax
- **[Instruction Set](InstructionSet.md)**: Complete instruction reference
- **[Examples](Examples.md)**: Complete program examples
- **[Expressions](Expressions.md)**: Address expressions and calculations
