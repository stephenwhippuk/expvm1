; Test: OR Logical Operation
; Source: InstructionSet.md line 1119
; Purpose: Test bitwise OR for combining values
; Expected: Successful assembly with OR operations

CODE
    LD AX, 0xFF00
    LD BX, 0x00FF
    OR AX, 0x00FF       ; AX = 0xFFFF
    OR AX, BX           ; AX = AX | BX
    HALT
