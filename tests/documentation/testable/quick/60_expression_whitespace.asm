; Test: Expression Whitespace
; Source: Syntax.md line 454
; Purpose: Test whitespace around operators in expressions
; Expected: Successful assembly with various spacing in expressions

DATA
    buffer: DB [1, 2, 3, 4, 5]

CODE
    LD AX, [buffer+5]   ; Valid - no spaces
    LD BX, [buffer + 5] ; Valid - with spaces
    HALT
