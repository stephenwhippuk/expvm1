DATA
    message: DB "Hi"
    msg_len: DW [2]
    
CODE
start:
    ; Push 'i' then 'H'
    PUSHB 0x69
    PUSHB 0x48
    
    ; Load and push length
    LD BX, msg_len      ; BX = address of msg_len DATA block
    ADD BX, 2           ; Skip the 2-byte size prefix
    LDA AX, BX          ; AX = value at address (2)
    PUSH AX
    
    SYS 0x0011
    HALT
