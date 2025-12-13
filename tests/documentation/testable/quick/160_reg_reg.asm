; Test: Register + register encoding
; Source: BinaryFormat.md line 398
; Category: quick

CODE
    LD AX, 100
    LD BX, 50
    LD AX, BX
    HALT
