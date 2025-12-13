; Test: SHL instruction for left shift
; Source: InstructionSet.md line 1340
; Category: quick

CODE
    LD AX, 0x0001
    SHL AX, 4           ; AX = 0x0010 (multiply by 16)
    HALT
