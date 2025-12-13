; Test: SUB instruction with immediate and register operands
; Source: InstructionSet.md line 850
; Category: quick

CODE
    LD AX, 100
    SUB AX, 30          ; AX = 70
    LD BX, 20
    SUB AX, BX          ; AX = AX - BX
    HALT
