; Test: Complete Binary Example
; Source: BinaryFormat.md line 245
; Purpose: Test complete program with data and code segments
; Expected: Successful assembly with proper binary structure

DATA
    value: DW [0x1234]

CODE
    LDA AX, value
    HALT
