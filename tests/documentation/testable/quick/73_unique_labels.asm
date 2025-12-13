; Test: Duplicate Labels (Valid Version)
; Source: ErrorReference.md line 590
; Purpose: Test unique labels throughout code
; Expected: Successful assembly with unique label names

CODE
start:
    LD AX, 10
end:
    HALT
