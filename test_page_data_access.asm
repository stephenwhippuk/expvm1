DATA
PAGE page1
value1: DB [42]

PAGE page2  
value2: DB [100]

CODE
START:
    ; Load from page 1
    LDA AL, value1
    
    ; Load from page 2
    LDA BL, value2
    
    ; Load from page 1 again
    LDA CL, value1
    
    HALT
