; Test: Division Operations
; Source: InstructionSet.md line 955
; Purpose: Test DIV instruction with immediate and register
; Expected: Successful assembly with integer division

CODE
    LD AX, 100
    LD BX, 3
    DIV AX, 3           ; AX = 33 (integer division)
    DIV AX, BX          ; AX = AX / BX
    HALT
