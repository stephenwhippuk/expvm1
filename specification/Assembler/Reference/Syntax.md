# Assembly Language Syntax Reference

## Overview

This document provides a comprehensive reference for the Pendragon Assembly Language syntax. The assembler uses a traditional two-section format with clear, readable syntax inspired by classic assembly languages.

## File Structure

Every assembly program consists of up to two sections:

```assembly
DATA
    ; Data definitions go here

CODE
    ; Instructions go here
```

### Section Rules

1. **Optional Sections**: Both `DATA` and `CODE` sections are optional, but at least one must be present
2. **Section Order**: `DATA` section must come before `CODE` section (if both present)
3. **Single Instance**: Each section can appear only once in a program
4. **Case Insensitive**: Section keywords are case-insensitive (`DATA`, `data`, `Data` all valid)

### Empty Sections

Sections can be empty:

```assembly
DATA
    ; No data definitions

CODE
    start:
        HALT
```

## Comments

Comments begin with a semicolon (`;`) and extend to the end of the line.

```assembly
; This is a full-line comment
DATA
    value: DW [42]    ; This is an end-of-line comment
    
CODE
    ; Comments can appear anywhere
    LDA AX, value     ; Load the value from memory
    HALT              ; And stop
```

### Comment Rules

- Single-line only (no multi-line comment syntax)
- Can appear on any line
- Can follow code on the same line
- Extend to the end of the line (no inline comments)
- Any text after `;` is ignored

## Identifiers and Labels

### Naming Rules

Identifiers (labels and symbol names) must follow these rules:

1. **Start Character**: Must begin with a letter (a-z, A-Z) or underscore (`_`)
2. **Subsequent Characters**: Can contain letters, digits (0-9), or underscores
3. **Case Sensitive**: `myLabel`, `MyLabel`, and `MYLABEL` are different symbols
4. **Not Keywords**: Cannot use reserved words (DATA, CODE, DB, DW, register names)
5. **Length**: No explicit limit, but keep reasonable (< 64 characters recommended)

### Valid Identifiers

```assembly
label1          ; Valid
_start          ; Valid
my_label        ; Valid
MyFunction123   ; Valid
__internal      ; Valid
```

### Invalid Identifiers

```assembly
123start        ; Cannot start with digit
my-label        ; Hyphen not allowed
my.label        ; Dot not allowed
DATA            ; Reserved keyword
AX              ; Reserved (register name)
```

## Labels

Labels mark positions in code or data sections. They can be defined in two contexts:

### Code Labels

Code labels mark instruction addresses:

```assembly
CODE
start:              ; Label with no instruction
    LD AX, 10
    
loop:               ; Label before instruction
    DEC AX
    JPNZ loop       ; Reference to label
    
end:                ; Label at end
    HALT
```

#### Code Label Rules

1. **Declaration**: Label name followed by colon (`:`)
2. **Placement**: Can appear on their own line or before an instruction
3. **Whitespace**: Optional whitespace before label, required after colon if instruction follows
4. **Multiple Labels**: Multiple labels can point to the same location

### Data Labels

Data labels name data definitions:

```assembly
DATA
count: DB [0]               ; Single byte
buffer: DB [10, 20, 30]     ; Array
message: DB "Hello"         ; String
value: DW [0x1234]          ; Word (16-bit)
```

#### Data Label Rules

1. **Required**: Every data definition must have a label
2. **Unique**: Each label must be unique within its section
3. **Format**: `label: TYPE [data]`

## Data Definitions

Data definitions allocate and initialize data in the DATA section.

### DB - Define Byte(s)

Defines one or more 8-bit bytes.

#### Single Byte

```assembly
DATA
byte_value: DB [42]         ; Decimal
hex_byte: DB [0xFF]         ; Hexadecimal
```

#### Byte Arrays

```assembly
DATA
array: DB [1, 2, 3, 4, 5]           ; Array of bytes
flags: DB [0x01, 0x02, 0x04, 0x08]  ; Hex array
empty: DB []                         ; Zero-length array (valid)
```

#### String Literals

```assembly
DATA
message: DB "Hello, World!"     ; String stored as bytes
newline: DB "\n"                ; Escape sequence
path: DB "C:\\Users\\name"      ; Escaped backslash
```

##### Supported Escape Sequences

- `\n` - Newline (0x0A)
- `\r` - Carriage return (0x0D)
- `\t` - Tab (0x09)
- `\\` - Backslash
- `\"` - Double quote
- `\0` - Null byte (0x00)

#### Multi-line Strings

Strings can span multiple lines:

