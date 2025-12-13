; ============================================
; TEST: hex_literals
; SOURCE: Syntax.md (line 394)
; DESCRIPTION: Hexadecimal literal support
; EXPECTED: AX=0xFF, BX=0x1234, CX=0xABCD
; ============================================

CODE
    LD AX, 0xFF
    LD BX, 0x1234
    LD CX, 0xABCD
    HALT
