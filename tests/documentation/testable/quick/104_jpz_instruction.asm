; Test: JPZ Jump if Zero
; Source: InstructionSet.md line 113
; Purpose: Test conditional jump on zero flag
; Expected: Successful assembly with zero-based jump

CODE
    LD AX, 10
    SUB AX, 10      ; AX = 0, ZERO flag set
    JPZ is_zero     ; Jump taken
    HALT            ; Skipped
is_zero:
    LD AX, 1
    HALT
