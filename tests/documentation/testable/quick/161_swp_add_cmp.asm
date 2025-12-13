; Test: Register + register instruction encoding
; Source: BinaryFormat.md line 405
; Category: quick

CODE
    LD AX, 100
    LD BX, 50
    SWP CX, DX
    ADD AX, BX
    CMP AX, BX
    HALT
