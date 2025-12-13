; Test: Consistent Indentation
; Source: Syntax.md line 575
; Purpose: Test readable indented code style
; Expected: Successful assembly with indented formatting

DATA
    value: DW [42]
    buffer: DB [1, 2, 3]

CODE
start:
    LD AX, 10
    HALT
