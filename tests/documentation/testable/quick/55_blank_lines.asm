; Test: Blank Lines Allowed
; Source: Syntax.md line 459
; Purpose: Test that blank lines are allowed and ignored
; Expected: Successful assembly with blank lines

DATA
    value: DW [42]

                        ; Multiple blank lines OK

CODE
    HALT
