# Error Messages & Debugging Guide

## Overview

This guide helps you understand and resolve errors produced by the Pendragon assembler. The assembler performs comprehensive error checking across all five compilation passes, providing detailed error messages with line and column information.

## Error Message Format

All assembler errors follow this format:

```
[PASS] Error at line X, column Y: Error message description
```

**Components**:
- **[PASS]**: Which compilation pass detected the error (LEXER, PARSER, SEMANTIC, CODEGEN, BINARY)
- **Line X**: Line number in source file (1-based)
- **Column Y**: Column number in line (1-based)
- **Error message**: Description of what went wrong

**Example**:
```
[SEMANTIC] Error at line 15, column 10: Undefined symbol 'myLabel'
```

## Error Categories by Pass

### Pass 1: Lexical Errors

Detected during tokenization of source code.

#### Unterminated String

**Error**: `Unterminated string`

**Cause**: String literal missing closing quote.

**Example**:
```assembly
DATA
    message: DB "Hello, World!
                             ^ Missing closing quote
```

**Solution**:
```assembly
DATA
    message: DB "Hello, World!"
```

---

#### Unexpected Character

**Error**: `Unexpected character`

**Cause**: Invalid character in source code.

**Example**:
```assembly
CODE
    LD AX, @100
           ^ Invalid character '@'
```

**Solution**:
```assembly
CODE
    LD AX, 100
```

---

### Pass 1: Parse Errors

Detected during syntax analysis and AST construction.

#### Expected Token

**Error**: `Expected <token>, got <token>`

**Cause**: Wrong token type at a specific position.

**Example**:
```assembly
DATA
    value DB [42]
          ^ Missing colon
```

**Solution**:
```assembly
DATA
    value: DB [42]
```

---

#### Missing Section

**Error**: `Expected DATA or CODE section`

**Cause**: File doesn't start with a section keyword.

**Example**:
```assembly
myLabel: LD AX, 10
^ No section declared
```

**Solution**:
```assembly
CODE
myLabel: LD AX, 10
```

---

#### Invalid Data Definition

**Error**: `Expected DB or DW after label in DATA section`

**Cause**: Data label not followed by type keyword.

**Example**:
```assembly
DATA
    value: [42]
           ^ Missing DB or DW
```

**Solution**:
```assembly
DATA
    value: DW [42]
```

---

#### Missing Brackets

**Error**: `Expected [ for data values`

**Cause**: Data values not enclosed in brackets.

**Example**:
```assembly
DATA
    array: DB 1, 2, 3
              ^ Missing brackets
```

**Solution**:
```assembly
DATA
    array: DB [1, 2, 3]
```

---

#### Invalid Operand

**Error**: `Invalid operand for instruction`

**Cause**: Operand syntax doesn't match instruction requirements.

**Example**:
```assembly
CODE
    LD AX, [
           ^ Incomplete operand
```

**Solution**:
```assembly
CODE
    LD AX, [value]
```

---

#### Wrong Section Order

**Error**: `CODE section cannot appear before DATA section`

**Cause**: CODE section placed before DATA section.

**Example**:
```assembly
CODE
    HALT

DATA
    value: DW [42]
```

**Solution**:
```assembly
DATA
    value: DW [42]

CODE
    HALT
```

---

### Pass 2: Semantic Errors

Detected during semantic analysis and symbol table construction.

#### Undefined Symbol

**Error**: `Undefined symbol '<name>'`

**Cause**: Reference to a label or symbol that hasn't been defined.

**Example**:
```assembly
CODE
    LD AX, [myValue]
            ^^^^^^^^ Not defined
    HALT
```

**Solution**:
```assembly
DATA
    myValue: DW [42]

CODE
    LD AX, [myValue]
    HALT
```

---

#### Duplicate Symbol

**Error**: `Duplicate symbol '<name>'`

**Cause**: Same label defined multiple times.

**Example**:
```assembly
DATA
    value: DW [10]
    value: DW [20]
    ^^^^^ Duplicate
```

**Solution**:
```assembly
DATA
    value: DW [10]
    value2: DW [20]
```

---

#### Invalid Register

**Error**: `Invalid register name '<name>'`

**Cause**: Unknown or misspelled register name.

**Example**:
```assembly
CODE
    LD AZ, 10
       ^^ Invalid register
```

**Solution**:
```assembly
CODE
    LD AX, 10
```

**Valid Registers**:
- 16-bit: `AX`, `BX`, `CX`, `DX`, `EX`
- 8-bit: `AH`, `AL`, `BH`, `BL`, `CH`, `CL`, `DH`, `DL`, `EH`, `EL`

