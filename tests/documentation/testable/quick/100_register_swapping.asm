; Test: Register Swapping Methods
; Source: Registers.md line 445
; Purpose: Test swapping register values with SWP instruction
; Expected: Successful assembly with register swap

CODE
    ; Swap AX and BX
    LD AX, 100
    LD BX, 200
    
    ; Using SWP instruction
    SWP AX, BX          ; AX = 200, BX = 100
    HALT
