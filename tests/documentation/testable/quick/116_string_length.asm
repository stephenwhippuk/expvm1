; Test: String Length Calculation
; Source: Examples.md line 405
; Purpose: Test calculating length of null-terminated string
; Expected: Successful assembly with string processing

DATA
    text: DB "Hello, World!\0"
    length: DW [0]

CODE
    LD AX, 0                ; AX = character count
    LD BX, 0                ; BX = string index

strlen_loop:
    ; Load current character
    LDAB CL, (text + 2 + BX)
    
    ; Check for null terminator
    CMP CL, 0
    JPZ strlen_done
    
    ; Not null, increment count and continue
    INC AX
    INC BX
    JMP strlen_loop

strlen_done:
    LDA length, AX         ; length = 13
    HALT
