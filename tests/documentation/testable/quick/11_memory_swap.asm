; ============================================
; TEST: memory_swap
; SOURCE: Examples.md (line 806)
; DESCRIPTION: Swap two memory locations
; EXPECTED: var1 = 200, var2 = 100
; ============================================

DATA
    var1: DW [100]
    var2: DW [200]

CODE
    LDA AX, var1
    LDA BX, var2
    LDA var1, BX
    LDA var2, AX
    HALT
