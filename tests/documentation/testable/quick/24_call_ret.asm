; ============================================
; TEST: call_ret
; SOURCE: InstructionSet.md (line 286)
; DESCRIPTION: Function call and return
; EXPECTED: AX=20 (10*2)
; ============================================

CODE
main:
    LD AX, 10
    CALL multiply_by_two
    HALT

multiply_by_two:
    MUL AX, 2
    RET
