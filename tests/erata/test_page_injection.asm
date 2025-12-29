DATA
PAGE page1
var1: DB [1, 2, 3]
PAGE page2
var2: DW [0x100, 0x200]

CODE
START:
    LDA AX, var1    ; Should inject PAGE 1
    LDA BX, var2    ; Should inject PAGE 2
    LDA AX, var1    ; Should inject PAGE 1 again
    HALT
