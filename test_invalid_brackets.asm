; Test invalid bracket usage

DATA
buffer: DW [0x1234]

CODE
    ; This should be an error - LD with [expr] without sugar syntax
    LD AX, [buffer]
    
    HALT
