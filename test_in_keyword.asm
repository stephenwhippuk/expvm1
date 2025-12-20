DATA
PAGE page1
var1: DB [1]

PAGE page2
var2: DB [2]

CODE
    ; Test inline data with IN keyword
    LDA AX, DW [100, 200] IN page1
    LDA BX, DW [300, 400] IN page2
    LDA CX, DW [500, 600] IN page1
    HALT
