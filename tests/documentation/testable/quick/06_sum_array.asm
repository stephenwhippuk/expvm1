; ============================================
; TEST: sum_array
; SOURCE: Examples.md (line 267)
; DESCRIPTION: Sum byte array elements
; EXPECTED: sum = 150 (10+20+30+40+50)
; ============================================

DATA
    array: DB [10, 20, 30, 40, 50]
    count: DW [5]
    sum: DW [0]

CODE
    LD AX, 0
    LDA CX, count
    LD BX, 0

sum_array_loop:
    LDAB DX, (array + 2 + BX)
    ADD AX, DX
    INC BX
    DEC CX
    JPNZ sum_array_loop
    
    LDA sum, AX
    HALT
