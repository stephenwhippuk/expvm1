; Test: AND instruction with immediate and register operands
; Source: InstructionSet.md line 1090
; Category: quick

CODE
    LD AX, 0xFF0F
    AND AX, 0x0FFF      ; AX = 0x0F0F
    LD BX, 0x00FF
    AND AX, BX          ; AX = AX & BX
    HALT
