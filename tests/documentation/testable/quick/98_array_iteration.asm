; Test: Array Iteration with Sum
; Source: Registers.md line 378
; Purpose: Test iterating array with pointer and accumulator
; Expected: Successful assembly with array summation

DATA
    array: DB [10, 20, 30, 40, 50]
    sum: DW [0]

CODE
    ; Sum array elements
    LD AX, 0            ; AX = accumulator (sum)
    LDA BX, array       ; BX = array base pointer
    LD CX, 5            ; CX = element count
    
sum_loop:
    LDAB DL, (BX)       ; Load array element into DL
    ADD AL, DL          ; Add to sum (byte addition)
    INC BX              ; Move to next element
    DEC CX              ; Decrement counter
    JPNZ sum_loop       ; Continue if CX != 0
    
    LD [sum], AX        ; Store result
    HALT
