; ============================================
; TEST: shr_shift_right
; SOURCE: InstructionSet.md (line 1405)
; DESCRIPTION: Shift right (divide by 16)
; EXPECTED: AX = 0x0010
; ============================================

CODE
    LD AX, 0x0100
    SHR AX, 4
    HALT
