; ============================================
; TEST: register_indexed_addressing
; SOURCE: Expressions.md (line 184)
; DESCRIPTION: Dynamic array indexing with register
; EXPECTED: Loads array[0], array[3], array[4]
; ============================================

DATA
    array: DB [10, 20, 30, 40, 50]

CODE
    LD CX, 0
    LDAB AL, (array + CX)
    
    LD CX, 3
    LDAB AL, (array + CX)
    
    INC CX
    LDAB AL, (array + CX)
    
    HALT
