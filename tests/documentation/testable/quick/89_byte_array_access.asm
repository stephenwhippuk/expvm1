; Test: Byte Array Access
; Source: Expressions.md line 325
; Purpose: Test accessing elements in byte array with constant offsets
; Expected: Successful assembly with byte array indexing

DATA
    bytes: DB [10, 20, 30, 40, 50]

CODE
    LDAB AL, [bytes + 0]        ; First element
    LDAB AL, [bytes + 1]        ; Second element
    LDAB AL, [bytes + 2]        ; Third element
    HALT
