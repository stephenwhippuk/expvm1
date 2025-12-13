; ============================================
; TEST: cmp_compare
; SOURCE: InstructionSet.md (line 1637)
; DESCRIPTION: Compare registers and values
; EXPECTED: Various comparison results in AX
; ============================================

CODE
    LD AX, 10
    LD BX, 20
    CMP AX, BX
    
    LD CX, 50
    CMP CX, 50
    
    LD DX, 100
    CMP DX, 50
    
    HALT
