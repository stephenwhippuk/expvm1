; Test: OR instruction with immediate and register operands
; Source: InstructionSet.md line 1160
; Category: quick

CODE
    LD AX, 0xFF00
    OR AX, 0x00FF       ; AX = 0xFFFF
    LD BX, 0x0001
    OR AX, BX           ; AX = AX | BX
    HALT
