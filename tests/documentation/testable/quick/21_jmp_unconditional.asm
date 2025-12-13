; ============================================
; TEST: jmp_unconditional
; SOURCE: InstructionSet.md (line 105)
; DESCRIPTION: Unconditional jump instruction
; EXPECTED: AX=10 (skips AX=20)
; ============================================

CODE
start:
    LD AX, 10
    JMP end
    LD AX, 20
end:
    HALT
