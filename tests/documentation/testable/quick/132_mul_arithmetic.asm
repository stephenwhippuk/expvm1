; Test: MUL instruction with immediate and register operands
; Source: InstructionSet.md line 900
; Category: quick

CODE
    LD AX, 10
    MUL AX, 5           ; AX = 50
    LD BX, 3
    MUL AX, BX          ; AX = AX * BX
    HALT
