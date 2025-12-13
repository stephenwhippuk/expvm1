; Test: Valid Expression
; Source: ErrorReference.md line 630
; Purpose: Test valid address expressions with offset
; Expected: Successful assembly with label + offset expression

DATA
    val1: DW [10]
    val2: DW [20]

CODE
    LDA AX, val1
    ADD AX, [val2]
    HALT
