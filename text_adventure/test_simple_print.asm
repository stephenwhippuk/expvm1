DATA
    ; Simple location text
    loc1_text: DB "You are in a room."
    loc1_len: DW [18]
    
CODE
start:
    ; Print location text
    LD DX, loc1_text      ; DX = address of text
    LDA AX, loc1_len      ; AX = address of length
    LDA AX, AX            ; AX = length value (indirect load)
    
    ; Push string to stack in reverse
    LD CX, AX             ; CX = length counter
    ADD DX, AX            ; DX = end of string
    DEC DX                ; DX = last char
    
push_loop:
    CMP CX, 0
    JPZ print_it
    LDAB AL, DX
    PUSHL AL
    DEC DX
    DEC CX
    JMP push_loop
    
print_it:
    LDA AX, loc1_len
    LDA AX, AX            ; Get length value
    PUSH AX
    SYS 0x0011            ; Print with newline
    HALT
