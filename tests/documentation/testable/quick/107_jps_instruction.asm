; Test: JPS Jump if Sign
; Source: InstructionSet.md line 188
; Purpose: Test conditional jump on sign flag (negative result)
; Expected: Successful assembly with sign-based jump

CODE
    LD AX, 10
    SUB AX, 20      ; Result negative, SIGN set
    JPS negative    ; Jump taken
    HALT
negative:
    HALT