---

#### Wrong Section for Symbol

**Error**: `Symbol '<name>' defined in wrong section`

**Cause**: Trying to use a code label in data context or vice versa.

**Example**:
```assembly
DATA
    value: DW [label]
               ^^^^^ Code label in data

CODE
label:
    HALT
```

**Solution**: Use appropriate symbol types or rethink program structure.

---

#### Forward Reference Not Resolved

**Error**: `Forward reference to '<name>' not resolved`

**Cause**: Label used before it's defined, and never defined.

**Example**:
```assembly
CODE
    JMP later
        ^^^^^ Never defined
    HALT
```

**Solution**:
```assembly
CODE
    JMP later
    HALT
later:
    ; Code here
```

---

### Pass 3-4: Code Generation Errors

Detected during intermediate representation and address resolution.

#### Unknown Instruction

**Error**: `Unknown instruction '<mnemonic>'`

**Cause**: Instruction mnemonic not recognized.

**Example**:
```assembly
CODE
    LOAD AX, 10
    ^^^^ Unknown mnemonic
```

**Solution**:
```assembly
CODE
    LD AX, 10
```

See [Instruction Set Reference](InstructionSet.md) for valid mnemonics.

---

#### Invalid Operand Count

**Error**: `Instruction '<mnemonic>' expects <n> operands, got <m>`

**Cause**: Wrong number of operands for instruction.

**Example**:
```assembly
CODE
    LD AX
    ^^^^^ Missing second operand
```

**Solution**:
```assembly
CODE
    LD AX, 10
```

---

#### Operand Type Mismatch

**Error**: `Invalid operand type for instruction`

**Cause**: Operand type doesn't match instruction requirements.

**Example**:
```assembly
CODE
    JMP 10
        ^^ JMP requires address/label, not immediate
```

**Solution**:
```assembly
CODE
start:
    JMP start
```

---

#### Value Out of Range

**Error**: `Value <value> out of range for <type>`

**Cause**: Numeric value too large for operand size.

**Example**:
```assembly
DATA
    byte_val: DB [256]
                  ^^^ Too large for byte (max 255)
```

**Solution**:
```assembly
DATA
    byte_val: DB [255]
    ; Or use word:
    word_val: DW [256]
```

**Ranges**:
- **Byte (DB)**: 0 to 255 (0x00 to 0xFF)
- **Word (DW)**: 0 to 65535 (0x0000 to 0xFFFF)

---

#### Cannot Resolve Address

**Error**: `Cannot resolve address for symbol '<name>'`

**Cause**: Expression or symbol reference couldn't be converted to address.

**Example**:
```assembly
CODE
    LD AX, [label + label2]
                    ^^^^^^ Can't add two labels
```

**Solution**:
```assembly
CODE
    LD AX, [label]
    ; Or use offset:
    LD BX, [label + 10]
```

---

### Pass 5: Binary Generation Errors

Detected during final binary file generation.

#### Binary Write Failed

**Error**: `Failed to write binary file`

**Cause**: File I/O error when writing output.

**Common Reasons**:
- No write permission
- Disk full
- Invalid output path

**Solution**: Check file permissions and disk space.

---

## Common Error Scenarios

### Scenario 1: Typo in Register Name

**Symptom**: "Invalid register name" error

**Code**:
```assembly
CODE
    LD AX, 100
    ADD AXX, 50
        ^^^ Typo
```

**Error**:
```
[SEMANTIC] Error at line 3, column 9: Invalid register name 'AXX'
```

**Fix**:
```assembly
CODE
    LD AX, 100
    ADD AX, 50
```

---

### Scenario 2: Forgetting Colon on Label

**Symptom**: Parser expects different token

**Code**:
```assembly
DATA
    myValue DW [42]
            ^^ Missing colon
```

**Error**:
```
[PARSER] Error at line 2, column 13: Expected ':', got 'DW'
```

**Fix**:
```assembly
DATA
    myValue: DW [42]
```

---

### Scenario 3: Using Undefined Label

**Symptom**: "Undefined symbol" error

**Code**:
```assembly
CODE
    LD AX, [data_value]
            ^^^^^^^^^^ Not defined
    HALT
```

**Error**:
```
[SEMANTIC] Error at line 2, column 13: Undefined symbol 'data_value'
```

**Fix**:
```assembly
DATA
    data_value: DW [100]

CODE
    LD AX, [data_value]
    HALT
```

---

### Scenario 4: Wrong Data Type

