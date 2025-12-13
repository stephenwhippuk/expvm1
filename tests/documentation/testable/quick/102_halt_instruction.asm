; Test: HALT Instruction
; Source: InstructionSet.md line 76
; Purpose: Test HALT (stop execution) instruction
; Expected: Successful assembly with halt

CODE
    LD AX, 42
    HALT            ; Stop here
