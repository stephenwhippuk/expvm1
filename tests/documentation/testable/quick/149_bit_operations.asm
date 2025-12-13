; Test: Bit manipulation with AND, OR, XOR
; Source: InstructionSet.md line 1835
; Category: quick

CODE
    LD AX, 0x00FF
    OR AX, 0x00F0       ; AX = 0x00FF
    AND AX, 0xFFF0      ; AX = 0x00F0
    XOR AX, 0x0100      ; AX = 0x01F0
    LD BX, AX
    AND BX, 0x0100      ; BX = 0x0100 (bit is set)
    CMP BX, 0
    JPNZ bit_is_set
bit_is_set:
    HALT
