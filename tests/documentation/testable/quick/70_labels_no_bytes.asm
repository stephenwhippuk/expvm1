; Test: Labels Don't Generate Bytes
; Source: BinaryFormat.md line 230
; Purpose: Test that labels are just address markers, not code
; Expected: Successful assembly, only NOP and HALT bytes generated

CODE
start:
    NOP
loop:
    HALT