```assembly
DATA
long_text: DB "This is a very long string that
spans multiple lines in the source code
but is stored as a continuous byte sequence"
```

### DW - Define Word(s)

Defines one or more 16-bit words (stored in little-endian format).

#### Single Word

```assembly
DATA
word_value: DW [1000]           ; Decimal word
hex_word: DW [0x1234]           ; Hexadecimal word
```

#### Word Arrays

```assembly
DATA
coordinates: DW [100, 200, 300]     ; Array of words
addresses: DW [0x0000, 0x1000, 0x2000]  ; Address table
```

### Data Definition Rules

1. **Label Required**: Every definition must have a label
2. **Brackets Required**: Data values must be enclosed in `[...]`
3. **Type Enforcement**: DB takes 8-bit values, DW takes 16-bit values
4. **Range Checking**: Values must fit in their type (0-255 for DB, 0-65535 for DW)
5. **Comma Separation**: Array elements separated by commas
6. **No Mixing**: Cannot mix strings and numeric arrays in a single definition

## Instructions

Instructions consist of a mnemonic and zero or more operands.

### Instruction Format

```assembly
MNEMONIC                    ; No operands (e.g., HALT, NOP, RET)
MNEMONIC operand            ; One operand (e.g., PUSH AX, JMP label)
MNEMONIC dest, source       ; Two operands (e.g., LD AX, 10)
```

### Instruction Rules

1. **Case Insensitive**: Mnemonics are case-insensitive (`HALT`, `halt`, `Halt` all valid)
2. **Whitespace**: At least one space or tab required between mnemonic and operands
3. **Comma Separator**: Multiple operands separated by comma (`,`)
4. **Optional Spaces**: Spaces around commas are optional but recommended

### Examples

```assembly
CODE
    HALT                ; No operands
    NOP                 ; No operands
    RET                 ; No operands
    
    PUSH AX             ; One operand (register)
    PUSH 100            ; One operand (immediate)
    JMP start           ; One operand (label)
    
    LD AX, 42           ; Two operands (register, immediate)
    LDA BX, value       ; Two operands (register, memory)
    ADD AX, BX          ; Two operands (register, register)
```

## Operands

Operands specify the data that instructions operate on.

### Operand Types

#### 1. Immediate Values (Constants)

Literal numeric values embedded in the instruction.

```assembly
CODE
    PUSHW 42            ; Decimal immediate
    PUSHW 0xFF          ; Hexadecimal immediate
    LD AX, 1000         ; 16-bit immediate
    PUSHB 10            ; 8-bit immediate
```

**Number Formats:**
- **Decimal**: `42`, `1000`, `255`
- **Hexadecimal**: `0x2A`, `0x03E8`, `0xFF` (prefix: `0x`)

**Rules:**
- Case insensitive for hex digits (`0xFF` = `0xff`)
- No separators allowed (no `1_000` or `0xFF_FF`)
- Must fit in instruction's operand size

#### 2. Registers

Reference to one of the CPU registers.

**16-bit Registers:**
```assembly
CODE
    LD AX, 100          ; Accumulator
    LD BX, 200          ; Base
    LD CX, 10           ; Counter
    LD DX, buffer       ; Data (address of buffer)
    LD EX, 0x1000       ; Extended
```

**8-bit Sub-Registers:**
```assembly
CODE
    LD AH, 0x12         ; High byte of AX
    LD AL, 0x34         ; Low byte of AX
    LD BH, 10           ; High byte of BX
    LD BL, 20           ; Low byte of BX
    ; Similar for CH/CL, DH/DL, EH/EL
```

**Rules:**
- Register names are case-insensitive
- Must use correct register for instruction (some instructions require specific registers)
- Sub-registers access high/low bytes of parent register

#### 3. Memory Addresses and Labels

Reference to a data label or code label. The syntax varies by instruction and context.

```assembly
DATA
value: DW [42]
buffer: DB [10, 20, 30]

CODE
start:
    ; Address operations (LD instruction)
    LD AX, value        ; Load ADDRESS of value into AX
    LD BX, (buffer + 2) ; Compute address (buffer + 2)
    
    ; Memory access operations (LDA instruction)
    LDA AX, value       ; Load VALUE from memory at value
    LDA CX, (buffer + 2) ; Load VALUE from computed address
    
    ; Syntactic sugar - array indexing
    LDA DX, buffer[2]   ; Sugar for: LDA DX, (buffer + 2)
    
    ; Jump/Call targets
    JMP start           ; Jump to code label
    CALL function       ; Call code label
```

**Instruction-Specific Syntax:**

