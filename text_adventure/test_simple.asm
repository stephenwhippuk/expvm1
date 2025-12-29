CODE
start:
    ; Push 'X' character
    PUSHB 88
    ; Push count = 1
    LD AX, 1
    PUSH AX
    ; Print
    SYS 0x0010
    HALT
