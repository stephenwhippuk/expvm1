; Test: Increment Register
; Source: InstructionSet.md line 1580
; Purpose: Test INC instruction for register increment
; Expected: Successful assembly with increment operations

CODE
    LD CX, 0
    INC CX              ; CX = 1
    INC CX              ; CX = 2
    HALT
