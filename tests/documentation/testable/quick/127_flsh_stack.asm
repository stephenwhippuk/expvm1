; Test: FLSH Flush Stack
; Source: InstructionSet.md line 697
; Purpose: Test clearing stack back to frame pointer
; Expected: Successful assembly with stack flush

CODE
    PUSHW 10
    PUSHW 20
    PUSHW 30
    FLSH                ; All three values removed
    HALT
