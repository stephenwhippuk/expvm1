; ============================================
; TEST: indexed_addressing
; SOURCE: Expressions.md (line 155)
; DESCRIPTION: Access array elements with fixed offsets
; EXPECTED: AX=100, BX=200, CX=300
; ============================================

DATA
    array: DW [100, 200, 300, 400, 500]

CODE
    LD AX, [array + 0]
    LD BX, [array + 2]
    LD CX, [array + 4]
    HALT
