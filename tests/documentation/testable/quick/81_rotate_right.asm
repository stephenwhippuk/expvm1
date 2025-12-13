; Test: Rotate Right Operation
; Source: InstructionSet.md line 1520
; Purpose: Test ROR instruction for bit rotation
; Expected: Successful assembly with rotate right

CODE
    LD AX, 0x0001
    ROR AX, 1           ; AX = 0x8000 (bit 0 wraps to bit 15)
    HALT
