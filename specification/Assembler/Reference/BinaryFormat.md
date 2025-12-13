# Binary Format Specification

## Overview

The Pendragon assembler generates binary files in a specific format designed for the BinaryLoader V1.0.0. This document describes the complete binary file structure, byte-level layout, and encoding rules.

## File Format Version

**Binary Format Version**: 1.0.0  
**Compatible with**: BinaryLoader V1.0.0  
**Target Machine**: Pendragon VM 1.0.0

## Binary File Structure

A binary file consists of four main sections:

1. **Header**: Metadata about the file format, machine, and program
2. **Data Segment**: Initialized data from the DATA section
3. **Code Segment**: Executable instructions from the CODE section

### Overall Layout

```
┌─────────────────────────────────────────┐
│           HEADER                         │
│  - Format information                    │
│  - Machine identification                │
│  - Program name                          │
├─────────────────────────────────────────┤
│         DATA SEGMENT                     │
│  - Size (4 bytes)                        │
│  - Data bytes                            │
├─────────────────────────────────────────┤
│         CODE SEGMENT                     │
│  - Size (4 bytes)                        │
│  - Instruction bytes                     │
└─────────────────────────────────────────┘
```

## Byte Order (Endianness)

**All multi-byte values are stored in little-endian format** (least significant byte first).

**Examples**:
- `0x1234` (16-bit) → Bytes: `[0x34, 0x12]`
- `0x12345678` (32-bit) → Bytes: `[0x78, 0x56, 0x34, 0x12]`

## Header Format

The header contains metadata about the binary file and program.

### Header Structure

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 2 bytes | Header Size | Total size of header in bytes |
| 0x02 | 1 byte | Header Version Major | Major version (1) |
| 0x03 | 1 byte | Header Version Minor | Minor version (0) |
| 0x04 | 2 bytes | Header Version Revision | Revision number (0) |
| 0x06 | 1 byte | Machine Name Length | Length of machine name string |
| 0x07 | N bytes | Machine Name | ASCII string "Pendragon" |
| 0x07+N | 1 byte | Machine Version Major | Major version (1) |
| 0x08+N | 1 byte | Machine Version Minor | Minor version (0) |
| 0x09+N | 2 bytes | Machine Version Revision | Revision number (0) |
| 0x0B+N | 2 bytes | Program Name Length | Length of program name string |
| 0x0D+N | M bytes | Program Name | ASCII program name (max 32 chars) |

### Header Version

**Format**: Major.Minor.Revision

**Current**: 1.0.0
- **Major (1 byte)**: 1 - Incompatible format changes
- **Minor (1 byte)**: 0 - Compatible feature additions
- **Revision (2 bytes)**: 0 - Bug fixes

### Machine Identification

**Machine Name**: "Pendragon" (9 ASCII bytes)
- Length: 0x09
- Bytes: `[0x50, 0x65, 0x6E, 0x64, 0x72, 0x61, 0x67, 0x6F, 0x6E]`

**Machine Version**: 1.0.0
- Major: 1
- Minor: 0
- Revision: 0

### Program Name

**Length**: 2 bytes (little-endian)
**Maximum Length**: 32 characters
**Encoding**: ASCII
**Truncation**: Names longer than 32 characters are truncated

**Example**: Program name "HelloWorld"
- Length bytes: `[0x0A, 0x00]` (10 in little-endian)
- String bytes: `[0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x57, 0x6F, 0x72, 0x6C, 0x64]`

### Header Size Calculation

The header size field indicates the total size of the header in bytes:

```
Header Size = 2 (header size field)
            + 4 (header version: major, minor, revision_hi, revision_lo)
            + 1 (machine name length)
            + N (machine name string)
            + 4 (machine version)
            + 2 (program name length)
            + M (program name string)
```

**Example**: For machine name "Pendragon" (9 bytes) and program name "Test" (4 bytes):
```
Header Size = 2 + 4 + 1 + 9 + 4 + 2 + 4 = 26 bytes (0x1A)
```

## Data Segment

The data segment contains all initialized data from the DATA section of the assembly program.

