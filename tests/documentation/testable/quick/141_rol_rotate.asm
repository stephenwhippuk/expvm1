; Test: ROL instruction for left rotation
; Source: InstructionSet.md line 1470
; Category: quick

CODE
    LD AX, 0x8001
    ROL AX, 1           ; AX = 0x0003 (bit 15 wraps to bit 0)
    HALT
