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
    LD AX, [buffer]         ; Direct label reference
    LD BX, [buffer + 2]     ; Label + offset
    LDA CX, buffer + 4      ; Address calculation
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

**Syntax**: `instruction operand, [address]` or `[label]`

**Examples**:
```assembly
DATA
    value: DW [0x1234]
    byte_val: DB [0x42]

CODE
    LD AX, [value]          ; Load word from address
    LDAB BX, [byte_val]     ; Load byte from address
    LD [value], CX          ; Store word to address
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
    LDA BX, array           ; BX = address of array
    LD AL, (BX)             ; Load byte from address in BX
    INC BX                  ; Move to next element
    LD AL, (BX)             ; Load next byte
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

**Syntax**: `instruction operand, [label + offset]`

**Examples**:
```assembly
DATA
    array: DW [100, 200, 300, 400, 500]

CODE
    LD AX, [array + 0]      ; First element (array[0])
    LD BX, [array + 2]      ; Second element (array[1]) - words are 2 bytes
    LD CX, [array + 4]      ; Third element (array[2])
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

**Syntax**: `instruction operand, [label + register]`

**Examples**:
```assembly
DATA
    array: DB [10, 20, 30, 40, 50]

CODE
    LD CX, 0                ; Index = 0
    LD AL, [array + CX]     ; Load array[0]
    
    LD CX, 3                ; Index = 3
    LD AL, [array + CX]     ; Load array[3]
    
    INC CX                  ; Index = 4
    LD AL, [array + CX]     ; Load array[4]
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
    LD AX, [base + 10]      ; base address + 10 bytes
    LD BX, [base + CX]      ; base address + CX bytes
    LDA DX, base + 100      ; Address of (base + 100)
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
    LD AX, [buffer + 10 - 2]    ; (buffer + 10) - 2 = buffer + 8
    LDA BX, end_marker - 1       ; Address before end_marker
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
    LD AX, [buffer + 4 + 2]     ; Evaluates as (buffer + 4) + 2 = buffer + 6
    LD BX, [buffer + 10 - 3]    ; Evaluates as (buffer + 10) - 3 = buffer + 7
```

---

### Expression Evaluation

#### Compile-Time Evaluation

Most expression parts are evaluated during assembly:

```assembly
DATA
    array: DB [1, 2, 3, 4, 5]

CODE
    LD AX, [array + 3]          ; Address computed at assembly time
                                 ; Becomes: LD AX, [0x0003] (if array is at 0x0000)
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
    LD AX, [array + CX]         ; CX added to base address at runtime
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
    LD AL, [bytes + 0]          ; First element
    LD AL, [bytes + 1]          ; Second element
    LD AL, [bytes + 2]          ; Third element
```

#### Word Arrays

Elements are 2 bytes apart:

```assembly
DATA
    words: DW [100, 200, 300, 400, 500]

CODE
    LD AX, [words + 0]          ; First element (100)
    LD AX, [words + 2]          ; Second element (200)
    LD AX, [words + 4]          ; Third element (300)
```

**Important**: Must account for element size in offset!

---

### Dynamic Array Indexing

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    index: DW [0]

CODE
    LD CX, [index]              ; Load index value
    LD AL, [array + CX]         ; Load array[index]
    
    ; Increment index
    INC CX
    LD [index], CX              ; Store new index
    LD AL, [array + CX]         ; Load array[index+1]
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
    LD AX, [person_age + 0]     ; Load age (word at offset 0)
    LDAB BX, [person_age + 2]   ; Load initial (byte at offset 2)
    LDAB CX, [person_age + 3]   ; Load grade (byte at offset 3)
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
    LD AL, [matrix + 5]         ; Load value 6
    
    ; Dynamic access: matrix[BX][CX] where width=3
    LD BX, 1                    ; row = 1
    LD AX, BX
    MUL AX, 3                   ; AX = row * width
    LD CX, 2                    ; col = 2
    ADD AX, CX                  ; AX = row * width + col
    LD AL, [matrix + AX]        ; Load matrix[1][2]
```

---

### Address Arithmetic

Loading addresses vs loading values:

```assembly
DATA
    buffer: DB [10, 20, 30, 40, 50]

CODE
    ; Load VALUE from address
    LD AX, [buffer]             ; AX = 10 (value at buffer)
    LD BX, [buffer + 2]         ; BX = 30 (value at buffer+2)
    
    ; Load ADDRESS into register
    LDA CX, buffer              ; CX = address of buffer
    LDA DX, buffer + 2          ; DX = address of buffer+2
    
    ; Use address for indirect access
    LD AL, (CX)                 ; AL = value at address in CX
```

---

## Expression Restrictions

### What You CAN Do

✅ Add constant to label:
```assembly
CODE
    LD AX, [label + 10]
```

✅ Add register to label:
```assembly
CODE
    LD AX, [label + CX]
```

✅ Subtract constant from label:
```assembly
CODE
    LD AX, [label + 10 - 3]     ; = label + 7
```

✅ Complex constant arithmetic:
```assembly
CODE
    LD AX, [label + 5 + 3 - 2]  ; = label + 6
