; ============================================
; TEST: bit_extraction
; SOURCE: Examples.md (line 926)
; DESCRIPTION: Extract bits 4-7 from value
; EXPECTED: extracted = 0x000C
; ============================================

DATA
    value: DW [0xABCD]
    extracted: DW [0]

CODE
    LDA AX, value
    SHR AX, 4
    AND AX, 0x000F
    LDA extracted, AX
    HALT
