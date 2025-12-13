; Test: CALL and RET Subroutine
; Source: InstructionSet.md line 239
; Purpose: Test subroutine call and return
; Expected: Successful assembly with function call

CODE
main:
    LD AX, 10
    CALL multiply_by_two
    HALT

multiply_by_two:
    MUL AX, 2
    RET
