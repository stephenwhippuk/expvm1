; ============================================
; TEST: register_operands
; SOURCE: Syntax.md (line 291)
; DESCRIPTION: 16-bit and 8-bit register operands
; EXPECTED: Register addressing works correctly
; ============================================

CODE
    LD AX, 100
    LD BX, 200
    LD CX, 10
    LD DX, 0x1234
    LD EX, 0x1000
    
    LD AH, 0x12
    LD AL, 0x34
    LD BH, 10
    LD BL, 20
    
    HALT
