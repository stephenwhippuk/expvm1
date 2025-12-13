; Test: Function call with square calculation
; Source: InstructionSet.md line 1858
; Category: quick

DATA
value: DW [10]

CODE
main:
    LD AX, [value]
    CALL square         ; Call square function
    LD [value], AX      ; Store result (100)
    HALT

square:
    MUL AX, AX          ; AX = AX * AX
    RET
