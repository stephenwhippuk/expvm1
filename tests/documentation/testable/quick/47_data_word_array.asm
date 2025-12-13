; ============================================
; TEST: data_word_array
; SOURCE: Syntax.md (line 208)
; DESCRIPTION: Define word (16-bit) arrays
; EXPECTED: Arrays with proper 16-bit values
; ============================================

DATA
    coordinates: DW [100, 200, 300]
    addresses: DW [0x0000, 0x1000, 0x2000]

CODE
    LDA AX, coordinates
    LDA BX, addresses
    HALT