**LD Instruction** (Load Address):
- `LD reg, label` - Load address of label into register
- `LD reg, (expression)` - Compute and load address
- `LD reg, label[index]` - Sugar syntax (converted to LDA/LDAB based on register size)

**LDA/LDAB/LDAW Instructions** (Memory Access):
- `LDA reg, label` - Load value FROM memory at label
- `LDA reg, (expression)` - Load value FROM computed address
- `LDA label, reg` - Store value TO memory at label
- `LDA (expression), reg` - Store value TO computed address

**Jump/Call Instructions:**
- Just the label name: `JMP label`, `CALL label`

**Square Brackets [] - Syntactic Sugar Only:**
- `label[expr]` is syntactic sugar that converts to memory access
- Examples: 
  - `LD AX, buffer[5]` → becomes → `LDA AX, (buffer + 5)` (16-bit register)
  - `LD AL, buffer[5]` → becomes → `LDAB AL, (buffer + 5)` (8-bit register)
- **Do not use** `[label]` or `[expression]` directly - use appropriate syntax above

#### 4. Address Expressions

Arithmetic expressions for address calculations, enclosed in parentheses.

```assembly
DATA
array: DB [1, 2, 3, 4, 5]

CODE
    ; Address expressions with parentheses
    LDA AX, (array + 2)      ; Load from array[2]
    LDA BX, (array + CX)     ; Dynamic index using register
    LDAB DL, (array + BX + 1) ; Complex expression
    
    ; Address loading
    LD CX, (array + 4)       ; Compute address of array[4]
    LD DX, array             ; Load base address
```

**Expression Components:**
- **Labels**: Base address (required first element)
- **Literals**: Numeric offsets
- **Registers**: Dynamic offsets (BX, CX, DX, etc.)
- **Operators**: `+` (addition), `-` (subtraction)

**Expression Rules:**
1. **Parentheses Required**: Use `(expression)` for address arithmetic
2. **Label First**: Expression must start with a label
3. **Operators**: Only `+` and `-` supported
4. **No Multiplication**: No `*`, `/`, or other operators
5. **Register Offsets**: Can add register values for dynamic addressing
6. **Evaluation**: Label + literal resolved at assembly time; register parts at runtime

**Array Indexing Patterns:**
```assembly
CODE
    ; Byte array access with parentheses
    LDAB AL, (bytes + 0)       ; First byte
    LDAB AL, (bytes + BX)      ; Dynamic byte index
    
    ; Word array access (2 bytes apart)
    LDA AX, words              ; First word
    LDA BX, (words + 2)        ; Second word  
    LDA CX, (words + DX)       ; Dynamic word index
    
    ; Sugar syntax (recommended for readability with register indices)
    LD AX, bytes[BX]           ; Becomes: LDA AX, (bytes + BX)
    LD BX, words[CX]           ; Becomes: LDA BX, (words + CX)
    LD DL, bytes[BX]           ; Becomes: LDAB DL, (bytes + BX)
    
    ; Sugar syntax works with any expression
    LD AX, array[BX + 1]       ; Becomes: LDA AX, (array + BX + 1)
    LD AL, buffer[CX]          ; Becomes: LDAB AL, (buffer + CX)
```

#### 5. Indirect Addressing

Memory access through a register containing an address.

```assembly
CODE
    LD BX, buffer       ; BX = address of buffer
    LDAB AL, BX         ; Load byte from address in BX
    LDA CX, DX          ; Load word from address in DX
```

**Rules:**
- Register contains the address to access
- No parentheses needed for basic indirect addressing
- Use with LDA/LDAB/LDAW instructions for memory operations

## Numeric Literals

### Supported Formats

#### Decimal
```assembly
0
42
255
1000
65535
```

#### Hexadecimal
```assembly
0x00            ; Zero
0xFF            ; 255
0x1234          ; 4660
0xABCD          ; Case insensitive
0xabcd          ; Same as above
```

**Note**: Binary literals (0b prefix) are not currently supported.

### Range Restrictions

- **8-bit (byte)**: 0 to 255 (0x00 to 0xFF)
- **16-bit (word)**: 0 to 65535 (0x0000 to 0xFFFF)
- Assembler will error if value exceeds operand size

### Negative Numbers

The assembler does not support negative number syntax. Use two's complement representation:

```assembly
CODE
    PUSHW 0xFFFF        ; -1 in 16-bit two's complement
    PUSHB 0xFF          ; -1 in 8-bit two's complement
    LD AX, 0xFFFE       ; -2 in 16-bit
```

## Whitespace Rules

### Required Whitespace

