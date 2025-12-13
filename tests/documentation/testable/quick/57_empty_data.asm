; Test: Empty Data Segment
; Source: BinaryFormat.md line 156
; Purpose: Test program with no DATA section
; Expected: Successful assembly with zero-size data segment

CODE
    LD AX, 100
    LD BX, 200
    ADD AX, BX
    HALT
