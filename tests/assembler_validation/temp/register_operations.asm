CODE
    LD AX, 0x1234
    LD BX, 0x5678
    SWP AX, BX
    LDH CX, AX
    LDL DX, BX
    HALT
