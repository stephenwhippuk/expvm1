; Test: Quick start hello world example
; Source: Overview.md line 40
; Category: quick

DATA
message: DB "Hello, World!"

CODE
start:
    LD CX, 0x0B
loop:
    DEC CX
    LDA AX, (message + CX)
    PUSHB AX
    CMP CX, 0x00
    JPNZ loop
    PUSHW 0x000B
    SYS 0x0011
    HALT
