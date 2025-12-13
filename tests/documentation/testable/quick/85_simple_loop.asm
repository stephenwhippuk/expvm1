; Test: Simple Loop Example
; Source: InstructionSet.md line 1799
; Purpose: Test complete loop with accumulator pattern
; Expected: Successful assembly with loop and arithmetic

CODE
    LD CX, 10           ; Loop 10 times
    LD AX, 0            ; Accumulator
loop:
    ADD AX, 1           ; Increment accumulator
    DEC CX              ; Decrement counter
    JPNZ loop           ; Continue if CX != 0
    HALT                ; AX = 10
