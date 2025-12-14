; Test: Array sum example
; Source: InstructionSet.md line 1825
; Category: quick

DATA
array: DB [10, 20, 30, 40, 50]
count: DW [5]

CODE
    LD AX, 0            ; Sum accumulator
    LD BX, (array + 2)        ; Array pointer (skip size prefix)
    LDA CX, count      ; Counter
loop:
    LDAB DX, BX       ; Load byte from array
    ADD AX, DX          ; Add to sum
    INC BX              ; Next element
    DEC CX              ; Decrement counter
    JPNZ loop           ; Continue if more elements
    HALT
