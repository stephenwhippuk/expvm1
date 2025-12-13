; Test: Valid Identifier
; Source: Syntax.md line 540
; Purpose: Test proper identifier naming with digits after letters
; Expected: Successful assembly with valid identifier

DATA
    value123: DW [42]

CODE
    LD AX, [value123]
    HALT
