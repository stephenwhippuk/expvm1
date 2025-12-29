DATA
PAGE page1
var1: DB [1]

PAGE page2
var2: DB [2]

CODE
    ; Test DB inline data with IN keyword
    LDA AX, DB [10, 20, 30] IN page1
    LDA BX, DB [40, 50, 60] IN page2
    ; Mix DB and DW
    LDA CX, DW [100, 200] IN page1
    LDA DX, DB [70, 80] IN page2
    HALT
