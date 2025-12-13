; Test: Simple loop example with ADD and DEC
; Source: InstructionSet.md line 1810
; Category: quick

CODE
    LD CX, 10           ; Loop 10 times
    LD AX, 0            ; Accumulator
loop:
    ADD AX, 1           ; Increment accumulator
    DEC CX              ; Decrement counter
    JPNZ loop           ; Continue if CX != 0
    HALT                ; AX = 10
