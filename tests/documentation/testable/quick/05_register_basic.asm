; ============================================
; TEST: register_basic
; SOURCE: Registers.md (line 337)
; DESCRIPTION: Basic register operations
; EXPECTED: Assembles successfully
; ============================================

CODE
    LD AX, 100
    LD BX, 200
    ADD AX, BX
    HALT
