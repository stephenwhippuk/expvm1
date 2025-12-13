; Test: Register Preservation Example
; Source: Examples.md line 587
; Purpose: Test function that saves and restores registers
; Expected: Successful assembly with stack-based preservation

DATA
    value: DW [10]
    result: DW [0]

CODE
main:
    LDA AX, value
    LD BX, 5                ; BX used by main
    
    CALL multiply_by_three
    
    ; BX still contains 5
    LDA result, AX
    HALT

; Function: multiply_by_three
; Input: AX = number
; Output: AX = number * 3
; Preserves: BX, CX
multiply_by_three:
    ; Save registers
    PUSH BX
    PUSH CX
    
    ; Use registers for calculation
    LD BX, 3
    MUL AX, BX
    
    ; Restore registers
    POP CX
    POP BX
    
    RET
