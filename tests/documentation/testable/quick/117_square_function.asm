; Test: Square Function Example
; Source: Examples.md line 542
; Purpose: Test function that squares a number
; Expected: Successful assembly with function call pattern

DATA
    input: DW [7]
    result: DW [0]

CODE
main:
    ; Load input
    LDA AX, input
    
    ; Call square function
    CALL square
    
    ; Store result
    LDA result, AX         ; result = 49
    HALT

; Function: square
; Input: AX = number
; Output: AX = number squared
square:
    MUL AX, AX              ; AX = AX * AX
    RET
