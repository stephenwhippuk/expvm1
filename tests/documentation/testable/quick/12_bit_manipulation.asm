; ============================================
; TEST: bit_manipulation
; SOURCE: Examples.md (line 869)
; DESCRIPTION: Set, clear, and toggle bits
; EXPECTED: flags = 0x0000
; ============================================

DATA
    flags: DW [0x0000]

CODE
    LD AX, [flags]
    OR AX, 0x0001
    OR AX, 0x0010
    AND AX, 0xFFFE
    XOR AX, 0x0010
    LD [flags], AX
    HALT
