; ============================================
; TEST: pushb_immediate
; SOURCE: InstructionSet.md (line 638)
; DESCRIPTION: Push immediate byte values
; EXPECTED: Stack contains byte constants
; ============================================

CODE
    PUSHB 0x42
    PUSHB 10
    HALT
