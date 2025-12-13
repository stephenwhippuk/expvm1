; Test: Memory Swap Example
; Source: Examples.md line 788
; Purpose: Test swapping two memory locations
; Expected: Successful assembly with memory swap

DATA
    var1: DW [100]
    var2: DW [200]

CODE
    ; Load both values
    LD AX, [var1]           ; AX = 100
    LD BX, [var2]           ; BX = 200
    
    ; Swap
    LD [var1], BX           ; var1 = 200
    LD [var2], AX           ; var2 = 100
    
    HALT
