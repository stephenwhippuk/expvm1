; Simple test program
; Just pushes a value and halts

DATA
    value: DW [0x1234]

CODE
start:
    PUSHW 0x1234
    HALT
