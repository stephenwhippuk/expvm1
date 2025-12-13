; Test: Array Sum Example
; Source: InstructionSet.md line 1811
; Purpose: Test complete array summation with loop
; Expected: Successful assembly with array processing

DATA
    array: DB [10, 20, 30, 40, 50]
    count: DW [5]
    result: DW [0]

CODE
    LD AX, 0            ; Sum accumulator
    LDA BX, array       ; Array pointer
    LD CX, [count]      ; Element count

sum_loop:
    LDAB DX, (BX)       ; Load byte from array
    ADD AX, DX          ; Add to sum
    INC BX              ; Next element
    DEC CX              ; Decrement count
    JPNZ sum_loop       ; Continue if more elements
    
    LD [result], AX     ; Store result (150)
    HALT
