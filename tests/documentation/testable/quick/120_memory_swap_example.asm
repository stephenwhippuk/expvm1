; Test: Memory Swap Example
; Source: Examples.md line 788
; Purpose: Test swapping two memory locations
; Expected: Successful assembly with memory swap

DATA
    var1: DW [100]
    var2: DW [200]

CODE
    ; Load both values
    LDA AX, var1           ; AX = 100
    LDA BX, var2           ; BX = 200
    
    ; Swap
    LDA var1, BX           ; var1 = 200
    LDA var2, AX           ; var2 = 100
    
    HALT
