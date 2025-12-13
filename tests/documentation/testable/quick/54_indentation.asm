; Test: Optional Indentation
; Source: Syntax.md line 447
; Purpose: Test that indentation is optional and allowed
; Expected: Successful assembly with indented code

CODE
    start:              ; Indented label
        HALT            ; Indented instruction
