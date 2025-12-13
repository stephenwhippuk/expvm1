; Test: REM instruction for remainder/modulo operation
; Source: InstructionSet.md line 1030
; Category: quick

CODE
    LD AX, 100
    REM AX, 7           ; AX = 2 (100 % 7)
    HALT
