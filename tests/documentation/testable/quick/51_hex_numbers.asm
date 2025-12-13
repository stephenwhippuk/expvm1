; Test: Hexadecimal Literals
; Source: Syntax.md line 402
; Purpose: Test hexadecimal number literals with various formats
; Expected: Successful assembly with hex values

CODE
    LD AX, 0x00         ; Zero
    LD BX, 0xFF         ; 255
    LD CX, 0x1234       ; 4660
    LD DX, 0xABCD       ; Case insensitive
    LD EX, 0xabcd       ; Same as above
    HALT
