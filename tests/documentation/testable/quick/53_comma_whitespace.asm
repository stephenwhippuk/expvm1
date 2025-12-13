; Test: Optional Comma Whitespace
; Source: Syntax.md line 439
; Purpose: Test that whitespace around commas is optional
; Expected: Successful assembly with various comma spacing styles

CODE
    LD AX,10            ; Valid - no spaces
    LD BX, 20           ; Valid - space after comma
    LD CX , 30          ; Valid - space before comma
    HALT
