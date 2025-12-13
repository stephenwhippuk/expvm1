; ============================================
; TEST: jpnz_jump_if_not_zero
; SOURCE: InstructionSet.md (line 162)
; DESCRIPTION: Jump if not zero (loop pattern)
; EXPECTED: CX=0 after loop
; ============================================

CODE
    LD CX, 10
loop:
    DEC CX
    JPNZ loop
    HALT
