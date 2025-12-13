; ============================================
; TEST: remainder_operations
; SOURCE: InstructionSet.md (line 1030)
; DESCRIPTION: Remainder (modulo) operation
; EXPECTED: AX = 2 (100 % 7)
; ============================================

CODE
    LD AX, 100
    REM AX, 7
    HALT