1. **After Label Colon**: Labels must be followed by newline
   ```assembly
   label:
       HALT            ; Labels require newline, then instruction
   ```

2. **Between Mnemonic and Operands**: At least one space/tab
   ```assembly
   LD AX, 10           ; Space after LD
   ```

### Optional Whitespace

1. **Around Commas**: Spaces around operand separators are optional
   ```assembly
   LD AX,10            ; Valid
   LD AX, 10           ; Valid
   LD AX , 10          ; Valid
   ```

2. **Start of Line**: Lines can be indented
   ```assembly
   CODE
       start:          ; Indented label
           HALT        ; Indented instruction
   ```

3. **Around Operators**: Spaces around `+` and `-` in expressions
   ```assembly
   LDA AX, (buffer+5)   ; Valid
   LDA AX, (buffer + 5) ; Valid
   ```

### Blank Lines

Blank lines are allowed and ignored anywhere in the file.

```assembly
DATA
    value: DW [42]

                        ; Multiple blank lines OK

CODE
    HALT
```

## Complete Example

Here's a complete program demonstrating all syntax elements:

```assembly
; ========================================
; Example: String Length Calculator
; Counts characters in a string
; ========================================

DATA
    ; String to measure (null-terminated)
    text: DB "Hello, Pendragon VM!\0"
    
    ; Storage for result
    length: DW [0]
    
    ; Constants
    null_byte: DB [0]

CODE
    ; Initialize
start:
    LD AX, text         ; AX = address of string
    LD BX, 0            ; BX = counter (starts at 0)
    
    ; Count loop
count_loop:
    LDAB CL, AX         ; Load byte from address in AX
    LDAB DL, null_byte  ; Load null byte value
    CMP CL, DL          ; Compare with null
    JPZ done            ; If zero, done
    
    INC BX              ; Increment counter
    INC AX              ; Move to next character
    JMP count_loop      ; Continue loop
    
    ; Store result and halt
done:
    LDA length, BX      ; Store length
    HALT                ; End program
```

## Syntax Error Examples

Understanding common syntax errors:

### Missing Colon on Label

```assembly
DATA
value DW [42]           ; ERROR: Missing colon after 'value'
```

**Correct:**
```assembly
value: DW [42]
```

### Missing Brackets on Data

```assembly
DATA
array: DB 1, 2, 3       ; ERROR: Missing brackets around data
```

**Correct:**
```assembly
array: DB [1, 2, 3]
```

### Invalid Identifier

```assembly
DATA
123value: DW [42]       ; ERROR: Identifier starts with digit
```

**Correct:**
```assembly
value123: DW [42]
```

### Wrong Section Order

```assembly
CODE                    ; ERROR: CODE before DATA
    HALT

DATA
    value: DW [42]
```

**Correct:**
```assembly
DATA
    value: DW [42]

CODE
    HALT
```

### Missing Operand

```assembly
CODE
    LD AX               ; ERROR: LD requires two operands
```

**Correct:**
```assembly
CODE
    LD AX, 10
```

## Best Practices

### 1. Consistent Indentation

Indent instructions and data definitions for readability:

```assembly
DATA
    value: DW [42]
    buffer: DB [1, 2, 3]

CODE
start:
    LD AX, 10
    HALT
```

### 2. Meaningful Labels

Use descriptive names that indicate purpose:

```assembly
; Good
DATA
    player_score: DW [0]
    enemy_count: DB [5]

CODE
initialize_game:
    ; ...

; Avoid
DATA
    x: DW [0]
    y: DB [5]

CODE
fn1:
    ; ...
```

### 3. Comments for Clarity

Add comments to explain complex logic:

```assembly
CODE
    ; Set up loop counter for 10 iterations
    LD CX, 10
    
loop:
    ; Perform operation
    ADD AX, BX
    
    ; Decrement and loop if not zero
    DEC CX
    JPNZ loop
```

### 4. Organize Code Sections

Group related code together:

```assembly
CODE
    ; === Initialization ===
init:
    LD AX, 0
    LD BX, 0

    ; === Main Loop ===
main_loop:
    ; ...
    
    ; === Subroutines ===
calculate:
    ; ...
    RET
```

### 5. Align Operands

Align operands in columns for readability:

```assembly
CODE
    LD    AX, 100
    LD    BX, 200
    ADD   AX, BX
    LDA   result, AX
```

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Register Reference](Registers.md)**: Detailed register information
- **[Instruction Set](InstructionSet.md)**: Complete instruction reference
- **[Expressions](Expressions.md)**: Advanced expression syntax
- **[Examples](Examples.md)**: Complete program examples
