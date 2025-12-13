# Pendragon Assembler - Overview

## Introduction

The Pendragon Assembler (`asm`) is a multi-pass assembler for the Pendragon Virtual Machine. It translates human-readable assembly language into executable binary files that can be loaded and run by the Pendragon VM.

### Key Features

- **Complete instruction set support**: All 119 Pendragon VM instructions
- **Full register support**: 16-bit (AX, BX, CX, DX, EX) and 8-bit sub-registers (AH, AL, etc.)
- **Flexible addressing modes**: Immediate, register, direct, and expression-based addressing
- **Rich error reporting**: Clear error messages with line and column numbers
- **Standards-compliant**: Generates binary files compatible with the Pendragon VM BinaryLoader

## Installation

The assembler is built as part of the Pendragon VM project:

```bash
cd /path/to/expvm1
cmake -B build
cmake --build build
```

This creates the `asm` executable in the `build/` directory.

## Quick Start

### 1. Create an Assembly File

Create a file named `hello.asm`:

```assembly
; Simple hello world program
DATA
    message: DB "Hello, World!"

CODE
start:
    LD CX, 0x0B         ; Length of message
    
loop:
    DEC CX
    LDA AX, (message + CX)
    PUSHB AX
    CMP CX, 0x00
    JPNZ loop
    
    PUSHW 0x000B        ; Push length
    SYS 0x0011          ; System call to print
    HALT
```

### 2. Assemble the Program

```bash
./build/asm hello.asm -o hello.bin
```

If successful, you'll see:
```
Assembly successful: hello.bin
```

### 3. Run the Program

```bash
./build/lvm hello.bin 0x0000
```

## Command-Line Usage

```
asm <input.asm> -o <output.bin>
```

### Arguments

- `<input.asm>` - Source assembly file to compile
- `-o <output.bin>` - Output binary file path (required)

### Examples

```bash
# Basic assembly
asm program.asm -o program.bin

# Using different paths
asm src/test.asm -o bin/test.bin

# Multiple source files (future feature)
# asm main.asm lib.asm -o program.bin
```

## Assembly File Structure

Every assembly file must follow this structure:

```assembly
; Comments start with semicolon

DATA                    ; Data section (optional)
    label1: DB [...]    ; Define bytes
    label2: DW [...]    ; Define words

CODE                    ; Code section (required)
label3:                 ; Code labels
    INSTRUCTION         ; Instructions
    INSTRUCTION
    HALT                ; Program should end with HALT
```

### Sections

- **DATA Section** (optional): Contains data definitions that will be placed in the data segment
- **CODE Section** (required): Contains executable instructions

### Elements

- **Comments**: Start with `;` and continue to end of line
- **Labels**: Identifiers followed by `:` that mark positions in memory
- **Data Definitions**: `DB` (byte) or `DW` (word) followed by values
- **Instructions**: Operation mnemonics with operands

## Compilation Workflow

The assembler processes your code in 5 passes:

```
Source File (.asm)
    ↓
[Pass 1: Lexer & Parser]
    ↓
Abstract Syntax Tree (AST)
    ↓
[Pass 2: Semantic Analysis]
    ↓
Validated AST + Symbol Table
    ↓
[Pass 3: Code Graph Construction]
    ↓
Intermediate Representation
    ↓
[Pass 4: Address Resolution]
    ↓
Resolved Addresses
    ↓
[Pass 5: Binary Generation]
    ↓
Binary File (.bin)
```

### Pass Details

1. **Lexer & Parser**: Converts text into structured representation (AST)
2. **Semantic Analysis**: Validates symbols, types, and references
3. **Code Graph**: Creates intermediate representation with separate data/code sections
4. **Address Resolution**: Calculates absolute addresses for all labels and data
5. **Binary Generation**: Emits final machine code in BinaryLoader format

## Understanding Errors

The assembler provides detailed error messages:

```
Semantic error at 15:9: Undefined symbol 'LOOP'
```

This tells you:
- **Error type**: Semantic (meaning/validity error)
- **Location**: Line 15, column 9
- **Problem**: The symbol 'LOOP' is referenced but not defined

Common error types:
- **Lexical errors**: Invalid characters or tokens
- **Syntax errors**: Grammar violations
- **Semantic errors**: Invalid symbols, types, or references
- **Builder errors**: Problems constructing intermediate representation
- **Resolution errors**: Cannot calculate addresses

## Binary Output Format

The assembler generates binary files in the Pendragon VM BinaryLoader format:

```
[Header]
- Header size (2 bytes)
- Header version (4 bytes): major.minor.revision
- Machine name size (1 byte)
- Machine name (variable)
- Machine version (4 bytes)
- Program name size (2 bytes)
- Program name (variable)

[Data Segment]
- Data segment size (4 bytes)
- Data bytes (variable)

[Code Segment]
- Code segment size (4 bytes)
- Code bytes (variable)
```

All multi-byte values use **little-endian** byte order.

## Memory Layout

When assembled, your program's memory layout is:

```
0x0000: [Data Segment]
        - First data definition at 0x0000
        - Second data definition immediately after
        - etc.

0xXXXX: [Code Segment]
        - Starts immediately after data segment
        - Instructions encoded sequentially
        - Labels resolved to absolute addresses
```

## Next Steps

- **[Syntax Reference](Syntax.md)**: Learn the assembly language syntax
- **[Register Reference](Registers.md)**: Understand available registers
- **[Instruction Set](InstructionSet.md)**: Complete instruction reference
- **[Examples](Examples.md)**: Full program examples and tutorials

## Getting Help

If you encounter issues:

1. Check error messages carefully - they include line numbers
2. Review the syntax reference for correct usage
3. Look at example programs for patterns
4. Verify your instruction operands match the requirements

## Version Information

- **Assembler Version**: 1.0.0
- **Binary Format Version**: 1.0.0
- **Target Machine**: Pendragon VM 1.0.0
- **Supported Instructions**: All 119 Pendragon VM opcodes
