# Expressions & Addressing Modes

## Overview

The Pendragon assembler supports expressions for calculating memory addresses and values at assembly time. This document describes the expression syntax, evaluation rules, and addressing modes available.

## Expression Basics

Expressions allow you to compute addresses and values using labels, constants, and operators. Most expressions are evaluated during assembly (compile-time), though some parts (like register offsets) are evaluated at runtime.

### Expression Components

1. **Labels**: Symbol names representing addresses
2. **Numeric Literals**: Decimal or hexadecimal constants
3. **Registers**: Register names for runtime address calculations
4. **Operators**: Addition (+) and subtraction (-)

### Simple Expressions

```assembly
DATA
    buffer: DB [0, 0, 0, 0, 0]

CODE
    LDA AX, buffer          ; Direct label reference
    LDA BX, (buffer + 2)    ; Label + offset
    LD CX, (buffer + 4)     ; Address calculation
```

## Addressing Modes

### 1. Immediate Addressing

Values are embedded directly in the instruction.

**Syntax**: `instruction operand, value`

**Examples**:
```assembly
CODE
    LD AX, 42               ; Load constant 42
    LD BX, 0x1234           ; Load hex constant
    ADD AX, 100             ; Add constant 100
    PUSHW 0xFF              ; Push constant to stack
```

**Characteristics**:
- Fastest execution (no memory access)
- Value is part of instruction encoding
- Limited to operand size (8 or 16 bits)

**Use Cases**:
- Loading constants
- Initializing registers
- Immediate arithmetic

---

### 2. Register Addressing

Operations use register contents directly.

**Syntax**: `instruction dest_reg, source_reg`

**Examples**:
```assembly
CODE
    LD AX, BX               ; Copy BX to AX
    ADD AX, CX              ; Add CX to AX
    SWP DX, EX              ; Swap DX and EX
    MUL AX, BX              ; Multiply AX by BX
```

**Characteristics**:
- Very fast (register-to-register)
- No memory access required
- Limited to available registers

**Use Cases**:
- Data movement between registers
- Arithmetic with register operands
- Temporary storage

---

### 3. Direct Addressing

Access memory using a compile-time known address.

**Syntax**: `LDA operand, label` or `LDA operand, (expression)`

**Examples**:
```assembly
DATA
    value: DW [0x1234]
    byte_val: DB [0x42]

CODE
    LDA AX, value           ; Load word from address
    LDAB BX, byte_val       ; Load byte from address
    LDA value, CX           ; Store word to address
```

**Characteristics**:
- Address computed at assembly time
- Single memory access
- Fixed location

**Use Cases**:
- Global variables
- Static data access
- Known memory locations

---

### 4. Indirect Addressing

Access memory using an address stored in a register.

**Syntax**: `instruction operand, (register)`

**Examples**:
```assembly
DATA
    array: DB [10, 20, 30, 40, 50]

CODE
    LD BX, array            ; BX = address of array
    LDAB AL, BX             ; Load byte from address in BX
    INC BX                  ; Move to next element
    LDAB AL, BX             ; Load next byte
```

**Characteristics**:
- Address computed at runtime
- Enables dynamic memory access
- Requires pointer in register

**Use Cases**:
- Array traversal
- Pointer dereferencing
- Dynamic data structures

**Note**: Not all instructions support indirect addressing. See [Instruction Set Reference](InstructionSet.md) for details.

---

### 5. Indexed Addressing (Expression-Based)

Combine label with offset for array element access.

**Syntax**: `LDA operand, (label + offset)` or sugar syntax `LDA operand, label[offset]`

**Examples**:
```assembly
DATA
    array: DW [100, 200, 300, 400, 500]

CODE
    LDA AX, (array + 0)     ; First element (array[0])
    LDA BX, (array + 2)     ; Second element (array[1]) - words are 2 bytes
    LDA CX, (array + 4)     ; Third element (array[2])
    
    ; Or using sugar syntax
    LDA AX, array[0]        ; Same as LDA AX, (array + 0)
    LDA BX, array[2]        ; Same as LDA BX, (array + 2)
```

