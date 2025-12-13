; ============================================
; TEST: countdown_loop
; SOURCE: Examples.md (line 210)
; DESCRIPTION: Loop with counter
; EXPECTED: CX = 0 after loop
; ============================================

DATA
    counter: DW [0]

CODE
    LD CX, 10

countdown_loop:
    DEC CX
    JPNZ countdown_loop
    
    LD [counter], CX
    HALT
