DATA
    PAGE page1
    var1: DW [100]
    var2: DW [200]
    var3: DW [300]
    
    PAGE page2
    var4: DW [400]

CODE
    ; First access to page1 - should inject PAGE 1
    LDA AX, var1
    
    ; Second access to page1 - should NOT inject (optimization)
    LDA BX, var2
    
    ; Third access to page1 - should NOT inject (optimization)
    LDA CX, var3
    
    ; Access to page2 - should inject PAGE 2
    LDA DX, var4
    
    ; Back to page1 - should inject PAGE 1
    LDA EX, var1
    
    ; Stay on page1 - should NOT inject
    LDA AX, var2
    
    HALT
