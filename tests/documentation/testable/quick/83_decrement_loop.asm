; Test: Decrement with Loop
; Source: InstructionSet.md line 1602
; Purpose: Test DEC instruction in countdown loop pattern
; Expected: Successful assembly with decrement loop

CODE
    LD CX, 10
loop:
    DEC CX              ; CX = CX - 1
    JPNZ loop           ; Continue if CX != 0
    HALT
