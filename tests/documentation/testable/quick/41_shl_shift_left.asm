; ============================================
; TEST: shl_shift_left
; SOURCE: InstructionSet.md (line 1345)
; DESCRIPTION: Shift left (multiply by 16)
; EXPECTED: AX = 0x0010
; ============================================

CODE
    LD AX, 0x0001
    SHL AX, 4
    HALT
