; Test: Code with Comments
; Source: Syntax.md line 608
; Purpose: Test code with explanatory comments
; Expected: Successful assembly with comment documentation

CODE
    ; Set up loop counter for 10 iterations
    LD CX, 10
    
loop:
    ; Perform operation
    ADD AX, BX
    
    ; Decrement and loop if not zero
    DEC CX
    JPNZ loop
    
    HALT