**Characteristics**:
- Offset computed at assembly time
- Single memory access
- Fixed index

**Use Cases**:
- Known array indices
- Structure field access
- Static offsets

---

### 6. Register-Indexed Addressing

Combine label with register for dynamic indexing.

**Syntax**: `LDA operand, (label + register)` or sugar syntax `LDA operand, label[register]`

**Examples**:
```assembly
DATA
    array: DB [10, 20, 30, 40, 50]

CODE
    LD CX, 0                ; Index = 0
    LDAB AL, (array + CX)   ; Load array[0]
    
    LD CX, 3                ; Index = 3
    LDAB AL, (array + CX)   ; Load array[3]
    
    INC CX                  ; Index = 4
    LDAB AL, (array + CX)   ; Load array[4]
    
    ; Or using sugar syntax
    LDAB AL, array[CX]      ; Same as LDAB AL, (array + CX)
```

**Characteristics**:
- Base address at assembly time
- Offset computed at runtime
- Enables dynamic array access

**Use Cases**:
- Array iteration
- Variable indexing
- Loop-based access

---

### 7. Array Index Sugar Syntax

Syntactic sugar for array indexing that automatically converts to memory access.

**Syntax**: `instruction operand, label[expression]`

**Examples**:
```assembly
DATA
    bytes: DB [10, 20, 30, 40, 50]
    words: DW [100, 200, 300, 400]

CODE
    ; Register index (PRIMARY use case for sugar syntax)
    LD CX, 4
    LD DL, bytes[CX]        ; → LDAB DL, (bytes + CX)
    LD BX, words[DX]        ; → LDA BX, (words + DX)
    
    ; Complex expressions with registers (GOOD use)
    LD BX, 2
    LD AL, bytes[BX + 1]    ; → LDAB AL, (bytes + BX + 1)
    LD AX, words[CX - 2]    ; → LDA AX, (words + CX - 2)
    
    ; Constant index (sugar works but explicit is clearer)
    LDAB AL, (bytes + 0)    ; Preferred for constants
    LDAB BL, (bytes + 3)    ; Preferred for constants
    LDA AX, (words + 2)     ; Preferred for constants
```

**Characteristics**:
- **Automatic conversion**: `LD reg, label[expr]` becomes `LDA` or `LDAB` based on register size
- **Register size detection**: 8-bit registers (AL, BL, etc.) → LDAB, 16-bit (AX, BX, etc.) → LDA
- **More readable**: Clearly indicates array indexing intent
- **Same behavior**: Identical to explicit parentheses syntax
- **Works with all instructions**: Converts LD appropriately

**Conversion Rules**:
1. If instruction is `LD` with sugar syntax → changes to `LDA` (16-bit) or `LDAB` (8-bit) based on destination register
2. Square brackets `[]` → parentheses `()`
3. Expression inside preserved exactly
4. Register size automatically detected (AL/BL/etc. → LDAB, AX/BX/etc. → LDA)

**Comparison**:
```assembly
; These are exactly equivalent (8-bit register):
LD AL, buffer[BX]           ; Sugar syntax (converts to LDAB)
LDAB AL, (buffer + BX)      ; Explicit syntax (what it becomes)

; These are exactly equivalent (16-bit register):
LD AX, array[CX]            ; Sugar syntax (converts to LDA)
LDA AX, (array + CX)        ; Explicit syntax (what it becomes)

; These are exactly equivalent (8-bit with expression):
LD DL, data[BX + 2]         ; Sugar with complex expression
LDAB DL, (data + BX + 2)    ; Explicit equivalent
```

**When to Use Sugar Syntax**:
- ✅ Array element access **with register indices**
- ✅ Dynamic indexing (index value in a register)
- ✅ Loop-based array traversal
- ✅ When the array indexing intent is clear

