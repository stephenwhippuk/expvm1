; ============================================
; TEST: mul_operations
; SOURCE: InstructionSet.md (line 908)
; DESCRIPTION: Multiply AX by immediate and register
; EXPECTED: AX = 50, then AX = 50 * BX
; ============================================

CODE
    LD AX, 10
    MUL AX, 5
    LD BX, 2
    MUL AX, BX
    HALT
