; Test: Function call with square calculation
; Source: InstructionSet.md line 1858
; Category: quick

DATA
value: DW [10]

CODE
main:
    LDA AX, value
    CALL square         ; Call square function
    LDA value, AX      ; Store result (100)
    HALT

square:
    MUL AX, AX          ; AX = AX * AX
    RET
