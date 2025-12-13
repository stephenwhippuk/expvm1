; Test debug syscall using workaround - use immediate value in register

DATA
result1: DW [0]

CODE
    ; Use CMP to test and print results directly
    LD AX, 0xFFFF  ; -1
    CMP AX, 1      ; Should return -1 or 1 depending on signed/unsigned
    
    ; Now we have result in AX, push it and print
    PUSH AX
    SYS 0x1500
    
    HALT
