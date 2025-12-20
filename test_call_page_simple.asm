DATA
    PAGE page1
    var1: DW [100]
    
    PAGE page2  
    var2: DW [200]

CODE
start:
    LDA AX, var1        ; Should inject PAGE 1
    CALL sub            ; Page state becomes unknown
    LDA BX, var2        ; Should inject PAGE 2 (even though var2 is on page 2)
    HALT

sub:
    LDA CX, var2        ; Should inject PAGE 2
    RET                 ; Returns - caller doesn't know what page we're on
