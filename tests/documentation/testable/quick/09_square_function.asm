; ============================================
; TEST: square_function
; SOURCE: Examples.md (line 526)
; DESCRIPTION: Function call with parameter
; EXPECTED: result = 49 (7*7)
; ============================================

DATA
    input: DW [7]
    result: DW [0]

CODE
main:
    LD AX, [input]
    CALL square
    LD [result], AX
    HALT

square:
    MUL AX, AX
    RET