```

✅ Register offset with constant adjustment:
```assembly
CODE
    LD AX, [label + CX + 5]
```

---

### What You CANNOT Do

❌ Add two labels:
```assembly
CODE
    LD AX, [label1 + label2]    ; ERROR: Cannot add labels
```

❌ Subtract labels:
```assembly
CODE
    LD AX, [label1 - label2]    ; ERROR: Cannot subtract labels
```

❌ Add two registers:
```assembly
CODE
    LD AX, [CX + DX]            ; ERROR: Cannot add registers
```

❌ Multiply or divide:
```assembly
CODE
    LD AX, [label * 2]          ; ERROR: No multiply operator
    LD AX, [label / 2]          ; ERROR: No divide operator
```

❌ Use parentheses for grouping:
```assembly
CODE
    LD AX, [(label + 5) * 2]    ; ERROR: No parentheses, no multiply
```

❌ Subtract register:
```assembly
CODE
    LD AX, [label - CX]         ; ERROR: Cannot subtract register
```

---

## Practical Examples

### Example 1: String Processing

```assembly
DATA
    message: DB "Hello, World!"
    length: DW [13]

CODE
    LDA BX, message             ; BX = string address
    LD CX, [length]             ; CX = string length
    LD AX, 0                    ; AX = sum accumulator

sum_loop:
    LDAB DX, (BX)               ; Load character (indirect)
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
    LDA BX, array               ; BX = array address
    LD CX, [count]              ; CX = element count
    LD AX, [array + 0]          ; AX = first element (initial max)

max_loop:
    LD DX, (BX)                 ; Load current element
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
    
    LD [max], AX                ; Store result
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
    LDA BX, source              ; BX = source pointer
    LDA DX, dest                ; DX = dest pointer
    LD CX, [size]               ; CX = byte count

copy_loop:
    LDAB AX, (BX)               ; Load byte from source
    ; Store to dest (would need store instruction)
    LD (DX), AL                 ; Store byte to dest
    
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
    LD CX, [input]              ; CX = input value
    
    ; Each word is 2 bytes, so multiply index by 2
    LD AX, CX
    MUL AX, 2                   ; AX = index * 2 (byte offset)
    
    LD BX, [squares + AX]       ; Load squares[input]
    LD [result], BX             ; Store result
    
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
    LD CX, [item_index]         ; CX = item index
    
    ; Calculate offset: index * 4 (each item is 4 bytes)
    LD AX, CX
    MUL AX, 4                   ; AX = byte offset to item
    
    ; Access fields
    LD BX, [items + AX + 0]     ; Load id field
    LD DX, [items + AX + 2]     ; Load value field
    
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
    LD AX, [value]              ; Address: 0x0002
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
    LD AX, [person_age + 0]             ; Load age
    LDAB BX, [person_name + 0]          ; Load first char of name
    LD CX, [person_name + 32]           ; Load score
```

### 2. Account for Element Size

Remember word arrays need offsets of 2, 4, 6, etc.:

```assembly
DATA
    words: DW [100, 200, 300]

CODE
    LD AX, [words + 0]          ; element 0
    LD BX, [words + 2]          ; element 1 (NOT +1!)
    LD CX, [words + 4]          ; element 2 (NOT +2!)
```

### 3. Validate Index Bounds

Check array bounds when using dynamic indexing:

```assembly
DATA
    array: DB [10, 20, 30, 40, 50]
    array_size: DW [5]

CODE
    LD CX, [index]              ; Load index
    CMP CX, [array_size]        ; Compare with size
    JPNC bounds_error           ; Jump if CX >= size
    
    LD AL, [array + CX]         ; Safe access
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
    LDA BX, source              ; BX = source pointer (convention)
    LDA DX, dest                ; DX = dest pointer (convention)
    LD CX, count                ; CX = counter (convention)
    
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
    LD AL, [matrix + AX]        ; Load matrix[row][col]
```

---

## Common Pitfalls

### 1. Word Array Indexing

**Wrong**:
```assembly
DATA
    words: DW [100, 200, 300]

CODE
    LD AX, [words + 1]          ; Loads partial word! (middle of elements)
```

**Correct**:
```assembly
CODE
    LD AX, [words + 2]          ; Loads second element (offset by 2 bytes)
```

---

### 2. Expression Order

**Potentially Confusing**:
```assembly
CODE
    LD AX, [10 + buffer]        ; Works, but unconventional
```

**Better**:
```assembly
CODE
    LD AX, [buffer + 10]        ; More readable
```

---

### 3. Register Arithmetic Limitations

**Wrong**:
```assembly
CODE
    LD AX, [CX + DX]            ; ERROR: Can't add two registers
```

**Correct**:
```assembly
CODE
    LD AX, CX
    ADD AX, DX                  ; Calculate address in AX
    LD BX, (AX)                 ; Load from calculated address
```

---

### 4. Forgetting Runtime vs Compile-Time

**Wrong Assumption**:
```assembly
CODE
    LD CX, 5
    LD AX, [buffer + CX]        ; CX evaluated at RUNTIME, not compile-time
    ; You can't do: LD AX, [buffer + 5] to mean the same thing
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