**When to Use Explicit Parentheses**:
- ✅ **Constant offsets** (clearer: `(array + 5)` vs `array[5]`)
- ✅ Address arithmetic (not array access)
- ✅ When loading addresses (LD instruction for address calculation)
- ✅ Complex pointer manipulation
- ✅ When sugar syntax seems less clear

**Best Practice**: Use sugar syntax primarily for **register-based indices** where it clearly shows array indexing. For constant offsets, explicit parentheses syntax is often clearer.

---

### 6. Inline Data Addressing

Create anonymous data blocks directly in instructions.

**Syntax**: `LD register, DB/DW data`

**Examples**:
```assembly
CODE
    ; Inline word array
    LD AX, DW [100, 200, 300, 400]
    LDA BX, (AX + 2)           ; Access third word (300)
    
    ; Inline byte array
    LD CX, DB [5, 10, 15, 20, 25]
    LDAB DL, (CX + 3)          ; Access fourth byte (20)
    
    ; Inline string
    LD DX, DB "Error: File not found"
    LDAB AL, DX                ; Load first character
```

**Characteristics**:
- **Anonymous Creation**: Assembler generates unique label (`__anon_0`, `__anon_1`, etc.)
- **Data Segment Placement**: Data stored in data segment, not inline in code
- **Address Loading**: Instruction receives address of data block
- **Single Use**: Intended for one-time access; duplicates create separate blocks
- **No Deduplication**: Identical inline data creates multiple copies

**Transformation**:
```assembly
; Source code:
LD AX, DW [1, 2, 3]

; Assembler generates:
DATA (internal)
    __anon_0: DW [1, 2, 3]
CODE
    LD AX, __anon_0
```

**When to Use**:
- ✅ One-time constant tables
- ✅ Function-local string literals  
- ✅ Small lookup tables used once
- ✅ Quick prototyping

**When to Use Labeled Data Instead**:
- ✅ Data accessed multiple times
- ✅ Data shared between functions
- ✅ Modifiable data
- ✅ Large data structures

**Common Patterns**:
```assembly
; Pattern 1: Inline lookup table
process_value:
    LD BX, DW [1, 4, 9, 16, 25]    ; Squares table
    LDA AX, (BX + CX)              ; Lookup by index
    RET

; Pattern 2: Error messages
error_handler:
    CMP AL, 1
    JPZ file_error
    LD AX, DB "Unknown error"      ; Different for each error
    RET
    
file_error:
    LD AX, DB "File not found"
    RET

; Pattern 3: Configuration data
init_device:
    LD AX, DB [0x80, 0x40, 0x20]   ; Device init sequence
    CALL send_config
    RET
```

**Important Notes**:
1. Each occurrence creates a new block (no automatic deduplication)
2. Data cannot be referenced from other locations (no label)
3. Suitable for true constants only (cannot be modified by label)
4. Memory overhead if same data defined multiple times

---

## Expression Syntax

### Arithmetic Operators

#### Addition (+)

Adds two values together.

**Syntax**: `expression + expression`

**Examples**:
```assembly
DATA
    base: DW [0x1000]
    
CODE
    LDA AX, (base + 10)     ; base address + 10 bytes
    LDA BX, (base + CX)     ; base address + CX bytes
    LD DX, (base + 100)     ; Address of (base + 100)
```

**Rules**:
- Can add label + literal
- Can add label + register
- Cannot add two labels
- Cannot add two registers

---

#### Subtraction (-)

Subtracts second value from first.

**Syntax**: `expression - expression`

**Examples**:
```assembly
DATA
    end_marker: DB [0xFF]
    buffer: DB [10, 20, 30]
    
CODE
    LDA AX, (buffer + 10 - 2)   ; (buffer + 10) - 2 = buffer + 8
    LD BX, (end_marker - 1)     ; Address before end_marker
```

