; Test sub-register operations (high/low byte)

DATA
    byte_val: DB [0x42]

CODE
start:
    ; Test loading into sub-registers
    LDH AX, 0xFF        ; Load 0xFF into high byte of AX
    LDL AX, 0x00        ; Load 0x00 into low byte of AX
    ; AX should now be 0xFF00
    
    ; Test high/low byte arithmetic
    ADH BX              ; Add high byte of BX to high byte of AX
    ADL CX              ; Add low byte of CX to low byte of AX
    
    ; Test push/pop with sub-registers
    PUSHH AX            ; Push high byte of AX
    PUSHL AX            ; Push low byte of AX
    
    POPL DX             ; Pop into low byte of DX
    POPH DX             ; Pop into high byte of DX
    
    ; Test sub-register comparisons  
    CPH AX, BX          ; Compare high bytes
    CPL CX, DX          ; Compare low bytes
    
    HALT
