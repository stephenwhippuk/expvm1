; Test: INC instruction for incrementing registers
; Source: InstructionSet.md line 1580
; Category: quick

CODE
    LD CX, 0
    INC CX              ; CX = 1
    INC CX              ; CX = 2
    HALT
