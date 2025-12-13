; Test: Missing Colon Error Example (Valid Version)
; Source: Syntax.md line 520
; Purpose: Test proper label with colon in DATA section
; Expected: Successful assembly with correct label syntax

DATA
    value: DW [42]

CODE
    LDA AX, value
    HALT
