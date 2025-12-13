; Test: ADD Arithmetic Operation
; Source: InstructionSet.md line 758
; Purpose: Test addition with immediate and register operands
; Expected: Successful assembly with add operations and flag updates

CODE
    LD AX, 100
    LD BX, 50
    ADD AX, 50          ; AX = 150
    ADD AX, BX          ; AX = AX + BX
    HALT
