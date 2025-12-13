; ============================================
; TEST: register_swap
; SOURCE: Registers.md (line 516)
; DESCRIPTION: Swap two registers
; EXPECTED: AX=200, BX=100
; ============================================

CODE
    LD AX, 100
    LD BX, 200
    SWP AX, BX
    HALT
