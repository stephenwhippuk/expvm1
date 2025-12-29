; Test with 1 character
CODE
start:
    ; Push one character
    PUSHB 65  ; 'A'
    
    ; Push count
    LD AX, 1
    PUSH AX
    
    ; Print
    SYS 0x0010
    
    HALT
