; Test: Buffer Copy Example
; Source: Expressions.md line 591
; Purpose: Test copying bytes between buffers
; Expected: Successful assembly with buffer copy operations

DATA
    source: DB [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    dest: DB [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    size: DW [10]

CODE
    LDA BX, source              ; BX = source pointer
    LDA DX, dest                ; DX = dest pointer
    LD CX, [size]               ; CX = byte count

copy_loop:
    LDAB AX, (BX)               ; Load byte from source
    LD (DX), AL                 ; Store byte to dest
    
    INC BX                      ; Next source byte
    INC DX                      ; Next dest byte
    DEC CX                      ; Decrement counter
    JPNZ copy_loop              ; Continue if more bytes
    
    HALT
