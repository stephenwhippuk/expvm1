; ============================================
; TEST: or_operations
; SOURCE: InstructionSet.md (line 1156)
; DESCRIPTION: Bitwise OR operations
; EXPECTED: AX = 0xFFFF
; ============================================

CODE
    LD AX, 0xFF00
    OR AX, 0x00FF
    LD BX, 0x0000
    OR AX, BX
    HALT
