; Test: Single register instruction encoding
; Source: BinaryFormat.md line 365
; Category: quick

CODE
    LD AX, 10
    PUSH AX
    POP BX
    INC CX
    DEC DX
    HALT
