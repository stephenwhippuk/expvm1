; ============================================
; TEST: twos_complement
; SOURCE: Syntax.md (line 423)
; DESCRIPTION: Two's complement representation
; EXPECTED: AX=0xFFFF (-1), BX=0xFFFE (-2)
; ============================================

CODE
    PUSHW 0xFFFF
    PUSHB 0xFF
    LD AX, 0xFFFF
    LD BX, 0xFFFE
    HALT
