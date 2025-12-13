; Test: Quick Start Example
; Source: Overview.md line 36
; Purpose: Test the hello world example from the overview
; Expected: Successful assembly

DATA
    message: DB "Hello, World!"

CODE
start:
    LD CX, 0x0B         ; Length of message minus null terminator
    
loop:
    DEC CX
    LDA AX, (message + CX)
    PUSHB AX
    CMP CX, 0x00
    JPNZ loop
    
    PUSHW 0x000B        ; Push length
    ; SYS 0x0011        ; System call to print (commented for test)
    HALT
