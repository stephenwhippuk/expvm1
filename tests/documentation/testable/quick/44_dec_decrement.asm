; ============================================
; TEST: dec_decrement
; SOURCE: InstructionSet.md (line 1609)
; DESCRIPTION: Decrement register in loop
; EXPECTED: CX = 0 after loop
; ============================================

CODE
    LD CX, 10
loop:
    DEC CX
    JPNZ loop
    HALT
