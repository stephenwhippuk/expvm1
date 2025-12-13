; ============================================
; TEST: conditional_if_else
; SOURCE: Examples.md (line 806, temperature check)
; DESCRIPTION: If-else conditional logic
; EXPECTED: status = 0 (normal, temp=75)
; ============================================

DATA
    temperature: DW [75]
    status: DW [0]

CODE
    LDA AX, temperature
    
    CMP AX, 50
    JPNZ check_hot
    
is_cold:
    LD BX, 1
    JMP store_status

check_hot:
    CMP AX, 80
    JPZ is_hot
    
is_normal:
    LD BX, 0
    JMP store_status

is_hot:
    LD BX, 2

store_status:
    LDA status, BX
    HALT
