; Test: Byte Packing and Unpacking
; Source: Registers.md line 361
; Purpose: Test packing and unpacking bytes in word register
; Expected: Successful assembly with byte manipulation

DATA
    packed: DW [0x1234]

CODE
    ; Unpack a word into separate bytes
    LD AX, [packed]     ; AX = 0x1234
    LD BL, AH           ; BL = 0x12 (high byte)
    LD CL, AL           ; CL = 0x34 (low byte)
    
    ; Pack two bytes into a word
    LD DH, 0xAB
    LD DL, 0xCD
    LD [packed], DX     ; Write 0xABCD to memory
    HALT
