; ============================================
; TEST: push_pop
; SOURCE: InstructionSet.md (line 573)
; DESCRIPTION: Stack push and pop operations
; EXPECTED: AX restored to original value
; ============================================

CODE
    LD AX, 100
    PUSH AX
    LD AX, 50
    POP AX
    HALT
