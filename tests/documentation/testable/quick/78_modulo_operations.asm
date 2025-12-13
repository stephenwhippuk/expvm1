; Test: Modulo Operations
; Source: InstructionSet.md line 1023
; Purpose: Test REM instruction for remainder calculation
; Expected: Successful assembly with modulo operation

CODE
    LD AX, 100
    REM AX, 7           ; AX = 2 (100 % 7)
    HALT
