; Test DA error - labels on different pages
DATA
    PAGE page1
    handler1: DW [0x1111]
    handler_table: DA [handler1, handler2]
    
    PAGE page2
    handler2: DW [0x2222]
    
CODE
start:
    HALT