**Rules**:
- Can subtract literal from label
- Can subtract literal from (label + register)
- Cannot subtract label from label
- Cannot subtract register from anything

---

### Operator Precedence

The assembler evaluates expressions left to right with these rules:

1. **No Parentheses**: Expressions don't support explicit parentheses
2. **Left-to-Right**: Operations evaluated in order
3. **Simple Algebra**: Only addition and subtraction

**Examples**:
```assembly
CODE
    LDA AX, (buffer + 4 + 2)    ; Evaluates as (buffer + 4) + 2 = buffer + 6
    LDA BX, (buffer + 10 - 3)   ; Evaluates as (buffer + 10) - 3 = buffer + 7
```

---

### Expression Evaluation

#### Compile-Time Evaluation

Most expression parts are evaluated during assembly:

```assembly
DATA
    array: DB [1, 2, 3, 4, 5]

CODE
    LDA AX, (array + 3)         ; Address computed at assembly time
                                 ; Becomes: LDA AX, (0x0003) (if array is at 0x0000)
```

**What's Evaluated at Compile-Time**:
- Label addresses
- Numeric literals
- Arithmetic on constants
- Label + constant expressions

---

#### Runtime Evaluation

Register-based offsets are evaluated at runtime:

```assembly
DATA
    array: DB [1, 2, 3, 4, 5]

CODE
    LD CX, 3                    ; Index = 3
    LDAB AX, (array + CX)       ; CX added to base address at runtime
                                 ; Actual address = array_address + CX_value
```

**What's Evaluated at Runtime**:
- Register values
- Register-based offsets
- Indirect addressing

---

## Advanced Expression Patterns

### Array Element Access

#### Byte Arrays

Elements are 1 byte apart:

```assembly
DATA
    bytes: DB [10, 20, 30, 40, 50]

CODE
    LDAB AL, (bytes + 0)        ; First element
    LDAB AL, (bytes + 1)        ; Second element
    LDAB AL, (bytes + 2)        ; Third element
    
    ; Or using sugar syntax
    LDAB AL, bytes[0]           ; Same as LDAB AL, (bytes + 0)
    LDAB AL, bytes[1]           ; Same as LDAB AL, (bytes + 1)
```

#### Word Arrays

Elements are 2 bytes apart:

```assembly
DATA
    words: DW [100, 200, 300, 400, 500]

CODE
    LDA AX, (words + 0)         ; First element (100)
    LDA AX, (words + 2)         ; Second element (200)
    LDA AX, (words + 4)         ; Third element (300)
    
    ; Or using sugar syntax
    LDA AX, words[0]            ; Same as LDA AX, (words + 0)
    LDA AX, words[2]            ; Same as LDA AX, (words + 2)
```

**Important**: Must account for element size in offset!

---

### Dynamic Array Indexing

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    index: DW [0]

CODE
    LDA CX, index               ; Load index value
    LDAB AL, (array + CX)       ; Load array[index]
    
    ; Increment index
    INC CX
    LDA index, CX               ; Store new index
    LDAB AL, (array + CX)       ; Load array[index+1]
```

---

### Structure Field Access

```assembly
DATA
    ; Simulated structure:
    ; struct Person {
    ;     word age;      // offset 0
    ;     byte initial;  // offset 2
    ;     byte grade;    // offset 3
    ; }
    person_age: DW [25]         ; age = 25
    person_initial: DB [0x4A]   ; initial = 'J'
    person_grade: DB [0x41]     ; grade = 'A'

CODE
    ; Access fields
    LDA AX, (person_age + 0)    ; Load age (word at offset 0)
    LDAB BX, (person_age + 2)   ; Load initial (byte at offset 2)
    LDAB CX, (person_age + 3)   ; Load grade (byte at offset 3)
