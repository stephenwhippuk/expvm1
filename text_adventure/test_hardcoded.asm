DATA
    msg: DB "Hi!"
    
CODE
start:
    ; msg is at offset 0, +2 for size prefix = 2
    ; String is 3 bytes: offsets 2,3,4
    ; Push in reverse: offset 4, 3, 2
    
    LDAB AL, (msg+4)
    PUSHL AL
    LDAB AL, (msg+3)
    PUSHL AL
    LDAB AL, (msg+2)
    PUSHL AL
    
    LD AX, 3     ; Length of string
    PUSH AX      ; Push length for syscall
    SYS 0x0011   ; Print with newline
    HALT
