; ============================================
; TEST: data_byte_array
; SOURCE: Syntax.md (line 126)
; DESCRIPTION: Define byte array
; EXPECTED: Array with 5 elements
; ============================================

DATA
    array: DB [1, 2, 3, 4, 5]
    flags: DB [0x01, 0x02, 0x04, 0x08]

CODE
    LDAB AX, [array + 0]
    LDAB BX, [flags + 0]
    HALT