```

---

### Multi-Dimensional Array Simulation

Calculate offset for 2D array access:

```assembly
DATA
    ; 3x3 matrix stored as linear array
    ; Row-major order: [0,0] [0,1] [0,2] [1,0] [1,1] [1,2] [2,0] [2,1] [2,2]
    matrix: DB [1, 2, 3, 4, 5, 6, 7, 8, 9]

CODE
    ; Access element at matrix[row][col]
    ; Offset = row * width + col (for byte elements)
    
    ; Access matrix[1][2] (row=1, col=2, width=3)
    ; Offset = 1 * 3 + 2 = 5
    LDAB AL, (matrix + 5)       ; Load value 6
    
    ; Dynamic access: matrix[BX][CX] where width=3
    LD BX, 1                    ; row = 1
    LD AX, BX
    MUL AX, 3                   ; AX = row * width
    LD CX, 2                    ; col = 2
    ADD AX, CX                  ; AX = row * width + col
    LDAB AL, (matrix + AX)      ; Load matrix[1][2]
```

---

### Address Arithmetic

Loading addresses vs loading values:

```assembly
DATA
    buffer: DB [10, 20, 30, 40, 50]

CODE
    ; Load VALUE from address
    LDAB AX, buffer             ; AX = 10 (value at buffer)
    LDAB BX, (buffer + 2)       ; BX = 30 (value at buffer+2)
    
    ; Load ADDRESS into register
    LD CX, buffer               ; CX = address of buffer
    LD DX, (buffer + 2)         ; DX = address of buffer+2
    
    ; Use address for indirect access
    LDAB AL, CX                 ; AL = value at address in CX
```

---

## Expression Restrictions

### What You CAN Do

✅ Add constant to label:
```assembly
CODE
    LDA AX, (label + 10)
```

✅ Add register to label:
```assembly
CODE
    LDA AX, (label + CX)
```

✅ Subtract constant from label:
```assembly
CODE
    LDA AX, (label + 10 - 3)    ; = label + 7
```

✅ Complex constant arithmetic:
```assembly
CODE
    LDA AX, (label + 5 + 3 - 2) ; = label + 6
```

✅ Register offset with constant adjustment:
```assembly
CODE
    LDA AX, (label + CX + 5)
```

---

### What You CANNOT Do

❌ Add two labels:
```assembly
CODE
    LDA AX, (label1 + label2)   ; ERROR: Cannot add labels
```

❌ Subtract labels:
```assembly
CODE
    LDA AX, (label1 - label2)   ; ERROR: Cannot subtract labels
```

❌ Add two registers:
```assembly
CODE
    LDA AX, (CX + DX)           ; ERROR: Cannot add registers
```

❌ Multiply or divide:
```assembly
CODE
    LDA AX, (label * 2)         ; ERROR: No multiply operator
    LDA AX, (label / 2)         ; ERROR: No divide operator
```

❌ Use nested parentheses for precedence:
```assembly
CODE
    LDA AX, ((label + 5) * 2)   ; ERROR: No multiply, no nested parens
```

❌ Subtract register:
```assembly
CODE
    LDA AX, (label - CX)        ; ERROR: Cannot subtract register
```

❌ Use square brackets for expressions (except sugar syntax):
```assembly
CODE
    LDA AX, [label + 10]        ; ERROR: Use (label + 10) instead
```

---

## Practical Examples

### Example 1: String Processing

```assembly
DATA
    message: DB "Hello, World!"
    length: DW [13]

CODE
    LD BX, message              ; BX = string address
    LDA CX, length              ; CX = string length
    LD AX, 0                    ; AX = sum accumulator

sum_loop:
    LDAB DX, BX                 ; Load character (indirect)
    ADD AX, DX                  ; Add to sum
    INC BX                      ; Next character
    DEC CX                      ; Decrement counter
    JPNZ sum_loop               ; Continue if more characters
    
    HALT                        ; AX = sum of ASCII values
```

---

### Example 2: Array Maximum

```assembly
DATA
    array: DW [23, 67, 12, 89, 45, 34, 78, 56]
    count: DW [8]
    max: DW [0]

