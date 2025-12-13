; Test: Circular Bit Rotation
; Source: Examples.md line 947
; Purpose: Test ROL and ROR for circular bit shifts
; Expected: Successful assembly with rotation operations

DATA
    value: DW [0x8001]
    rotated_left: DW [0]
    rotated_right: DW [0]

CODE
    LD AX, [value]          ; AX = 0x8001
    
    ; Rotate left 1 bit
    ROL AX, 1               ; AX = 0x0003 (bit 15 wraps to bit 0)
    LD [rotated_left], AX
    
    LD AX, [value]          ; AX = 0x8001
    
    ; Rotate right 1 bit
    ROR AX, 1               ; AX = 0xC000 (bit 0 wraps to bit 15)
    LD [rotated_right], AX
    
    HALT
