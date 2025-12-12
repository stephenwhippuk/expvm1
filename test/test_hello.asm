; Test file for parser
; Simple hello world program

DATA
HELLO: DB "Hello, World!"

CODE
START:
    LD CX, 0xB
    
LOOP:
    DEC CX
    LDA AX, (HELLO + CX)
    PUSHB AX
    CMP CX, 0x00
    JNZ LOOP
    
    PUSHW 0x000B
    SYS 0x0011
    HALT
