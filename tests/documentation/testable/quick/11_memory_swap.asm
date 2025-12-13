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
    LD AX, [var1]
    LD BX, [var2]
    LD [var1], BX
    LD [var2], AX
    HALT