**Symptom**: "Value out of range" error

**Code**:
```assembly
DATA
    small: DB [1000]
               ^^^^ Too large
```

**Error**:
```
[CODEGEN] Error at line 2, column 16: Value 1000 out of range for byte (max 255)
```

**Fix**:
```assembly
DATA
    small: DW [1000]
    ; Or reduce value:
    small2: DB [255]
```

---

### Scenario 5: Missing Brackets in Data

**Symptom**: Parser expects brackets

**Code**:
```assembly
DATA
    array: DB 1, 2, 3
              ^ Missing brackets
```

**Error**:
```
[PARSER] Error at line 2, column 15: Expected '[', got NUMBER
```

**Fix**:
```assembly
DATA
    array: DB [1, 2, 3]
```

---

### Scenario 6: Duplicate Labels

**Symptom**: "Duplicate symbol" error

**Code**:
```assembly
CODE
start:
    LD AX, 10
start:
    HALT
```

**Error**:
```
[SEMANTIC] Error at line 4, column 1: Duplicate symbol 'start'
```

**Fix**:
```assembly
CODE
start:
    LD AX, 10
end:
    HALT
```

---

### Scenario 7: Wrong Number of Operands

**Symptom**: "expects N operands" error

**Code**:
```assembly
CODE
    LD AX
```

**Error**:
```
[PARSER] Error at line 2, column 10: Expected operand after instruction
```

**Fix**:
```assembly
CODE
    LD AX, 10
```

---

### Scenario 8: Invalid Expression

**Symptom**: "Cannot resolve address" error

**Code**:
```assembly
DATA
    val1: DW [10]
    val2: DW [20]

CODE
    LD AX, [val1 + val2]
                   ^^^^ Can't add labels
```

**Error**:
```
[CODEGEN] Error at line 6, column 20: Cannot add two label addresses
```

**Fix**:
```assembly
CODE
    LD AX, [val1]
    ADD AX, [val2]
```

---

### Scenario 9: Unterminated String

**Symptom**: "Unterminated string" error

**Code**:
```assembly
DATA
    text: DB "Hello World
                          ^ Missing quote
```

**Error**:
```
[LEXER] Error at line 2, column 15: Unterminated string
```

**Fix**:
```assembly
DATA
    text: DB "Hello World"
```

---

### Scenario 10: Section Order

**Symptom**: "CODE section cannot appear before DATA" error

**Code**:
```assembly
CODE
    HALT

DATA
    value: DW [42]
```

**Error**:
```
[PARSER] Error at line 4, column 1: CODE section must appear after DATA section
```

**Fix**:
```assembly
DATA
    value: DW [42]

CODE
    HALT
```

---

## Debugging Strategies

### 1. Read Error Messages Carefully

Error messages include:
- Exact line and column numbers
- Description of what's wrong
- What was expected vs what was found

**Strategy**: Go to the specified line and column, read the surrounding code.

---

### 2. Check Previous Lines

Sometimes errors are caused by issues on earlier lines.

**Example**:
```assembly
DATA
    value1: DW [10]
    value2 DW [20]      ; Missing colon
           ^^
    value3: DW [30]

CODE
    LD AX, [value3]     ; This might error if parser is confused
```

**Strategy**: If an error doesn't make sense, check the few lines above it.

---

### 3. Verify Symbol Definitions

Keep track of where symbols are defined:
- DATA labels in DATA section
- CODE labels in CODE section

**Strategy**: Search for symbol definition before its use.

---

### 4. Check Register Names

Common typos:
- `AXX` instead of `AX`
- `AX1` instead of `AX`
- `A` instead of `AX`

**Strategy**: Refer to [Register Reference](Registers.md) for valid names.

---

### 5. Validate Data Types

Ensure values fit in their declared type:
- DB: 0-255
- DW: 0-65535

**Strategy**: Use hex calculator or check ranges.

---

### 6. Test in Isolation

If a large program fails, create a minimal test case:

**Original (failing)**:
```assembly
DATA
    ; 50 lines of data definitions
    
CODE
    ; 200 lines of code
    ; Error somewhere in here
```

**Minimal Test**:
```assembly
DATA
    test: DW [42]

CODE
    LD AX, [test]
    HALT
```

**Strategy**: Gradually add code back until error reappears.

---

### 7. Comment Out Sections

Temporarily disable code to isolate errors:

```assembly
CODE
    LD AX, 10
    ; ADD AX, [undefined_label]  ; Comment out problem line
    HALT
```

**Strategy**: Uncomment sections one at a time.

