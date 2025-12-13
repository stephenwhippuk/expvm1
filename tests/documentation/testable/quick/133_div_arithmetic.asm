; Test: DIV instruction with immediate and register operands
; Source: InstructionSet.md line 960
; Category: quick

CODE
    LD AX, 100
    DIV AX, 3           ; AX = 33 (integer division)
    LD BX, 5
    DIV AX, BX          ; AX = AX / BX
    HALT
