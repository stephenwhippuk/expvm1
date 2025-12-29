; Test bracket syntax parsing

DATA
buffer: DW [0x1234, 0x5678]
value: DW [42]

CODE
    ; Test parentheses - address expressions
    LD AX, (buffer)
    LD BX, (buffer + 1)
    
    ; Test square brackets - memory access
    LDA AX, [buffer]
    LDA BX, [buffer + 1]
    
    ; Test sugar syntax - label[index]
    LD AX, buffer[0]
    LD BX, buffer[CX]
    LD CX, buffer[DX + 1]
    
    ; Test bare identifiers
    LD AX, buffer
    LDA AX, buffer
    
    HALT