### Data Segment Structure

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 bytes | Data Size | Size of data segment in bytes |
| 0x04 | N bytes | Data Bytes | Raw data bytes |

### Data Segment Size

**Type**: 32-bit unsigned integer (little-endian)
**Range**: 0 to 4,294,967,295 bytes

**Example**: Data size of 42 bytes
```
Bytes: [0x2A, 0x00, 0x00, 0x00]
```

### Data Layout

Data is laid out sequentially in the order defined in the DATA section:

**Assembly**:
```assembly
DATA
    byte1: DB [0x42]
    word1: DW [0x1234]
    array: DB [10, 20, 30]
```

**Binary Data Segment**:
```
Size: 6 bytes
Bytes: [0x42, 0x34, 0x12, 0x0A, 0x14, 0x1E]
       ^^^^  ^^^^^^^^^^  ^^^^^^^^^^^^^^^^^
       byte1    word1         array
```

### Empty Data Segment

If the assembly program has no DATA section, the data segment size is 0:

```
Size bytes: [0x00, 0x00, 0x00, 0x00]
(No data bytes follow)
```

## Code Segment

The code segment contains all executable instructions from the CODE section.

### Code Segment Structure

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 bytes | Code Size | Size of code segment in bytes |
| 0x04 | N bytes | Code Bytes | Machine code instructions |

### Code Segment Size

**Type**: 32-bit unsigned integer (little-endian)
**Range**: 0 to 4,294,967,295 bytes

**Example**: Code size of 100 bytes
```
Bytes: [0x64, 0x00, 0x00, 0x00]
```

### Instruction Encoding

Each instruction is encoded as:
1. **Opcode** (1 byte)
2. **Operands** (0-4 bytes depending on instruction)

**Encoding Rules**:
- Opcodes are single bytes (0x00-0x7F)
- Register operands are single bytes (AX=0x00, BX=0x01, CX=0x02, DX=0x03, EX=0x04)
- Immediate byte values are single bytes
- Immediate word values are 2 bytes (little-endian)
- Addresses are 2 bytes (little-endian)

### Code Layout Example

**Assembly**:
```assembly
CODE
start:
    LD AX, 100
    HALT
```

**Binary Code Segment**:
```
Size: 5 bytes
Bytes: [0x02, 0x00, 0x64, 0x00, 0x01]
       ^^^^  ^^^^  ^^^^^^^^^^  ^^^^
      LD(1)  AX      100      HALT
```

**Breakdown**:
- `0x02`: LD opcode (variant 1: immediate)
- `0x00`: AX register
- `0x64, 0x00`: 100 in little-endian (0x0064)
- `0x01`: HALT opcode

### Labels in Code

Labels do not generate any bytes in the binary. They only affect address resolution.

**Assembly**:
```assembly
CODE
start:
    NOP
loop:
    HALT
```

**Binary**: Only the actual instructions:
```
Bytes: [0x00, 0x01]
       ^^^^  ^^^^
       NOP   HALT
```

## Complete Binary Example

### Example Program

```assembly
DATA
    value: DW [0x1234]

CODE
    LD AX, [value]
    HALT
```

### Binary Breakdown

#### Header (26 bytes)

```
Offset  Value       Description
------  ----------  -----------
0x00    1A 00       Header size: 26 bytes
0x02    01          Header version major: 1
0x03    00          Header version minor: 0
0x04    00 00       Header version revision: 0
0x06    09          Machine name length: 9
0x07-0F 50 65 ...   Machine name: "Pendragon"
0x10    01          Machine version major: 1
0x11    00          Machine version minor: 0
0x12    00 00       Machine version revision: 0
0x14    04 00       Program name length: 4
0x16-19 54 65 73... Program name: "Test"
```

#### Data Segment (6 bytes)

```
Offset  Value       Description
------  ----------  -----------
0x1A    02 00 00 00 Data size: 2 bytes
0x1E    34 12       value: 0x1234 (little-endian)
```

#### Code Segment (9 bytes)

```
Offset  Value       Description
------  ----------  -----------
0x20    05 00 00 00 Code size: 5 bytes
0x24    09          LD opcode (variant: load from address)
0x25    00          AX register
0x26    00 00       Address: 0x0000 (value's address)
0x28    01          HALT opcode
```

