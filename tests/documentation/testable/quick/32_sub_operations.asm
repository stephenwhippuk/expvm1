; ============================================
; TEST: sub_operations
; SOURCE: InstructionSet.md (line 849)
; DESCRIPTION: Subtract immediate and register from AX
; EXPECTED: AX = 70, then AX = 70 - BX
; ============================================

CODE
    LD AX, 100
    SUB AX, 30
    LD BX, 10
    SUB AX, BX
    HALT
