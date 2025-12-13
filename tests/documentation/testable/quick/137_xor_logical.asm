; Test: XOR instruction with immediate and register operands
; Source: InstructionSet.md line 1220
; Category: quick

CODE
    LD AX, 0xFFFF
    XOR AX, 0xFFFF      ; AX = 0x0000 (quick clear)
    LD AX, 0xAAAA
    LD BX, 0x5555
    XOR AX, BX          ; AX = AX ^ BX
    HALT
