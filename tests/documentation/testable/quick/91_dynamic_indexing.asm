; Test: Dynamic Array Indexing
; Source: Expressions.md line 355
; Purpose: Test dynamic index with register-based offset
; Expected: Successful assembly with runtime indexing

DATA
    array: DB [10, 20, 30, 40, 50]
    index: DW [0]

CODE
    LD CX, [index]              ; Load index value
    LDAB AL, [array + CX]       ; Load array[index]
    
    ; Increment index
    INC CX
    LD [index], CX              ; Store new index
    LDAB AL, [array + CX]       ; Load array[index+1]
    HALT
