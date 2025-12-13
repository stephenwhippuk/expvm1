; ============================================
; TEST: add_immediate_register
; SOURCE: InstructionSet.md (line 784)
; DESCRIPTION: Add immediate and register to AX
; EXPECTED: AX = 150, then AX = 150 + BX
; ============================================

CODE
    LD AX, 100
    ADD AX, 50
    LD BX, 25
    ADD AX, BX
    HALT
