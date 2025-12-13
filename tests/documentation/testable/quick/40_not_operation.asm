; ============================================
; TEST: not_operation
; SOURCE: InstructionSet.md (line 1284)
; DESCRIPTION: Bitwise NOT (one's complement)
; EXPECTED: AX = 0xFF00
; ============================================

CODE
    LD AX, 0x00FF
    NOT AX
    HALT
