; ============================================
; TEST: indexed_addressing
; SOURCE: Expressions.md (line 155)
; DESCRIPTION: Access array elements with fixed offsets
; EXPECTED: AX=100, BX=200, CX=300
; ============================================

DATA
    array: DW [100, 200, 300, 400, 500]

CODE
    LDA AX, array
    LDA BX, (array + 2)
    LDA CX, (array + 4)
    HALT