---

### 8. Use Simple Test Programs

Verify basic functionality works:

**Test 1: Empty Program**
```assembly
CODE
    HALT
```

**Test 2: Simple Data**
```assembly
DATA
    value: DW [42]

CODE
    HALT
```

**Test 3: Simple Load**
```assembly
DATA
    value: DW [42]

CODE
    LD AX, [value]
    HALT
```

**Strategy**: Build complexity incrementally.

---

### 9. Check File Format

Ensure:
- File is UTF-8 or ASCII
- Line endings are consistent (LF or CRLF)
- No binary characters

**Strategy**: Open in hex editor if text editor shows strange characters.

---

### 10. Verify Output Path

Ensure output file path is writable:

```bash
# Check directory exists
ls -la $(dirname output.bin)

# Check write permission
touch output.bin && rm output.bin
```

---

## Error Reference Table

| Error Type | Pass | Common Causes | Quick Fix |
|------------|------|---------------|-----------|
| Unterminated string | Lexer | Missing quote | Add closing `"` |
| Unexpected character | Lexer | Invalid symbol | Remove/replace character |
| Expected token | Parser | Missing syntax element | Add missing token |
| Missing section | Parser | No DATA/CODE | Add section keyword |
| Invalid data definition | Parser | Wrong syntax | Use `label: DB/DW [...]` |
| Missing brackets | Parser | Forgot `[` `]` | Wrap data values in brackets |
| Wrong section order | Parser | CODE before DATA | Move DATA first |
| Undefined symbol | Semantic | Label not defined | Define label or fix typo |
| Duplicate symbol | Semantic | Label defined twice | Rename one label |
| Invalid register | Semantic | Wrong register name | Use valid register (AX-EX) |
| Unknown instruction | Codegen | Wrong mnemonic | Check instruction set |
| Invalid operand count | Codegen | Wrong arg count | Match instruction signature |
| Value out of range | Codegen | Number too large | Reduce value or use larger type |
| Cannot resolve address | Codegen | Invalid expression | Simplify expression |
| Binary write failed | Binary | File I/O error | Check permissions/path |

---

## Preventive Measures

### 1. Use Consistent Naming

```assembly
; Good: Consistent style
DATA
    player_score: DW [0]
    player_health: DW [100]
    player_level: DW [1]

; Avoid: Inconsistent
DATA
    SCORE: DW [0]
    health_player: DW [100]
    lvl: DW [1]
```

---

### 2. Comment Complex Code

```assembly
CODE
    ; Calculate offset: row * width + col
    LD AX, BX           ; AX = row
    MUL AX, 5           ; AX = row * 5 (width)
    ADD AX, CX          ; AX = row * 5 + col
    LD AL, [matrix + AX]
```

---

### 3. Organize Sections

```assembly
DATA
    ; === Constants ===
    MAX_HEALTH: DW [100]
    
    ; === Player State ===
    player_x: DW [0]
    player_y: DW [0]
    
    ; === Game State ===
    score: DW [0]

CODE
    ; === Initialization ===
init:
    LD AX, 0
    
    ; === Main Loop ===
main_loop:
    ; ...
```

---

### 4. Define Before Use

```assembly
; Good: Define first
DATA
    value: DW [42]

CODE
    LD AX, [value]

; Avoid: Use before definition (though forward refs work)
CODE
    LD AX, [value]
    
DATA
    value: DW [42]
```

---

### 5. Validate Ranges

```assembly
DATA
    ; Bad: Value out of range
    ; byte_val: DB [256]
    
    ; Good: In range
    byte_val: DB [255]
    
    ; Or use appropriate type
    word_val: DW [256]
```

---

## Getting Help

When asking for help with errors:

1. **Include the error message**: Full text with line/column
2. **Show relevant code**: 5-10 lines around error location
3. **Describe what you're trying to do**: Expected behavior
4. **Mention what you've tried**: Attempted fixes

**Example Help Request**:
```
I'm getting this error:
[SEMANTIC] Error at line 15, column 10: Undefined symbol 'counter'

Code around line 15:
13: CODE
14: loop:
15:     DEC counter
16:     JPNZ loop
17:     HALT

I'm trying to create a countdown loop. I thought counter would be 
defined from my earlier code but can't find where. I've tried adding 
it to DATA section but still getting errors.
```

---

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Correct syntax rules
- **[Instruction Set](InstructionSet.md)**: Valid instructions
- **[Register Reference](Registers.md)**: Valid register names
- **[Examples](Examples.md)**: Working code examples
