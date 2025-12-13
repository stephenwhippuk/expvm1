; Test: Multi-Register Calculation
; Source: Registers.md line 399
; Purpose: Test calculation using multiple registers
; Expected: Successful assembly with multi-register arithmetic

DATA
    a: DW [10]
    b: DW [20]
    c: DW [30]
    d: DW [5]
    result: DW [0]

CODE
    ; Calculate: result = (a + b) * (c - d)
    LDA AX, a
    ADD AX, b         ; AX = a + b
    
    LDA DX, c
    SUB DX, d         ; DX = c - d
    
    MUL AX, DX          ; AX = (a + b) * (c - d)
    LDA result, AX
    HALT
