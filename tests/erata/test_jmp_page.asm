DATA
    PAGE page1
    var1: DW [100]
    
    PAGE page2  
    var2: DW [200]

CODE
start:
    LDA AX, var1        ; Should inject PAGE 1
    JMP after_jump
    
after_jump:
    LDA BX, var2        ; Should inject PAGE 2 (page unknown after JMP)
    HALT
