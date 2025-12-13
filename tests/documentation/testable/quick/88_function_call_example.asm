; Test: Function Call Example
; Source: InstructionSet.md line 1858
; Purpose: Test CALL and RET with subroutine pattern
; Expected: Successful assembly with function call

DATA
    value: DW [10]

CODE
main:
    LD AX, [value]
    CALL square         ; Call square function
    LD [value], AX      ; Store result (100)
    HALT

square:
    MUL AX, AX          ; AX = AX * AX
    RET
