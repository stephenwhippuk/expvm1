; ============================================
; TEST: string_length
; SOURCE: Examples.md (line 397)
; DESCRIPTION: Calculate null-terminated string length
; EXPECTED: length = 13
; ============================================

DATA
    text: DB "Hello, World!\0"
    length: DW [0]

CODE
    LD AX, 0
    LD BX, 0

strlen_loop:
    LDAB CL, (text + BX)
    CMP CL, 0
    JPZ strlen_done
    INC AX
    INC BX
    JMP strlen_loop

strlen_done:
    LDA length, AX
    HALT
