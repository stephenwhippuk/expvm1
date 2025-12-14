; Test: Dynamic Structure Access
; Source: Expressions.md line 634
; Purpose: Test accessing array of structures with dynamic indexing
; Expected: Successful assembly with structure array access

DATA
    ; Array of structures: each has 2 words (4 bytes)
    ; struct Item { word id; word value; }
    items: DW [1, 100, 2, 200, 3, 300]
    item_index: DW [1]          ; Access item 1

CODE
    LDA CX, item_index         ; CX = item index
    
    ; Calculate offset: index * 4 (each item is 4 bytes)
    LD AX, CX
    MUL AX, 4                   ; AX = byte offset to item
    
    ; Access fields
    LDA BX, (items + 2 + AX)     ; Load id field
    LDA DX, (items + 2 + AX + 2)     ; Load value field
    
    ; BX = 2 (id of item 1)
    ; DX = 200 (value of item 1)
    
    HALT
