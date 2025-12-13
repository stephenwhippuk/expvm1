; ============================================
; TEST: code_labels
; SOURCE: Syntax.md (line 86)
; DESCRIPTION: Multiple code labels
; EXPECTED: Labels mark instruction positions correctly
; ============================================

CODE
start:
    LD AX, 10

loop:
    DEC AX
    JPNZ loop

end:
    HALT
