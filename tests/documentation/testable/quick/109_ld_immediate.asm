; Test: LD Load Immediate and Register
; Source: InstructionSet.md line 330
; Purpose: Test loading immediate values and copying between registers
; Expected: Successful assembly with load operations

CODE
    LD AX, 1000         ; Load immediate into AX
    LD BX, AX           ; Copy AX to BX
    LD CX, 0xFF00       ; Load hex value into CX
    HALT
