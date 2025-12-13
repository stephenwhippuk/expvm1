; Test with 0x0100 to force 2-byte encoding

CODE
    PUSHW 42
    SYS 0x0100
    HALT
