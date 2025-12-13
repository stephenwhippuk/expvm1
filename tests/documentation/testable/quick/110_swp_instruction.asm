; Test: SWP Swap Registers
; Source: InstructionSet.md line 351
; Purpose: Test swapping contents between two registers
; Expected: Successful assembly with register swap

CODE
    LD AX, 100
    LD BX, 200
    SWP AX, BX          ; AX=200, BX=100
    HALT
