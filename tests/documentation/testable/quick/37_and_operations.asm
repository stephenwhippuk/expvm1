; ============================================
; TEST: and_operations
; SOURCE: InstructionSet.md (line 1092)
; DESCRIPTION: Bitwise AND operations
; EXPECTED: AX = 0x0F0F
; ============================================

CODE
    LD AX, 0xFF0F
    AND AX, 0x0FFF
    LD BX, 0xFFFF
    AND AX, BX
    HALT
