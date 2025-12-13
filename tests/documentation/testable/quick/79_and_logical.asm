; Test: AND Logical Operation
; Source: InstructionSet.md line 1078
; Purpose: Test bitwise AND for masking operations
; Expected: Successful assembly with AND operations

CODE
    LD AX, 0xFF0F
    LD BX, 0x0FFF
    AND AX, 0x0FFF      ; AX = 0x0F0F
    AND AX, BX          ; AX = AX & BX
    HALT
