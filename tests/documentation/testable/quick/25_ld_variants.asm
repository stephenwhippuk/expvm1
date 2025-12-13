; ============================================
; TEST: ld_variants
; SOURCE: InstructionSet.md (line 341)
; DESCRIPTION: Load immediate and register-to-register
; EXPECTED: AX=1000, BX=1000, CX=0xFF00
; ============================================

CODE
    LD AX, 1000
    LD BX, AX
    LD CX, 0xFF00
    HALT
