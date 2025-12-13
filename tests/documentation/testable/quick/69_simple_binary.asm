; Test: Simple Binary Example
; Source: BinaryFormat.md line 210
; Purpose: Test simple load and halt program encoding
; Expected: Successful assembly generating 5-byte code segment

CODE
start:
    LD AX, 100
    HALT
