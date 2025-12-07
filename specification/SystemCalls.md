# Pendragon Virtual Machine - System Call Reference

## System Call Interface

System calls provide access to operating system-level services such as I/O operations. System calls are invoked through the `SYSCALL` instruction (opcode 0x80) which takes a system call number as an argument.

### System Call Convention

When making a system call:
1. Prepare any required arguments on the stack
2. Execute `SYSCALL <number>` where `<number>` is the system call index
3. Results (if any) are returned on the stack

### Reserved Ranges

- **0x0000 - 0x000F**: Reserved for system exit modes
- **0x0010 - 0x00FF**: I/O operations
- **0x0100 - 0x01FF**: Reserved for future use

## System Call Table

| Index | Hex    | Name                        | Category | Summary |
|-------|--------|----------------------------|----------|---------|
| 16    | 0x0010 | PRINT_STRING_FROM_STACK    | I/O      | Output a string from the stack to console |
| 17    | 0x0011 | PRINT_LINE_FROM_STACK      | I/O      | Output a string from the stack to console with newline |
| 18    | 0x0012 | READ_LINE_ONTO_STACK       | I/O      | Read a line from console input onto the stack |

---

## I/O Operations (0x0010 - 0x00FF)

### PRINT_STRING_FROM_STACK (0x0010)

**Description**: Outputs a string from the stack to the console without adding a newline.

**Stack Arguments** (in order of pushing):
- Character bytes (pushed in reverse order)
- Character count (WORD) - number of characters to print

**Stack Layout Before Call**:
```
TOP -> [count_low] [count_high] [char_n] ... [char_2] [char_1]
```

**Returns**: Nothing

**Side Effects**: 
- Pops count + 2 bytes from stack
- Outputs text to stdout

**Example Usage**:
```asm
; Print "Hello"
.byte 0x76, 'o'    ; PUSHB 'o'
.byte 0x76, 'l'    ; PUSHB 'l'
.byte 0x76, 'l'    ; PUSHB 'l'
.byte 0x76, 'e'    ; PUSHB 'e'
.byte 0x76, 'H'    ; PUSHB 'H'
.byte 0x02, 0x00, 0x05, 0x00  ; LD AX, 5 (5 characters)
.byte 0x10, 0x00   ; PUSH AX (push count)
.byte 0x7F, 0x10, 0x00  ; SYS 0x0010 (print string)
```

---

### PRINT_LINE_FROM_STACK (0x0011)

**Description**: Outputs a string from the stack to the console followed by a newline character.

**Stack Arguments** (in order of pushing):
- Character bytes (pushed in reverse order)
- Character count (WORD) - number of characters to print

**Stack Layout Before Call**:
```
TOP -> [count_low] [count_high] [char_n] ... [char_2] [char_1]
```

**Returns**: Nothing

**Side Effects**: 
- Pops count + 2 bytes from stack
- Outputs text to stdout followed by '\n'
- Flushes output buffer

**Example Usage**:
```asm
; Print "Hello" with newline
.byte 0x76, 'o'    ; PUSHB 'o'
.byte 0x76, 'l'    ; PUSHB 'l'
.byte 0x76, 'l'    ; PUSHB 'l'
.byte 0x76, 'e'    ; PUSHB 'e'
.byte 0x76, 'H'    ; PUSHB 'H'
.byte 0x02, 0x00, 0x05, 0x00  ; LD AX, 5 (5 characters)
.byte 0x10, 0x00   ; PUSH AX (push count)
.byte 0x7F, 0x11, 0x00  ; SYS 0x0011 (print line)
```

---

### READ_LINE_ONTO_STACK (0x0012)

**Description**: Reads a line of text from console input (stdin) and pushes it onto the stack.

**Stack Arguments** (in order of pushing):
- Maximum length (WORD) - maximum number of characters to read

**Stack Layout Before Call**:
```
TOP -> [max_length_low] [max_length_high]
```

**Returns** (on stack):
```
TOP -> [count_low] [count_high] [char_1] [char_2] ... [char_n]
```
Where:
- count (WORD) - actual number of characters read
- char_1 through char_n - the character bytes in reading order

**Side Effects**: 
- Pops 2 bytes (max length) from stack
- Pushes actual_count + 2 bytes onto stack
- Reads from stdin until newline or max_length reached
- Input is truncated if it exceeds max_length

**Notes**:
- The newline character is NOT included in the returned string
- Characters are pushed in reverse order (char_n first, char_1 last)
- Count reflects actual characters read, which may be less than max_length

**Example Usage**:
```asm
; Read up to 80 characters
.byte 0x02, 0x00, 0x50, 0x00  ; LD AX, 80 (max length)
.byte 0x10, 0x00   ; PUSH AX (push max length)
.byte 0x7F, 0x12, 0x00  ; SYS 0x0012 (read line)

; Stack now contains:
; [count] [char_1] [char_2] ... [char_n]
.byte 0x13, 0x00   ; POP AX (get actual count)
```

---

## Error Handling

If an invalid system call number is provided, the system will:
1. Throw a runtime error
2. Display: "Invalid system call number: <number>"
3. Halt execution

**Valid System Call Numbers**: 0x0010 - 0x0012 (currently implemented)

---

## Implementation Notes

### Stack Usage

All I/O system calls use the stack accessor in the appropriate mode:
- **READ_ONLY** mode for output operations (PRINT_STRING, PRINT_LINE)
- **READ_WRITE** mode for input operations (READ_LINE)

### Character Encoding

- All text is treated as single-byte characters
- Characters are cast between `byte_t` and `char` types
- No Unicode or multi-byte character support in current implementation

### Buffer Management

- **PRINT operations**: Characters are accumulated in a std::string before output
- **READ operation**: Input is read via std::getline and processed immediately
- No internal buffering beyond standard C++ iostream buffers

---

## Future System Calls

Reserved ranges for planned future functionality:

- **0x0020 - 0x002F**: File I/O operations
- **0x0030 - 0x003F**: Memory management
- **0x0040 - 0x004F**: Process control
- **0x0050 - 0x005F**: Time and date operations
