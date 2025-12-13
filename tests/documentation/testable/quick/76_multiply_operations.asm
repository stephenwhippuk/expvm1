; Test: Multiply Operations
; Source: InstructionSet.md line 904
; Purpose: Test MUL instruction with immediate and register
; Expected: Successful assembly with multiply operations

CODE
    LD AX, 10
    LD BX, 5
    MUL AX, 5           ; AX = 50
    MUL AX, BX          ; AX = AX * BX
    HALT
