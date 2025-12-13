; ============================================
; TEST: inc_increment
; SOURCE: InstructionSet.md (line 1584)
; DESCRIPTION: Increment register
; EXPECTED: CX = 2
; ============================================

CODE
    LD CX, 0
    INC CX
    INC CX
    HALT
