; Simple test - just print a string
DATA
    PAGE test_data
    test_msg: DB "Hello World"
    
CODE
start:
    ; Push characters in reverse order
    PUSHB 100  ; 'd'
    PUSHB 108  ; 'l'
    PUSHB 114  ; 'r'
    PUSHB 111  ; 'o'
    PUSHB 87   ; 'W'
    PUSHB 32   ; ' '
    PUSHB 111  ; 'o'
    PUSHB 108  ; 'l'
    PUSHB 108  ; 'l'
    PUSHB 101  ; 'e'
    PUSHB 72   ; 'H'
    
    ; Push count
    LD AX, 11
    PUSH AX
    
    ; Print
    SYS 0x0010
    
    HALT
