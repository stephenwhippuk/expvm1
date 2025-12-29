CODE
start:
    ; Push "Hi" manually
    PUSHB 0x69  ; 'i'
    PUSHB 0x48  ; 'H'
    LD AX, 2    ; Length
    PUSH AX
    SYS 0x0011  ; Print with newline
    HALT