#### Complete Hex Dump

```
0000: 1A 00 01 00 00 00 09 50 65 6E 64 72 61 67 6F 6E  |.......Pendragon|
0010: 01 00 00 00 04 00 54 65 73 74 02 00 00 00 34 12  |......Test....4.|
0020: 05 00 00 00 09 00 00 00 01                       |.........|
```

## Memory Layout at Runtime

When the binary is loaded into the Pendragon VM:

### Data Segment Placement

Data is loaded starting at address **0x0000**.

**Example**:
```assembly
DATA
    byte1: DB [0x42]      ; Address: 0x0000
    word1: DW [0x1234]    ; Address: 0x0001
```

**Memory Layout**:
```
Address  Value
-------  -----
0x0000   0x42
0x0001   0x34
0x0002   0x12
```

### Code Segment Placement

Code is loaded immediately after the data segment.

**Example**: If data segment is 10 bytes:
```
Data: 0x0000 - 0x0009 (10 bytes)
Code: 0x000A onwards
```

### Program Entry Point

Execution begins at the **first instruction** in the code segment.

## Instruction Encoding Details

### No-Operand Instructions

**Format**: Opcode only (1 byte)

**Examples**:
```
NOP  → [0x00]
HALT → [0x01]
RET  → [0x28]
FLSH → [0x1A]
```

### Single Register Instructions

**Format**: Opcode + Register (2 bytes)

**Examples**:
```
PUSH AX  → [0x10, 0x00]
POP BX   → [0x13, 0x01]
INC CX   → [0x6A, 0x02]
DEC DX   → [0x6B, 0x03]
```

### Register + Immediate (Word)

**Format**: Opcode + Register + Word (4 bytes)

**Examples**:
```
LD AX, 100     → [0x02, 0x00, 0x64, 0x00]
ADD AX, 50     → [0x29, 0x32, 0x00]
SUB AX, 0x1234 → [0x2E, 0x34, 0x12]
```

### Register + Immediate (Byte)

**Format**: Opcode + Register + Byte (3 bytes)

**Examples**:
```
LDH AH, 0x12  → [0x05, 0x00, 0x12]
LDL AL, 0x34  → [0x07, 0x00, 0x34]
ADB AX, 10    → [0x2B, 0x0A]
```

### Register + Register

**Format**: Opcode + Register1 + Register2 (3 bytes)

**Examples**:
```
LD AX, BX    → [0x03, 0x00, 0x01]
SWP CX, DX   → [0x04, 0x02, 0x03]
ADD AX, BX   → [0x2A, 0x01]
CMP AX, BX   → [0x6C, 0x00, 0x01]
```

### Register + Address

**Format**: Opcode + Register + Address (4 bytes)

**Examples**:
```
LD AX, [0x0010]   → [0x09, 0x00, 0x10, 0x00]
LD [0x0020], BX   → [0x0D, 0x20, 0x00, 0x01]
LDAB CX, [0x0005] → [0x0A, 0x02, 0x05, 0x00]
```

### Jump/Call Instructions

**Format**: Opcode + Address (3 bytes)

**Examples**:
```
JMP 0x0100   → [0x1E, 0x00, 0x01]
JPZ 0x0050   → [0x1F, 0x50, 0x00]
JPNZ 0x0080  → [0x20, 0x80, 0x00]
CALL 0x0200  → [0x27, 0x00, 0x02]
```

### Immediate Stack Operations

**Format**: Opcode + Immediate (3 bytes for word, 2 bytes for byte)

**Examples**:
```
PUSHW 0x1234 → [0x75, 0x34, 0x12]
PUSHB 0x42   → [0x76, 0x42]
```

## File Size Calculation

Total binary file size:

```
File Size = Header Size
          + 4 (data segment size field)
          + Data Segment Size
          + 4 (code segment size field)
          + Code Segment Size
```

**Example**:
- Header: 26 bytes
- Data segment size field: 4 bytes
- Data segment: 10 bytes
- Code segment size field: 4 bytes
- Code segment: 20 bytes
- **Total**: 26 + 4 + 10 + 4 + 20 = **64 bytes**

