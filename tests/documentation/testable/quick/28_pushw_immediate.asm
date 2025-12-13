; ============================================
; TEST: pushw_immediate
; SOURCE: InstructionSet.md (line 619)
; DESCRIPTION: Push immediate word values
; EXPECTED: Stack contains constants
; ============================================

CODE
    PUSHW 0x1234
    PUSHW 42
    HALT
