; Test: Function call with square calculation
; Source: InstructionSet.md line 1858
; Category: quick

DATA
value: DW [10]

CODE
main:
    LDA AX, (value + 2)
    CALL square         ; Call square function
    LDA (value + 2), AX      ; Store result (100)
    HALT

square:
    MUL AX, AX          ; AX = AX * AX
    RET
