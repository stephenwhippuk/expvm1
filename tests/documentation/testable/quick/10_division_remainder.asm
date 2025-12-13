; ============================================
; TEST: division_remainder
; SOURCE: Examples.md (line 165, extended example)
; DESCRIPTION: Division with quotient and remainder
; EXPECTED: quotient = 14, remainder = 2 (100/7)
; ============================================

DATA
    dividend: DW [100]
    divisor: DW [7]
    quotient: DW [0]
    remainder: DW [0]

CODE
    LDA AX, dividend
    DIV AX, divisor
    LDA quotient, AX
    
    LDA AX, dividend
    REM AX, divisor
    LDA remainder, AX
    
    HALT
