; Test: SHR instruction for right shift (logical)
; Source: InstructionSet.md line 1380
; Category: quick

CODE
    LD AX, 0x0010
    SHR AX, 2           ; AX = 0x0004 (divide by 4)
    HALT
