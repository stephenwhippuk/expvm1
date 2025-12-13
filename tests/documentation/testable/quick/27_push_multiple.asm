; ============================================
; TEST: push_multiple
; SOURCE: InstructionSet.md (line 523)
; DESCRIPTION: Push multiple values to stack
; EXPECTED: Stack contains [100, 200]
; ============================================

CODE
    LD AX, 100
    PUSH AX
    LD AX, 200
    PUSH AX
    HALT
