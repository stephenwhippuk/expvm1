; ============================================
; TEST: count_up
; SOURCE: Examples.md (line 267, variation)
; DESCRIPTION: Count up from 0 to 9
; EXPECTED: final_count = 10
; ============================================

DATA
    final_count: DW [0]

CODE
    LD AX, 0
    LD CX, 10

count_up_loop:
    INC AX
    DEC CX
    JPNZ count_up_loop
    
    LDA final_count, AX
    HALT
