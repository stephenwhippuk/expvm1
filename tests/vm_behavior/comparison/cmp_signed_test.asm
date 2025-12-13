; Test: Does CMP treat values as signed or unsigned?
; Use 0x8000 which doesn't get optimized

CODE
    ; Load using values that force proper encoding
    LD BX, 0x8000    ; -32768 signed or 32768 unsigned
    LD CX, 0x0100    ; 256
    CMP BX, CX       ; Compare
    ; If signed: AX = 0xFFFF (-32768 < 256)
    ; If unsigned: AX = 0x0001 (32768 > 256)
    
    PUSH AX
    SYS 0x1500
    
    HALT

; Will print:
; 65535 if CMP is signed
; 1 if CMP is unsigned
