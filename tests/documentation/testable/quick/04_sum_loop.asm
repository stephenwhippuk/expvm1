; ============================================
; TEST: sum_loop
; SOURCE: Examples.md (line 234)
; DESCRIPTION: Sum from 1 to 10
; EXPECTED: sum = 55
; ============================================

DATA
    sum: DW [0]

CODE
    LD AX, 0
    LD CX, 10

sum_loop:
    ADD AX, CX
    DEC CX
    JPNZ sum_loop
    
    LDA sum, AX
    HALT
