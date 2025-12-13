; Test: Bit Shifting Multiplication/Division
; Source: Examples.md line 904
; Purpose: Test using shifts for fast multiplication and division by powers of 2
; Expected: Successful assembly with shift operations

DATA
    value: DW [16]
    times_four: DW [0]
    div_by_two: DW [0]

CODE
    LD AX, [value]          ; AX = 16
    
    ; Multiply by 4 (shift left 2 bits)
    SHL AX, 2               ; AX = 64
    LD [times_four], AX
    
    LD AX, [value]          ; AX = 16
    
    ; Divide by 2 (shift right 1 bit)
    SHR AX, 1               ; AX = 8
    LD [div_by_two], AX
    
    HALT
