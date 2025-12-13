; ============================================
; TEST: multi_register_calc
; SOURCE: Registers.md (line 413)
; DESCRIPTION: Multi-register calculation
; EXPECTED: result = (a+b)*(c-d) = (10+5)*(20-8) = 180
; ============================================

DATA
    a: DW [10]
    b: DW [5]
    c: DW [20]
    d: DW [8]
    result: DW [0]

CODE
    LDA AX, a
    ADD AX, b
    
    LDA DX, c
    SUB DX, d
    
    MUL AX, DX
    LDA result, AX
    
    HALT
