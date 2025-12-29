; Minimal text adventure - Version 1
; Just print one location and halt
DATA
    loc1_text: DB "You are in the starting room."
    loc1_len: DW [30]
    
CODE
start:
    ; For now, manually push the string since LDAB has assembler issues
    ; "You are in the starting room." = 30 chars
    ; We'll just print a short message for now
    PUSHB 0x2E  ; .
    PUSHB 0x6D  ; m
    PUSHB 0x6F  ; o
    PUSHB 0x6F  ; o
    PUSHB 0x72  ; r
    PUSHB 0x20  ; (space)
    PUSHB 0x61  ; a
    PUSHB 0x20  ; (space)
    PUSHB 0x6E  ; n
    PUSHB 0x69  ; i
    
    LD AX, 10
    PUSH AX
    SYS 0x0011  ; Print "in a room."
    HALT