CODE
    LD BX, array                ; BX = array address
    LDA CX, count               ; CX = element count
    LDA AX, (array + 0)         ; AX = first element (initial max)

max_loop:
    LDA DX, BX                  ; Load current element
    CMP DX, AX                  ; Compare with current max
    JPZ update_max              ; If DX > AX (result = 1), update
    JMP continue                ; Otherwise continue

update_max:
    LD AX, DX                   ; New max

continue:
    INC BX                      ; Move to next element (word = 2 bytes)
    INC BX
    DEC CX                      ; Decrement counter
    JPNZ max_loop               ; Continue if more elements
    
    LDA max, AX                 ; Store result
    HALT
```

---

### Example 3: Buffer Copy

```assembly
DATA
    source: DB [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    dest: DB [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    size: DW [10]

CODE
    LD BX, source               ; BX = source pointer
    LD DX, dest                 ; DX = dest pointer
    LDA CX, size                ; CX = byte count

copy_loop:
    LDAB AX, BX                 ; Load byte from source
    LDAB DX, AL                 ; Store byte to dest
    
    INC BX                      ; Next source byte
    INC DX                      ; Next dest byte
    DEC CX                      ; Decrement counter
    JPNZ copy_loop              ; Continue if more bytes
    
    HALT
```

---

### Example 4: Lookup Table

```assembly
DATA
    ; Square lookup table for 0-9
    squares: DW [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]
    input: DW [5]
    result: DW [0]

CODE
    LDA CX, input               ; CX = input value
    
    ; Each word is 2 bytes, so multiply index by 2
    LD AX, CX
    MUL AX, 2                   ; AX = index * 2 (byte offset)
    
    LDA BX, (squares + AX)      ; Load squares[input]
    LDA result, BX              ; Store result
    
    HALT                        ; result = 25 (square of 5)
```

---

### Example 5: Dynamic Structure Access

```assembly
DATA
    ; Array of structures: each has 2 words (4 bytes)
    ; struct Item { word id; word value; }
    items: DW [1, 100, 2, 200, 3, 300]
    item_index: DW [1]          ; Access item 1

CODE
    LDA CX, item_index          ; CX = item index
    
    ; Calculate offset: index * 4 (each item is 4 bytes)
    LD AX, CX
    MUL AX, 4                   ; AX = byte offset to item
    
    ; Access fields
    LDA BX, (items + AX + 0)    ; Load id field
    LDA DX, (items + AX + 2)    ; Load value field
    
    ; BX = 2 (id of item 1)
    ; DX = 200 (value of item 1)
    
    HALT
```

---

## Memory Layout Considerations

### Data Section Layout

Data definitions are laid out sequentially from address 0x0000:

```assembly
DATA
    byte1: DB [0x42]            ; Address: 0x0000 (1 byte)
    word1: DW [0x1234]          ; Address: 0x0001 (2 bytes)
    array: DB [1, 2, 3]         ; Address: 0x0003 (3 bytes)
    word2: DW [0x5678]          ; Address: 0x0006 (2 bytes)
    ; Next address: 0x0008
```

### Code Section Layout

Code starts immediately after the data section:

```assembly
DATA
    value: DW [42]              ; 0x0000-0x0001 (2 bytes)
    ; Data ends at 0x0002

CODE
start:                          ; Address: 0x0002
    LDA AX, value               ; Address: 0x0002
    HALT                        ; Address: 0x0005
```

### Calculating Addresses

Understanding layout helps with expressions:

```assembly
DATA
    buffer: DB [10, 20, 30, 40, 50]     ; Starts at 0x0000
    
CODE
    ; buffer is at 0x0000
    ; buffer + 0 = 0x0000 (element 0)
    ; buffer + 1 = 0x0001 (element 1)
    ; buffer + 2 = 0x0002 (element 2)
    ; etc.
```

---

## Best Practices

### 1. Use Named Offsets

For readability, calculate and document offsets:

```assembly
DATA
    ; Structure offsets
    ; PERSON_AGE = 0
    ; PERSON_NAME = 2
    ; PERSON_SCORE = 34
    
    person_age: DW [25]                 ; age
    person_name: DB "John Doe"          ; name (32 bytes)
    person_score: DW [95]               ; score

CODE
    LDA AX, (person_age + 0)            ; Load age
    LDAB BX, (person_name + 0)          ; Load first char of name
    LDA CX, (person_name + 32)          ; Load score
```

### 2. Account for Element Size

Remember word arrays need offsets of 2, 4, 6, etc.:

```assembly
DATA
    words: DW [100, 200, 300]

CODE
    LDA AX, (words + 0)         ; element 0
    LDA BX, (words + 2)         ; element 1 (NOT +1!)
    LDA CX, (words + 4)         ; element 2 (NOT +2!)
```

### 3. Validate Index Bounds

Check array bounds when using dynamic indexing:

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    array_size: DW [5]

CODE
    LDA CX, index               ; Load index
    LDA DX, array_size          ; Load size
    CMP CX, DX                  ; Compare with size
    JPNC bounds_error           ; Jump if CX >= size
    
    LDAB AL, (array + CX)       ; Safe access
    JMP continue
    
bounds_error:
    ; Handle error
    HALT
    
continue:
    ; Continue processing
```

### 4. Use Pointer Registers Consistently

Designate specific registers for pointers:

```assembly
CODE
    LD BX, source               ; BX = source pointer (convention)
    LD DX, dest                 ; DX = dest pointer (convention)
    LDA CX, count               ; CX = counter (convention)
    
    ; Clear intent: BX and DX are pointers, CX is a count
```

### 5. Comment Complex Expressions

Make address calculations explicit:

```assembly
CODE
    ; Access matrix[row][col] where width=5
    LD AX, BX                   ; AX = row
    MUL AX, 5                   ; AX = row * width
    ADD AX, CX                  ; AX = row * width + col
    LDAB AL, (matrix + AX)      ; Load matrix[row][col]
```

---

## Common Pitfalls

### 1. Word Array Indexing

**Wrong**:
```assembly
DATA
    words: DW [100, 200, 300]

CODE
    LDA AX, (words + 1)         ; Loads partial word! (middle of elements)
```

**Correct**:
```assembly
CODE
    LDA AX, (words + 2)         ; Loads second element (offset by 2 bytes)
```

---

### 2. Expression Order

**Potentially Confusing**:
```assembly
CODE
    LDA AX, (10 + buffer)       ; Works, but unconventional
```

**Better**:
```assembly
CODE
    LDA AX, (buffer + 10)       ; More readable
```

---

### 3. Register Arithmetic Limitations

**Wrong**:
```assembly
CODE
    LDA AX, (CX + DX)           ; ERROR: Can't add two registers
```

**Correct**:
```assembly
CODE
    LD AX, CX
    ADD AX, DX                  ; Calculate address in AX
    LDA BX, AX                  ; Load from calculated address
```

---

### 4. Forgetting Runtime vs Compile-Time

**Wrong Assumption**:
```assembly
CODE
    LD CX, 5
    LDA AX, (buffer + CX)       ; CX evaluated at RUNTIME, not compile-time
    ; You can't do: LDA AX, (buffer + 5) to mean the same thing
```

**Understanding**:
```assembly
CODE
    ; Compile-time: offset is constant
    LD AX, [buffer + 5]         ; Offset 5 is part of instruction
    
    ; Runtime: offset is in register
    LD CX, 5
    LD AX, [buffer + CX]        ; CX value added at runtime
```

---

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Assembly language syntax
- **[Register Reference](Registers.md)**: Register details
- **[Instruction Set](InstructionSet.md)**: Complete instruction reference
- **[Examples](Examples.md)**: Complete program examples
