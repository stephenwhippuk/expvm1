; Test: JMP Unconditional Jump
; Source: InstructionSet.md line 92
; Purpose: Test unconditional jump instruction
; Expected: Successful assembly with jump skipping code

CODE
start:
    LD AX, 10
    JMP end         ; Jump to 'end' label
    LD AX, 20       ; Skipped
end:
    HALT
