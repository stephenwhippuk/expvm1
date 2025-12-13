; Test: Correct Data Type for Range
; Source: ErrorReference.md line 555
; Purpose: Test using word type for values > 255
; Expected: Successful assembly with proper DW usage

DATA
    small: DW [1000]
    small2: DB [255]

CODE
    LDA AX, small
    LDA BX, small2
    HALT
