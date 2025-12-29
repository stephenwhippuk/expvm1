; Test DA (Define Address) - array of pointers
DATA
    PAGE handlers
    handler1: DW [0x1111]
    handler2: DW [0x2222]
    handler3: DW [0x3333]
    
    ; DA array must be on same page as referenced labels
    handler_table: DA [handler1, handler2, handler3]
    
CODE
start:
    ; Load address from handler_table
    LDA AX, handler_table        ; Get address of first entry
    LDA BX, (handler_table + 2)  ; Get address of second entry  
    HALT
