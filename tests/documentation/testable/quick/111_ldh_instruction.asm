; Test: LDH Load High Byte
; Source: InstructionSet.md line 388
; Purpose: Test loading byte into high byte of register
; Expected: Successful assembly with high byte load

CODE
    LD AX, 0x0000
    LDH AH, 0x12        ; AX becomes 0x1200
    LDH BH, AH          ; Copy AH to BH
    HALT
