; Minimal test - print nothing
CODE
start:
    ; Push count of 0
    LD AX, 0
    PUSH AX
    
    ; Print (should print nothing)
    SYS 0x0010
    
    HALT
