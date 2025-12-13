; ============================================
; TEST: basic_arithmetic
; SOURCE: Examples.md (line 92)
; DESCRIPTION: Arithmetic operations
; EXPECTED: result = 27
; ============================================

DATA
    result: DW [0]

CODE
    LD AX, 10
    ADD AX, 5
    MUL AX, 2
    SUB AX, 3
    LDA result, AX
    HALT
