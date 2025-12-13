; Test: Bit Manipulation Example
; Source: InstructionSet.md line 1833
; Purpose: Test complex bit operations (set, clear, toggle, test)
; Expected: Successful assembly with logical operations

CODE
    ; Set bits 4-7
    LD AX, 0x0000
    OR AX, 0x00F0       ; AX = 0x00F0
    
    ; Clear bits 0-3
    AND AX, 0xFFF0      ; AX = 0x00F0
    
    ; Toggle bit 8
    XOR AX, 0x0100      ; AX = 0x01F0
    
    ; Test bit 8
    LD BX, AX
    AND BX, 0x0100      ; BX = 0x0100 (bit is set)
    CMP BX, 0
    JPNZ bit_is_set
    
bit_is_set:
    HALT
