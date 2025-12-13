; Test: Register + immediate byte encoding
; Source: BinaryFormat.md line 387
; Category: quick

CODE
    LD AX, 0x0000
    LDH AH, 0x12
    LDL AL, 0x34
    ADB AX, 10
    HALT
