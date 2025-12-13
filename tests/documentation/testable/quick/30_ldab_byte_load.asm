; ============================================
; TEST: ldab_byte_load
; SOURCE: InstructionSet.md (line 468)
; DESCRIPTION: Load byte from memory address
; EXPECTED: AX = 0x0042 (zero-extended)
; ============================================

DATA
    byte_val: DB [0x42]

CODE
    LDAB AX, [byte_val]
    HALT
