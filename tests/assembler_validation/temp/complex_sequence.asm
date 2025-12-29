CODE
    LD AX, 100
    LD BX, 10
    ADD BX
    MUL BX
    DIV BX
    CMP AX, BX
    JPZ end
    INC AX
end:
    HALT
