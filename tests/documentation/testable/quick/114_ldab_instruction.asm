; Test: LDAB Load Byte from Address
; Source: InstructionSet.md line 460
; Purpose: Test loading single byte from memory
; Expected: Successful assembly with byte load

DATA
    byte_val: DB [0x42]

CODE
    LDAB AX, (byte_val + 2) ; AX = 0x0042 (byte zero-extended)
    HALT
