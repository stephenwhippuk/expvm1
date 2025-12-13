; ============================================
; TEST: jpz_jump_if_zero
; SOURCE: InstructionSet.md (line 133)
; DESCRIPTION: Jump if zero flag set
; EXPECTED: AX=1 (jump taken when AX-AX=0)
; ============================================

CODE
    LD AX, 10
    SUB AX, 10
    JPZ is_zero
    HALT
is_zero:
    LD AX, 1
    HALT
