DATA
    values: DW [0x1234, 0x5678, 0x9ABC]
    
CODE
start:
    ; Test: load second value (0x5678) using computed address
    LD BX, 1          ; Index 1
    SHL BX, 1         ; Multiply by 2 (word size)
    LD DX, values     ; Base address
    ADD DX, BX        ; DX = values + (1*2)
    LDA AX, DX        ; Load from computed address
    HALT
