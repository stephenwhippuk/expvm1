; Test: Add Function with Two Parameters
; Source: Examples.md line 563
; Purpose: Test function with multiple parameters
; Expected: Successful assembly with two-parameter function

DATA
    num1: DW [15]
    num2: DW [27]
    sum: DW [0]

CODE
main:
    ; Load parameters
    LD AX, [num1]           ; First parameter
    LD BX, [num2]           ; Second parameter
    
    ; Call add function
    CALL add_numbers
    
    ; Store result
    LD [sum], AX            ; sum = 42
    HALT

; Function: add_numbers
; Input: AX = first number, BX = second number
; Output: AX = sum
add_numbers:
    ADD AX, BX
    RET
