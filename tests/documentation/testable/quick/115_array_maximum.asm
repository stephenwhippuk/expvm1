; Test: Array Maximum Example
; Source: Examples.md line 314
; Purpose: Test finding maximum value in array
; Expected: Successful assembly with array maximum algorithm

DATA
    numbers: DW [23, 67, 12, 89, 45, 34]
    count: DW [6]
    maximum: DW [0]

CODE
    ; Initialize with first element
    LDA AX, (numbers + 2)    ; AX = current max
    LDA CX, count          ; CX = count
    DEC CX                  ; Already processed first element
    LD BX, 4                ; BX = index (words are 2 bytes, +2 for size prefix)

find_max_loop:
    ; Load current element
    LDA DX, (numbers + 2 + BX)
    
    ; Compare with current max
    CMP DX, AX              ; Compare DX with AX
    JPZ update_max          ; If DX > AX (result = 1), update
    JMP continue_max
    
update_max:
    LD AX, DX               ; New maximum
    
continue_max:
    ; Next element (words are 2 bytes apart)
    ADD BX, 2
    DEC CX
    JPNZ find_max_loop
    
    LDA maximum, AX        ; maximum = 89
    HALT
