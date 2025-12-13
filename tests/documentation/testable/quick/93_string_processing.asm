; Test: String Processing Example
; Source: Expressions.md line 537
; Purpose: Test string character sum with indirect addressing
; Expected: Successful assembly with string processing

DATA
    message: DB "Hello, World!"
    length: DW [13]

CODE
    LDA BX, message             ; BX = string address
    LD CX, [length]             ; CX = string length
    LD AX, 0                    ; AX = sum accumulator

sum_loop:
    LDAB DX, (BX)               ; Load character (indirect)
    ADD AX, DX                  ; Add to sum
    INC BX                      ; Next character
    DEC CX                      ; Decrement counter
    JPNZ sum_loop               ; Continue if more characters
    
    HALT                        ; AX = sum of ASCII values
