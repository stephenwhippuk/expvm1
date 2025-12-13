; Test: Word Array Access
; Source: Expressions.md line 338
; Purpose: Test accessing elements in word array (2 bytes apart)
; Expected: Successful assembly with word array indexing

DATA
    words: DW [100, 200, 300, 400, 500]

CODE
    LDA AX, words          ; First element (100)
    LDA AX, (words + 2)          ; Second element (200)
    LDA AX, (words + 4)          ; Third element (300)
    HALT
