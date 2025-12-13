; Test sugar syntax array indexing
; Verifies that label[expr] converts to LDA (label + expr)

DATA
    bytes: DB [10, 20, 30, 40, 50]
    words: DW [100, 200, 300, 400, 500]
    result: DW [0]

CODE
    ; Test 1: Constant index with bytes
    LDAB AL, bytes[0]        ; Should load 10
    LDAB BL, bytes[1]        ; Should load 20
    LDAB CL, bytes[4]        ; Should load 50
    
    ; Test 2: Constant index with words
    LDA AX, words[0]         ; Should load 100
    LDA BX, words[2]         ; Should load 200
    
    ; Test 3: Register index
    LD DX, 2
    LDAB DL, bytes[DX]       ; Should load bytes[2] = 30
    
    ; Test 4: Register index with words
    LD CX, 4
    LDA AX, words[CX]        ; Should load words[4] = 300
    
    ; Test 5: Complex expression
    LD BX, 1
    LDAB AL, bytes[BX + 1]   ; Should load bytes[2] = 30
    
    ; Test 6: Store with sugar syntax
    LD AL, 0xFF
    LDAB bytes[3], AL        ; Store 0xFF at bytes[3]
    
    ; Store result and halt
    LDA result, AX
    HALT
