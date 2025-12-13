; Test: Compare Register and Immediate
; Source: InstructionSet.md line 1627
; Purpose: Test CMP instruction with various comparisons
; Expected: Successful assembly with comparison operations

CODE
    LD AX, 10
    LD BX, 20
    CMP AX, BX          ; AX = -1 (10 < 20)
    
    LD CX, 50
    CMP CX, 50          ; AX = 0 (50 == 50)
    
    LD DX, 100
    CMP DX, 50          ; AX = 1 (100 > 50)
    HALT
