; Test: NOP Instruction
; Source: InstructionSet.md line 60
; Purpose: Test NOP (no operation) instruction
; Expected: Successful assembly with NOP doing nothing

CODE
    NOP             ; Does nothing
    LD AX, 10
    HALT
