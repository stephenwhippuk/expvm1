; Test: Undefined Symbol Error (Valid Version)
; Source: ErrorReference.md line 530
; Purpose: Test proper symbol definition before use
; Expected: Successful assembly with defined symbol

DATA
    data_value: DW [100]

CODE
    LD AX, [data_value]
    HALT
