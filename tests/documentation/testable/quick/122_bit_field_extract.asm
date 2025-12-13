; Test: Extract Bit Field
; Source: Examples.md line 929
; Purpose: Test extracting specific bits from a value using shift and mask
; Expected: Successful assembly with bit field extraction

DATA
    value: DW [0xABCD]
    extracted: DW [0]

CODE
    LDA AX, value          ; AX = 0xABCD
    
    ; Shift right 4 bits
    SHR AX, 4               ; AX = 0x0ABC
    
    ; Mask to keep only lower 4 bits
    AND AX, 0x000F          ; AX = 0x000C
    
    LDA extracted, AX      ; extracted = 0x000C
    HALT
