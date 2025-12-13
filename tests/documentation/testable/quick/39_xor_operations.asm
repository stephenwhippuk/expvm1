; ============================================
; TEST: xor_operations
; SOURCE: InstructionSet.md (line 1220)
; DESCRIPTION: Bitwise XOR operations
; EXPECTED: AX = 0x0000 (quick clear technique)
; ============================================

CODE
    LD AX, 0xFFFF
    XOR AX, 0xFFFF
    HALT
