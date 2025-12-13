# Examples & Tutorials

## Overview

This document provides complete, working example programs for the Pendragon assembler. Each example includes detailed explanations, assembly code, expected output, and learning objectives.

## Table of Contents

1. [Hello World](#hello-world)
2. [Array Index Sugar Syntax](#array-index-sugar-syntax)
3. [Basic Arithmetic](#basic-arithmetic)
4. [Loops and Counting](#loops-and-counting)
5. [Array Operations](#array-operations)
6. [String Processing](#string-processing)
7. [Subroutines and Functions](#subroutines-and-functions)
8. [Conditional Logic](#conditional-logic)
9. [Memory Operations](#memory-operations)
10. [Bit Manipulation](#bit-manipulation)
11. [Advanced: Sorting Algorithm](#advanced-sorting-algorithm)

---

## Hello World

### Learning Objectives
- Basic program structure
- Using DATA and CODE sections
- Simple data definitions
- Loading and halting

### Code

```assembly
; ========================================
; Hello World - Minimal Program
; Demonstrates basic structure
; ========================================

DATA
    message: DB "Hello, World!"
    msg_len: DW [13]

CODE
start:
    ; Load the first character
    LDAB AX, message
    
    ; Program ends
    HALT
```

### Explanation

1. **DATA Section**: Defines a string and its length
   - `message`: String stored as bytes
   - `msg_len`: Word (16-bit) value storing length

2. **CODE Section**: Simple execution flow
   - `start:` - Optional label for entry point
   - `LDAB AX, message` - Loads first byte of message into AX
   - `HALT` - Stops execution

### Expected Behavior

- AX will contain 'H' (0x48) when program halts
- Program terminates cleanly

### Variations

**Version 2: Load Multiple Characters**
```assembly
DATA
    message: DB "Hi!"

CODE
    LDAB AL, (message + 0)    ; Load 'H'
    LDAB BL, (message + 1)    ; Load 'i'
    LDAB CL, (message + 2)    ; Load '!'
    HALT
```

**Note**: For direct byte access from a label, the parentheses syntax `(label + offset)` is used with LDAB.

---

## Array Index Sugar Syntax

### Learning Objectives
- Understanding syntactic sugar for arrays
- When to use sugar syntax vs explicit parentheses
- Conversion rules from sugar to explicit syntax
- Writing clearer array access code

### Overview

The assembler provides **syntactic sugar** for array indexing to make code more readable. The syntax `label[expression]` is automatically converted to memory access with parentheses.

### Basic Examples

```assembly
; ========================================
; Sugar Syntax Basics
; Demonstrates array indexing conversion
; ========================================

DATA
    bytes: DB [10, 20, 30, 40, 50]
    words: DW [100, 200, 300, 400, 500]

CODE
    ; === REGISTER INDICES (Primary use case) ===
    
    LD BX, 3                  ; BX = index
    
    ; These two are IDENTICAL (8-bit register):
    LD CL, bytes[BX]          ; Sugar syntax (auto-converts to LDAB)
    LDAB CL, (bytes + BX)     ; Explicit syntax
    
    ; These two are IDENTICAL (16-bit register):
    LD AX, words[BX]          ; Sugar syntax (auto-converts to LDA)
    LDA AX, (words + BX)      ; Explicit syntax
    
    ; === COMPLEX EXPRESSIONS WITH REGISTERS ===
    
    ; These two are IDENTICAL (8-bit register):
    LD DL, bytes[BX + 1]      ; Sugar (converts to LDAB)
    LDAB DL, (bytes + BX + 1) ; Explicit equivalent
    
    ; === CONSTANT INDICES (Use explicit syntax instead) ===
    
    ; For constant offsets, use explicit parentheses syntax:
    LDAB AL, (bytes + 0)      ; First byte
    LDA AX, (words + 2)       ; Second word
    
    ; Don't use: bytes[0] - this creates LD which converts to LDA
    ; The explicit syntax is clearer for constants
    
    HALT
```

### Conversion Rules

The assembler automatically detects the register size and chooses the appropriate instruction:

**For 16-bit registers** (AX, BX, CX, DX, EX):
```assembly
LD AX, array[index]    →    LDA AX, (array + index)
```

**For 8-bit registers** (AL, AH, BL, BH, CL, CH, DL, DH, EL, EH):
```assembly
LD AL, bytes[index]    →    LDAB AL, (bytes + index)
```

**Key transformations**:
1. `LD` instruction → `LDA` (16-bit) or `LDAB` (8-bit) based on destination register
2. Square brackets `[...]` → Parentheses `(...)`
3. Expression inside is preserved
4. Register size is automatically detected

### When to Use Sugar Syntax

✅ **Use Sugar Syntax When**:
- Accessing array elements **with a register index**
- Dynamic indexing (index in a register)
- Loop-based array access
- Code readability is enhanced by showing array indexing intent

```assembly
; Good use of sugar syntax (register indices)
LD BX, 5
LD AL, buffer[BX]            ; Clear: buffer[BX] (auto → LDAB AL, (buffer + BX))
LD AX, table[CX]             ; Clear: table[CX] (auto → LDA AX, (table + CX))
LD CL, string[DX]            ; Clear: string[DX] (auto → LDAB CL, (string + DX))
```

✅ **Use Explicit Parentheses When**:
- Accessing with **constant offsets**
- Doing address arithmetic (not array access)
- Loading addresses (not values)
- Complex pointer manipulation

```assembly
; Good use of explicit syntax
LDAB AL, (buffer + 0)        ; Constant offset - use parentheses
LDA AX, (table + 4)          ; Constant offset - use parentheses
LD BX, (base + offset)       ; Computing an address
```

**Important**: Sugar syntax `label[const]` with a constant is valid but less clear than `(label + const)`. The sugar syntax shines when used with register indices.

### Complete Example: Array Processing

```assembly
; ========================================
; Array Sum with Sugar Syntax
; Shows sugar syntax in practical use
; ========================================

DATA
    numbers: DW [5, 10, 15, 20, 25, 30]
    count: DW [6]
    total: DW [0]

CODE
    LD AX, 0                 ; AX = accumulator
    LDA CX, count            ; CX = element count
    LD BX, 0                 ; BX = index (starts at 0)

sum_loop:
    ; Load element using sugar syntax
    LDA DX, numbers[BX]      ; Clear: numbers[BX]
    
    ; Add to total
    ADD AX, DX
    
    ; Move to next element (words are 2 bytes)
    ADD BX, 2
    DEC CX
    JPNZ sum_loop
    
    ; Store result
    LDA total, AX            ; total = 105
    HALT
```

### Comparison: With and Without Sugar

```assembly
; ========================================
; Side-by-Side Comparison
; ========================================

DATA
    data: DB [1, 2, 3, 4, 5]

CODE
    ; === WITHOUT SUGAR (Explicit) ===
    LD BX, 0
    LDAB AL, (data + BX)     ; Load data[0]
    INC BX
    LDAB AL, (data + BX)     ; Load data[1]
    INC BX
    LDAB AL, (data + BX)     ; Load data[2]
    
    ; === WITH SUGAR (Clearer) ===
    LD BX, 0
    LD AL, data[BX]          ; Load data[0] (converts to LDAB)
    INC BX
    LD AL, data[BX]          ; Load data[1] (converts to LDAB)
    INC BX
    LD AL, data[BX]          ; Load data[2] (converts to LDAB)
    
    HALT
```

### Advanced: Multi-Dimensional Arrays

```assembly
; ========================================
; 2D Array Access with Sugar Syntax
; array[row][col] pattern
; ========================================

DATA
    ; 3x3 matrix stored as linear array
    matrix: DB [1, 2, 3,
                4, 5, 6,
                7, 8, 9]
    width: DW [3]

CODE
    ; Access matrix[1][2] (row=1, col=2)
    ; Offset = row * width + col = 1 * 3 + 2 = 5
    
    LD BX, 1                 ; row
    LDA AX, width
    MUL BX, AX               ; BX = row * width = 3
    ADD BX, 2                ; BX = row * width + col = 5
    
    ; Now access element using sugar syntax
    LD AL, matrix[BX]        ; Load matrix[1][2] = 6 (converts to LDAB)
    
    HALT
```

### Important Notes

1. **Automatic Conversion**: Sugar syntax is converted during parsing, before semantic analysis
2. **No Performance Difference**: Sugar and explicit syntax generate identical code
3. **Register Size Detection**: `LD` with sugar automatically becomes `LDA` (16-bit) or `LDAB` (8-bit) based on destination register
4. **Works Everywhere**: Can be used with any instruction that accepts memory operands
5. **Automatic Instruction Selection**: You write `LD`, the assembler chooses the right instruction

### Common Patterns

```assembly
DATA
    buffer: DB [0, 0, 0, 0, 0]
    values: DW [10, 20, 30, 40]

CODE
    ; Pattern 1: Fixed index (use explicit syntax)
    LDAB AL, (buffer + 0)    ; First element - constant offset
    LDAB BL, (buffer + 4)    ; Fifth element - constant offset
    
    ; Pattern 2: Variable index (GOOD use of sugar)
    LD CX, 2
    LD AX, values[CX]        ; values[CX] (converts to LDA)
    
    ; Pattern 3: Index with offset (GOOD use of sugar)
    LD BX, 1
    LD DL, buffer[BX + 1]    ; buffer[BX + 1] (converts to LDAB)
    
    ; Pattern 4: Storing with sugar (GOOD use)
    LD BX, 3
    LD AL, 0xFF
    LD buffer[BX], AL        ; buffer[BX] = 0xFF (converts to LDAB)
    
    HALT
```

---

## Basic Arithmetic

### Learning Objectives
- Arithmetic operations (ADD, SUB, MUL, DIV)
- Working with immediate values
- Storing results

### Code

```assembly
; ========================================
; Basic Arithmetic
; Performs: result = (10 + 5) * 2 - 3
; ========================================

DATA
    result: DW [0]

CODE
    ; Calculate (10 + 5) * 2 - 3
    LD AX, 10           ; AX = 10
    ADD AX, 5           ; AX = 15
    MUL AX, 2           ; AX = 30
    SUB AX, 3           ; AX = 27
    
    ; Store result
    LDA result, AX
    
    HALT
```

### Explanation

1. **Initialize**: Load first value (10) into AX
2. **Addition**: Add 5 to AX (now 15)
3. **Multiplication**: Multiply AX by 2 (now 30)
4. **Subtraction**: Subtract 3 from AX (now 27)
5. **Store**: Save final result to memory
6. **Halt**: End program

### Expected Results

- `result` memory location: 27 (0x001B)
- AX register: 27

### Extended Example: Division with Remainder

```assembly
DATA
    dividend: DW [100]
    divisor: DW [7]
    quotient: DW [0]
    remainder: DW [0]

CODE
    ; Calculate 100 / 7
    LDA AX, dividend
    LDA BX, divisor
    DIV AX, BX              ; AX = 14 (quotient)
    LDA quotient, AX
    
    ; Calculate 100 % 7
    LDA AX, dividend
    LDA BX, divisor
    REM AX, BX              ; AX = 2 (remainder)
    LDA remainder, AX
    
    HALT
```

**Results**: quotient = 14, remainder = 2

---

## Loops and Counting

### Learning Objectives
- Loop construction
- Counter management
- Conditional jumps
- Decrement operation

### Code

```assembly
; ========================================
; Countdown Loop
; Counts down from 10 to 0
; ========================================

DATA
    counter: DW [0]

CODE
    ; Initialize counter
    LD CX, 10           ; CX = loop counter

countdown_loop:
    ; Loop body (process CX value here)
    ; For this example, just continue
    
    ; Decrement and check
    DEC CX              ; CX = CX - 1
    JPNZ countdown_loop ; Jump if CX != 0
    
    ; Loop finished, CX is now 0
    LDA counter, CX
    HALT
```

### Explanation

1. **Initialize**: Set CX to 10
2. **Loop Body**: Process current counter value
3. **Decrement**: Reduce counter by 1
4. **Conditional Jump**: Continue loop if not zero
5. **Exit**: When CX reaches 0, fall through to HALT

### Loop Iterations

```
Iteration 1: CX = 10 → 9
Iteration 2: CX = 9 → 8
...
Iteration 10: CX = 1 → 0 (loop exits)
```

### Sum Example

```assembly
; ========================================
; Sum from 1 to 10
; Result: 1+2+3+...+10 = 55
; ========================================

DATA
    sum: DW [0]

CODE
    LD AX, 0            ; AX = accumulator
    LD CX, 10           ; CX = counter (10 down to 1)

sum_loop:
    ADD AX, CX          ; Add counter to sum
    DEC CX              ; Decrement counter
    JPNZ sum_loop       ; Continue if CX != 0
    
    LDA sum, AX         ; Store result (55)
    HALT
```

### Counting Up Example

```assembly
; ========================================
; Count Up from 0 to 9
; ========================================

DATA
    final_count: DW [0]

CODE
    LD AX, 0            ; AX = current count
    LD CX, 10           ; CX = iterations

count_up_loop:
    INC AX              ; Increment count
    DEC CX              ; Decrement iterations
    JPNZ count_up_loop
    
    LDA final_count, AX  ; AX = 10
    HALT
```

---

## Array Operations

### Learning Objectives
- Array indexing
- Pointer arithmetic
- Byte vs word arrays
- Dynamic array access

### Code: Sum Array Elements

```assembly
; ========================================
; Sum Array Elements
; Calculates sum of byte array
; ========================================

DATA
    array: DB [10, 20, 30, 40, 50]
    count: DW [5]
    sum: DW [0]

CODE
    LD AX, 0            ; AX = sum accumulator
    LDA CX, count       ; CX = element count
    LD BX, 0            ; BX = array index

sum_array_loop:
    ; Load array element
    LDAB DX, (array + BX)
    
    ; Add to sum
    ADD AX, DX
    
    ; Next element
    INC BX
    DEC CX
    JPNZ sum_array_loop
    
    ; Store result
    LDA sum, AX         ; sum = 150
    HALT
```

**Alternative with Sugar Syntax**:
```assembly
; Same program using array indexing sugar syntax
CODE
    LD AX, 0            ; AX = sum accumulator
    LDA CX, count       ; CX = element count
    LD BX, 0            ; BX = array index

sum_array_loop:
    ; Load array element using sugar syntax
    LDAB DX, array[BX]  ; Clearer: array[BX]
    
    ; Add to sum
    ADD AX, DX
    
    ; Next element
    INC BX
    DEC CX
    JPNZ sum_array_loop
    
    ; Store result
    LDA sum, AX         ; sum = 150
    HALT
```

### Explanation

1. **Initialize Accumulator**: AX = 0 for sum
2. **Load Count**: CX = number of elements
3. **Index**: BX = current array index (starts at 0)
4. **Loop**:
   - Load element at index BX
   - Add to running sum
   - Increment index
   - Decrement counter
5. **Store**: Save final sum

### Array Maximum

```assembly
; ========================================
; Find Maximum Value in Array
; ========================================

DATA
    numbers: DW [23, 67, 12, 89, 45, 34]
    count: DW [6]
    maximum: DW [0]

CODE
    ; Initialize with first element
    LDA AX, (numbers + 0)   ; AX = current max
    LDA CX, count           ; CX = count
    DEC CX                  ; Already processed first element
    LD BX, 2                ; BX = index (words are 2 bytes)

find_max_loop:
    ; Load current element
    LDA DX, (numbers + BX)
    
    ; Compare with current max
    CMP DX, AX              ; Compare DX with AX
    JPZ update_max          ; If DX > AX (result = 1), update
    JMP continue_max
    
update_max:
    LD AX, DX               ; New maximum
    
continue_max:
    ; Next element (words are 2 bytes apart)
    ADD BX, 2
    DEC CX
    JPNZ find_max_loop
    
    LDA maximum, AX         ; maximum = 89
    HALT
```

### Array Copy

```assembly
; ========================================
; Copy Array
; Copies source array to destination
; ========================================

DATA
    source: DB [1, 2, 3, 4, 5]
    dest: DB [0, 0, 0, 0, 0]
    size: DW [5]

CODE
    LDA CX, size            ; CX = element count
    LD BX, 0                ; BX = index

copy_loop:
    ; Load from source
    LDAB AL, (source + BX)
    
    ; Store to destination
    LDAB (dest + BX), AL
    
    INC BX
    DEC CX
    JPNZ copy_loop
    
    HALT
```

**Alternative with Sugar Syntax**:
```assembly
; Array copy using sugar syntax for clarity
CODE
    LDA CX, size            ; CX = element count
    LD BX, 0                ; BX = index

copy_loop:
    ; Load from source and store to destination
    LDAB AL, source[BX]     ; Clear array indexing
    LDAB dest[BX], AL       ; Store to dest[BX]
    
    INC BX
    DEC CX
    JPNZ copy_loop
    
    HALT
```

---

## String Processing

### Learning Objectives
- Character-by-character processing
- Null-terminated strings
- Character comparison
- String length calculation

### Code: String Length

```assembly
; ========================================
; Calculate String Length
; Finds length of null-terminated string
; ========================================

DATA
    text: DB "Hello, World!\0"
    length: DW [0]

CODE
    LD AX, 0                ; AX = character count
    LD BX, 0                ; BX = string index

strlen_loop:
    ; Load current character
    LDAB CL, (text + BX)
    
    ; Check for null terminator
    CMP CL, 0
    JPZ strlen_done
    
    ; Not null, increment count and continue
    INC AX
    INC BX
    JMP strlen_loop

strlen_done:
    LDA length, AX          ; length = 13
    HALT
```

### Character Count

```assembly
; ========================================
; Count Specific Character
; Counts occurrences of 'l' in string
; ========================================

DATA
    message: DB "Hello, World!\0"
    target: DB [0x6C]       ; ASCII 'l'
    count: DW [0]

CODE
    LD AX, 0                ; AX = match count
    LD BX, 0                ; BX = string index

count_char_loop:
    ; Load current character
    LDAB CL, (text + BX)
    
    ; Check for null terminator
    CMP CL, 0
    JPZ count_done
    
    ; Compare with target character
    LDAB DL, target
    CPL CL, DL
    JPZ is_match
    JMP next_char

is_match:
    INC AX                  ; Increment count

next_char:
    INC BX                  ; Next character
    JMP count_char_loop

count_done:
    LDA count, AX           ; count = 3 (three 'l's)
    HALT
```

### Character Case Conversion

```assembly
; ========================================
; Convert to Uppercase
; Converts lowercase letters to uppercase
; ========================================

DATA
    input: DB "hello"
    output: DB [0, 0, 0, 0, 0, 0]

CODE
    LD BX, 0                ; Index

convert_loop:
    ; Load character
    LDAB AL, (input + BX)
    
    ; Check if null
    CMP AL, 0
    JPZ convert_done
    
    ; Check if lowercase (0x61-0x7A)
    CMP AL, 0x61
    JPC convert_loop        ; If < 'a', skip
    CMP AL, 0x7A
    JPNC skip_convert       ; If > 'z', skip
    
    ; Convert: subtract 32
    SUB AL, 32
    
skip_convert:
    ; Store converted character
    ; (Conceptual - actual store may differ)
    
    INC BX
    JMP convert_loop

convert_done:
    HALT
```

---

## Subroutines and Functions

### Learning Objectives
- Function calls and returns
- Parameter passing
- Register preservation
- Stack usage

### Code: Square Function

```assembly
; ========================================
; Square Function
; Calculates square of a number
; ========================================

DATA
    input: DW [7]
    result: DW [0]

CODE
main:
    ; Load input
    LDA AX, input
    
    ; Call square function
    CALL square
    
    ; Store result
    LDA result, AX          ; result = 49
    HALT

; Function: square
; Input: AX = number
; Output: AX = number squared
square:
    MUL AX, AX              ; AX = AX * AX
    RET
```

### Multiple Parameters

```assembly
; ========================================
; Add Function with Two Parameters
; Adds two numbers
; ========================================

DATA
    num1: DW [15]
    num2: DW [27]
    sum: DW [0]

CODE
main:
    ; Load parameters
    LDA AX, num1            ; First parameter
    LDA BX, num2            ; Second parameter
    
    ; Call add function
    CALL add_numbers
    
    ; Store result
    LDA sum, AX             ; sum = 42
    HALT

; Function: add_numbers
; Input: AX = first number, BX = second number
; Output: AX = sum
add_numbers:
    ADD AX, BX
    RET
```

### Register Preservation

```assembly
; ========================================
; Function with Register Preservation
; Saves and restores registers
; ========================================

DATA
    value: DW [10]
    result: DW [0]

CODE
main:
    LDA AX, value
    LD BX, 5                ; BX used by main
    
    CALL multiply_by_three
    
    ; BX still contains 5
    LDA result, AX
    HALT

; Function: multiply_by_three
; Input: AX = number
; Output: AX = number * 3
; Preserves: BX, CX
multiply_by_three:
    ; Save registers
    PUSH BX
    PUSH CX
    
    ; Use registers for calculation
    LD BX, 3
    MUL AX, BX
    
    ; Restore registers
    POP CX
    POP BX
    
    RET
```

---

## Conditional Logic

### Learning Objectives
- If-then-else structures
- Multiple conditions
- Flag-based branching
- Comparison operations

### Code: Simple If-Else

```assembly
; ========================================
; Temperature Check
; Categorizes temperature as cold/normal/hot
; ========================================

DATA
    temperature: DW [75]
    status: DW [0]          ; 0=normal, 1=cold, 2=hot

CODE
    LDA AX, temperature
    
    ; Check if temperature < 50 (cold)
    CMP AX, 50
    JPNZ check_hot          ; If AX >= 50, check if hot
    
is_cold:
    LD BX, 1                ; Status: cold
    JMP store_status

check_hot:
    ; Check if temperature > 80 (hot)
    CMP AX, 80
    JPZ is_hot              ; If AX > 80, it's hot
    
is_normal:
    LD BX, 0                ; Status: normal
    JMP store_status

is_hot:
    LD BX, 2                ; Status: hot

store_status:
    LDA status, BX
    HALT
```

### Range Checking

```assembly
; ========================================
; Grade Calculator
; Converts score to letter grade
; ========================================

DATA
    score: DW [85]
    grade: DB [0]           ; 'A'=65, 'B'=66, 'C'=67, 'D'=68, 'F'=70

CODE
    LDA AX, score
    
    ; Check for A (90-100)
    CMP AX, 90
    JPNC check_b
    LD BL, 0x41             ; 'A'
    JMP store_grade

check_b:
    CMP AX, 80
    JPNC check_c
    LD BL, 0x42             ; 'B'
    JMP store_grade

check_c:
    CMP AX, 70
    JPNC check_d
    LD BL, 0x43             ; 'C'
    JMP store_grade

check_d:
    CMP AX, 60
    JPNC grade_f
    LD BL, 0x44             ; 'D'
    JMP store_grade

grade_f:
    LD BL, 0x46             ; 'F'

store_grade:
    LDAB grade, BL
    HALT
```

### Multiple Conditions (AND)

```assembly
; ========================================
; Check Valid Range
; Value must be >= 10 AND <= 100
; ========================================

DATA
    value: DW [50]
    is_valid: DW [0]        ; 0=invalid, 1=valid

CODE
    LDA AX, value
    
    ; Check if value >= 10
    CMP AX, 10
    JPC invalid             ; If < 10, invalid
    
    ; Check if value <= 100
    CMP AX, 100
    JPZ invalid             ; If > 100, invalid
    
valid:
    LD BX, 1                ; Valid
    JMP store_result

invalid:
    LD BX, 0                ; Invalid

store_result:
    LDA is_valid, BX
    HALT
```

---

## Memory Operations

### Learning Objectives
- Direct memory access
- Pointer usage
- Memory copying
- Address calculation

### Code: Memory Fill

```assembly
; ========================================
; Fill Memory Block
; Sets all bytes in a block to a value
; ========================================

DATA
    buffer: DB [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    fill_value: DB [0xFF]
    size: DW [10]

CODE
    LDAB AL, fill_value     ; AL = value to fill
    LDA CX, size            ; CX = byte count
    LD BX, 0                ; BX = index

fill_loop:
    ; Store value at buffer[BX]
    LDAB (buffer + BX), AL
    
    INC BX
    DEC CX
    JPNZ fill_loop
    
    HALT
```

### Memory Swap

```assembly
; ========================================
; Swap Two Memory Locations
; ========================================

DATA
    var1: DW [100]
    var2: DW [200]

CODE
    ; Load both values
    LDA AX, var1            ; AX = 100
    LDA BX, var2            ; BX = 200
    
    ; Swap
    LDA var1, BX            ; var1 = 200
    LDA var2, AX            ; var2 = 100
    
    HALT
```

### Pointer Traversal

```assembly
; ========================================
; Process Buffer Using Pointer
; ========================================

DATA
    buffer: DB [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    count: DW [10]
    sum: DW [0]

CODE
    LD BX, buffer           ; BX = pointer to buffer
    LDA CX, count           ; CX = element count
    LD AX, 0                ; AX = sum

pointer_loop:
    ; Load byte from address in BX
    LDAB DL, BX             ; Indirect addressing
    ADD AX, DX
    
    INC BX                  ; Move pointer
    DEC CX
    JPNZ pointer_loop
    
    LDA sum, AX             ; sum = 55
    HALT
```

---

## Bit Manipulation

### Learning Objectives
- Bitwise operations (AND, OR, XOR, NOT)
- Bit shifting and rotation
- Bit masking
- Flag manipulation

### Code: Set and Clear Bits

```assembly
; ========================================
; Bit Manipulation Operations
; ========================================

DATA
    flags: DW [0x0000]

CODE
    LDA AX, flags
    
    ; Set bit 0 (OR with 0x0001)
    OR AX, 0x0001           ; AX = 0x0001
    
    ; Set bit 4 (OR with 0x0010)
    OR AX, 0x0010           ; AX = 0x0011
    
    ; Clear bit 0 (AND with ~0x0001 = 0xFFFE)
    AND AX, 0xFFFE          ; AX = 0x0010
    
    ; Toggle bit 4 (XOR with 0x0010)
    XOR AX, 0x0010          ; AX = 0x0000
    
    LDA flags, AX
    HALT
```

### Bit Shifting for Multiplication/Division

```assembly
; ========================================
; Fast Multiplication/Division by Powers of 2
; ========================================

DATA
    value: DW [16]
    times_four: DW [0]
    div_by_two: DW [0]

CODE
    LDA AX, value           ; AX = 16
    
    ; Multiply by 4 (shift left 2 bits)
    SHL AX, 2               ; AX = 64
    LDA times_four, AX
    
    LDA AX, value           ; AX = 16
    
    ; Divide by 2 (shift right 1 bit)
    SHR AX, 1               ; AX = 8
    LDA div_by_two, AX
    
    HALT
```

### Extract Bit Field

```assembly
; ========================================
; Extract Bits 4-7 from a Value
; ========================================

DATA
    value: DW [0xABCD]
    extracted: DW [0]

CODE
    LDA AX, value           ; AX = 0xABCD
    
    ; Shift right 4 bits
    SHR AX, 4               ; AX = 0x0ABC
    
    ; Mask to keep only lower 4 bits
    AND AX, 0x000F          ; AX = 0x000C
    
    LDA extracted, AX       ; extracted = 0x000C
    HALT
```

### Rotate for Circular Shifts

```assembly
; ========================================
; Circular Bit Rotation
; ========================================

DATA
    value: DW [0x8001]
    rotated_left: DW [0]
    rotated_right: DW [0]

CODE
    LDA AX, value           ; AX = 0x8001
    
    ; Rotate left 1 bit
    ROL AX, 1               ; AX = 0x0003 (bit 15 wraps to bit 0)
    LDA rotated_left, AX
    
    LDA AX, value           ; AX = 0x8001
    
    ; Rotate right 1 bit
    ROR AX, 1               ; AX = 0xC000 (bit 0 wraps to bit 15)
    LDA rotated_right, AX
    
    HALT
```

---

## Advanced: Sorting Algorithm

### Learning Objectives
- Complex algorithm implementation
- Nested loops
- Array manipulation
- Comparison and swapping

### Code: Bubble Sort

```assembly
; ========================================
; Bubble Sort Algorithm
; Sorts array in ascending order
; ========================================

DATA
    array: DW [64, 34, 25, 12, 22, 11, 90]
    size: DW [7]

CODE
    LDA CX, size            ; CX = outer loop counter
    DEC CX                  ; n-1 passes

outer_loop:
    PUSH CX                 ; Save outer counter
    
    ; Inner loop: compare adjacent elements
    LD BX, 0                ; BX = array index
    LD DX, CX               ; DX = inner loop counter

inner_loop:
    ; Load array[i]
    LDA AX, (array + BX)
    
    ; Load array[i+1]
    LD DI, BX
    ADD DI, 2               ; Next word (2 bytes)
    LDA EX, (array + DI)
    
    ; Compare array[i] with array[i+1]
    CMP AX, EX
    JPZ swap_elements       ; If array[i] > array[i+1], swap
    JMP no_swap

swap_elements:
    ; Swap array[i] and array[i+1]
    LDA (array + BX), EX
    LDA (array + DI), AX

no_swap:
    ; Move to next pair
    ADD BX, 2
    DEC DX
    JPNZ inner_loop
    
    ; Continue outer loop
    POP CX
    DEC CX
    JPNZ outer_loop
    
    HALT
```

### Explanation

1. **Outer Loop**: Runs n-1 times (where n is array size)
2. **Inner Loop**: Compares adjacent elements
3. **Comparison**: If current > next, swap them
4. **Progress**: After each outer loop iteration, largest unsorted element "bubbles" to end
5. **Result**: Array sorted in ascending order

### Expected Results

**Before**: [64, 34, 25, 12, 22, 11, 90]  
**After**: [11, 12, 22, 25, 34, 64, 90]

---

## Best Practices Demonstrated

### 1. Clear Comments

```assembly
; ========================================
; Function: calculate_average
; Purpose: Calculates average of array
; Input: Array address in BX, count in CX
; Output: Average in AX
; ========================================
```

### 2. Meaningful Labels

```assembly
; Good
calculate_sum_loop:
    ...

; Avoid
loop1:
    ...
```

### 3. Organized Data Section

```assembly
DATA
    ; === Input Values ===
    input1: DW [10]
    input2: DW [20]
    
    ; === Results ===
    sum: DW [0]
    product: DW [0]
    
    ; === Constants ===
    MAX_VALUE: DW [100]
```

### 4. Register Usage Conventions

```assembly
CODE
    LDA CX, count       ; CX for counters
    LD BX, array        ; BX for pointers
    LD AX, 0            ; AX for accumulation
```

### 5. Error-Free Practices

- Always initialize counters
- Check array bounds
- Preserve registers in functions
- Clear accumulator before loops

---

## Building Complexity

### Progression Path

1. **Start Simple**: Hello World, basic arithmetic
2. **Add Control Flow**: Loops, conditionals
3. **Work with Data**: Arrays, strings
4. **Modularize**: Functions, subroutines
5. **Combine Concepts**: Complex algorithms

### Example Progression

**Level 1**: Load and halt
```assembly
CODE
    LD AX, 42
    HALT
```

**Level 2**: Simple arithmetic
```assembly
CODE
    LD AX, 10
    ADD AX, 5
    HALT
```

**Level 3**: Loop
```assembly
CODE
    LD CX, 10
loop:
    DEC CX
    JPNZ loop
    HALT
```

**Level 4**: Array sum
```assembly
DATA
    array: DB [1, 2, 3]
CODE
    LD AX, 0
    LD CX, 3
    LD BX, 0
loop:
    LDAB DX, (array + BX)
    ADD AX, DX
    INC BX
    DEC CX
    JPNZ loop
    HALT
```

---

## Debugging Tips

### 1. Start Small

Test each part independently:
```assembly
; Test data loading first
CODE
    LDA AX, value
    HALT

; Then add arithmetic
CODE
    LDA AX, value
    ADD AX, 10
    HALT
```

### 2. Use Strategic HALTs

Insert HALTs to check intermediate states:
```assembly
CODE
    LD AX, 10
    HALT              ; Check AX = 10
    
    ADD AX, 5
    HALT              ; Check AX = 15
```

### 3. Verify Loop Logic

Test loop with small iterations:
```assembly
; Instead of:
LD CX, 1000

; Use:
LD CX, 3          ; Easy to verify manually
```

---

## See Also

- **[Overview](Overview.md)**: Getting started guide
- **[Syntax Reference](Syntax.md)**: Assembly language syntax
- **[Register Reference](Registers.md)**: Register usage
- **[Instruction Set](InstructionSet.md)**: Complete instruction reference
- **[Expressions](Expressions.md)**: Address expressions
- **[Error Reference](ErrorReference.md)**: Debugging help
- **[Binary Format](BinaryFormat.md)**: Understanding output files
