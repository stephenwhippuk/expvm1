; Test: Register + immediate word encoding
; Source: BinaryFormat.md line 377
; Category: quick

CODE
    LD AX, 100
    ADD AX, 50
    SUB AX, 0x1234
    HALT
