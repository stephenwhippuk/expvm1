; Test: JPC Jump if Carry
; Source: InstructionSet.md line 155
; Purpose: Test conditional jump on carry flag (overflow)
; Expected: Successful assembly with carry-based jump

CODE
    LD AL, 255
    ADD AL, 1       ; Overflow sets CARRY
    JPC overflow    ; Jump taken
    HALT
overflow:
    HALT
