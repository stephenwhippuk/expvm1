; Test: NOT instruction for bitwise complement
; Source: InstructionSet.md line 1280
; Category: quick

CODE
    LD AX, 0x00FF
    NOT AX              ; AX = 0xFF00
    LD BX, 0xAAAA
    NOT BX              ; AX = ~BX
    HALT
