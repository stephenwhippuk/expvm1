; Test: Sequential Data Layout
; Source: BinaryFormat.md line 141
; Purpose: Test that data is laid out sequentially as defined
; Expected: Successful assembly with mixed byte and word data

DATA
    byte1: DB [0x42]
    word1: DW [0x1234]
    array: DB [10, 20, 30]

CODE
    LD AX, [byte1]
    LD BX, [word1]
    HALT
