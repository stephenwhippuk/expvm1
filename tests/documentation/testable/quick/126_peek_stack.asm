; Test: PEEK Stack Inspection
; Source: InstructionSet.md line 644
; Purpose: Test peeking at stack values without popping
; Expected: Successful assembly with stack peek operations

CODE
    PUSHW 100
    PUSHW 200
    PEEK AX, 0          ; AX = 200 (top of stack)
    PEEK BX, 2          ; BX = 100 (previous value)
    HALT
