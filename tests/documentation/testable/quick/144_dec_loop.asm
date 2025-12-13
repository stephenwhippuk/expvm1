; Test: DEC instruction with loop
; Source: InstructionSet.md line 1610
; Category: quick

CODE
    LD CX, 10
loop:
    DEC CX              ; CX = CX - 1
    JPNZ loop           ; Continue if CX != 0
    HALT
