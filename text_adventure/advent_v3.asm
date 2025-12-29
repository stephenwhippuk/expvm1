CODE
start:
    ; Manually push "Hi!" reversed
    PUSHB 0x21  ; !
    PUSHB 0x69  ; i
    PUSHB 0x48  ; H
    
    LD AX, 3
    PUSH AX
    SYS 0x0011
    HALT
