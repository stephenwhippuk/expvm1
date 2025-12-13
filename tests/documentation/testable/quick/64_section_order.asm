; Test: Correct Section Order
; Source: Syntax.md line 550
; Purpose: Test proper DATA before CODE section order
; Expected: Successful assembly with correct section ordering

DATA
    value: DW [42]

CODE
    HALT
