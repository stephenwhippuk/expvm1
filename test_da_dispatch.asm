; Comprehensive DA test - Handler dispatch table
DATA
    PAGE handlers
    ; Handler data structures (simulating function entry points)
    handle_input: DW [0x1000]
    handle_update: DW [0x2000]
    handle_render: DW [0x3000]
    
    ; Dispatch table using DA
    handler_table: DA [handle_input, handle_update, handle_render]
    
CODE
start:
    ; Load first handler address
    LDA AX, handler_table
    PUSHW AX                ; Should be 0x0000 (address of handle_input)
    
    ; Load second handler address  
    LDA AX, (handler_table + 2)
    PUSHW AX                ; Should be 0x0004 (address of handle_update)
    
    ; Load third handler address
    LDA AX, (handler_table + 4)
    PUSHW AX                ; Should be 0x0008 (address of handle_render)
    
    HALT
