; Test: ROR instruction for right rotation
; Source: InstructionSet.md line 1530
; Category: quick

CODE
    LD AX, 0x0001
    ROR AX, 1           ; AX = 0x8000 (bit 0 wraps to bit 15)
    HALT
