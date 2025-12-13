; Test: JPNZ Jump if Not Zero
; Source: InstructionSet.md line 135
; Purpose: Test loop with JPNZ for non-zero condition
; Expected: Successful assembly with countdown loop

CODE
    LD CX, 10
loop:
    DEC CX          ; Decrement counter
    JPNZ loop       ; Continue if CX != 0
    HALT
