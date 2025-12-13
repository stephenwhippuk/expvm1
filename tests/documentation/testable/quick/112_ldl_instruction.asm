; Test: LDL Load Low Byte
; Source: InstructionSet.md line 413
; Purpose: Test loading byte into low byte of register
; Expected: Successful assembly with low byte load

CODE
    LD AX, 0x0000
    LDL AL, 0x34        ; AX becomes 0x0034
    LDL BL, AL          ; Copy AL to BL
    HALT
