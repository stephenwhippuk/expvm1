DATA
    ; Location 1
    loc1_text: DB "You are in a room."
    loc1_len: DW [18]
    
CODE
start:
    ; Print location text
    LD DX, loc1_text
    INC DX
    INC DX                ; Skip size prefix
    LD BX, loc1_len
    INC BX
    INC BX                ; Skip size prefix
    LDA AX, BX            ; AX = length value
    
    ; Push string to stack in reverse
    LD CX, AX             ; CX = length counter
    LD AX, DX             ; AX = text pointer
    ADD AX, CX            ; AX = end of string
    LD DX, AX             ; DX = end of string
    DEC DX                ; DX = last char
    
push_loop:
    CMP CX, 0
    JPZ print_it          ; If CX == 0 (AX == 0), jump
    LDAB AL, DX
    PUSHL AL
    DEC DX
    DEC CX
    JMP push_loop
    
print_it:
    LD BX, loc1_len
    INC BX
    INC BX
    LDA AX, BX            ; Get length value
    PUSH AX
    SYS 0x0011            ; Print with newline
    HALT
