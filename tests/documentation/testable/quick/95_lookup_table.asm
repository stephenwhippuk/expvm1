; Test: Lookup Table
; Source: Expressions.md line 616
; Purpose: Test table lookup with calculated offset
; Expected: Successful assembly with table access

DATA
    ; Square lookup table for 0-9
    squares: DW [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]
    input: DW [5]
    result: DW [0]

CODE
    LDA CX, input              ; CX = input value
    
    ; Each word is 2 bytes, so multiply index by 2
    LD AX, CX
    MUL AX, 2                   ; AX = index * 2 (byte offset)
    
    LDA BX, (squares + AX)       ; Load squares[input]
    LDA result, BX             ; Store result
    
    HALT                        ; result = 25 (square of 5)
