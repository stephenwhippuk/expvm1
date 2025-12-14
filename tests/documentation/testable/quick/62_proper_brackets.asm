; Test: Missing Brackets Error Example (Valid Version)
; Source: Syntax.md line 530
; Purpose: Test proper data array with brackets
; Expected: Successful assembly with correct bracket syntax

DATA
    array: DB [1, 2, 3]

CODE
    LDA AX, array      ; Load address of array (points to size word)
    HALT