## Binary File Tools

### Hexdump Viewing

View binary file in hexadecimal:

```bash
# Linux/macOS
hexdump -C program.bin

# With xxd
xxd program.bin
```

**Example Output**:
```
00000000: 1a 00 01 00 00 00 09 50 65 6e 64 72 61 67 6f 6e  |.......Pendragon|
00000010: 01 00 00 00 04 00 54 65 73 74 02 00 00 00 34 12  |......Test....4.|
00000020: 05 00 00 00 09 00 00 00 01                       |.........|
```

### Binary Analysis

Extract header information:

```bash
# First 2 bytes (header size)
od -An -t u2 -N 2 program.bin

# Machine name (skip to offset 7, read 9 bytes)
dd if=program.bin bs=1 skip=7 count=9 2>/dev/null
```

## Validation and Verification

### Header Validation

1. **Header size**: Must be ≥ 22 bytes (minimum header)
2. **Header version**: Should be 1.0.0
3. **Machine name**: Should be "Pendragon"
4. **Machine version**: Should match VM version
5. **Program name**: Length must match actual string length

### Segment Validation

1. **Data segment size**: Must not exceed file size
2. **Code segment size**: Must not exceed file size
3. **Total size**: Header + data (size + bytes) + code (size + bytes) = file size

### Checksum (Future Enhancement)

Currently, no checksum is included. Future versions may add:
- CRC32 of entire file
- SHA-256 hash
- Per-segment checksums

## Compatibility Notes

### Version Compatibility

**BinaryLoader 1.0.0** can load:
- Binary format 1.0.x (minor/revision changes are compatible)

**BinaryLoader 1.x.x** may load:
- Binary format 1.x.x (forward compatible within major version)

### Platform Independence

The binary format is platform-independent:
- Little-endian encoding on all platforms
- No platform-specific padding
- ASCII string encoding
- Fixed-size fields

### Future Extensions

Reserved for future use:
- Opcodes 0x80-0xFF (extended instruction sets)
- Additional header fields (backward compatible)
- Optional metadata sections
- Debug information sections
- Symbol table sections

## Binary Generation from Assembler

The assembler generates binaries through the `BinaryWriter` class:

```cpp
// Pseudo-code of binary generation
1. Calculate header size
2. Write header:
   - Header size
   - Version information
   - Machine identification
   - Program name
3. Collect data blocks from code graph
4. Write data segment size
5. Write data segment bytes
6. Collect instruction nodes from code graph
7. Encode each instruction
8. Write code segment size
9. Write code segment bytes
```

## Common Binary Patterns

### Minimal Program

**Assembly**:
```assembly
CODE
    HALT
```

**Binary** (35 bytes):
- Header: 26 bytes
- Data segment: 4 bytes (size=0) + 0 bytes
- Code segment: 4 bytes (size=1) + 1 byte (HALT)

### Data Only

**Assembly**:
```assembly
DATA
    values: DW [1, 2, 3, 4, 5]
```

**Binary** (40 bytes):
- Header: 26 bytes
- Data segment: 4 bytes (size=10) + 10 bytes
- Code segment: 4 bytes (size=0) + 0 bytes

### Hello World Pattern

Typical structure:
- Header: ~30 bytes
- Data: String + variables (~20-50 bytes)
- Code: Loop + I/O (~30-100 bytes)
- **Total**: ~80-180 bytes

## Error Conditions

### Invalid Binary Files

**Truncated Header**:
- File size < header size field
- Missing machine name or program name

**Truncated Segments**:
- File size < header + data size
- File size < header + data + code size

**Invalid Values**:
- Header size = 0
- Machine name length = 0
- Segment sizes exceed reasonable limits

### Loader Behavior

On invalid binary:
1. Validate header size
2. Validate version compatibility
3. Validate machine name
4. Validate segment sizes
5. Reject if any check fails

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Assembly language syntax
- **[Instruction Set](InstructionSet.md)**: Instruction encoding reference
- **[Examples](Examples.md)**: Complete program examples with binary output
