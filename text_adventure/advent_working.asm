DATA
    ; Location 1 text
    loc1_text: DB "You are in the starting room."
    loc1_len: DW [30]
    
    ; Prompt message
    prompt: DB ">"
    prompt_len: DW [1]
    
CODE
start:
    CALL print_location
    HALT

; Print the current location description
print_location:
    ; Get text address and skip size prefix
    LD DX, loc1_text
    INC DX
    INC DX           ; DX now points to actual text data
    
    ; Get length value
    LD BX, loc1_len
    INC BX
    INC BX           ; BX points to actual length value
    LDA CX, BX       ; CX = length (30)
    
    ; Calculate end of string: DX + CX - 1
    SWP AX, DX       ; Swap so DX has text start
    ADD CX           ; AX (was DX) = one past end
    DEC AX           ; AX = last character
    SWP AX, DX       ; DX = last character position, AX = text start
    
    ; Push characters in reverse
push_loop:
    CMP CX, 0
    JPZ do_print
    
    ; Load byte and push
    LDAB AL, DX
    PUSHL AL
    
    ; Move to previous character
    DEC DX
    DEC CX
    JMP push_loop
    
do_print:
    ; Get length again for syscall
    LD BX, loc1_len
    INC BX
    INC BX
    LDA AX, BX
    PUSH AX
    SYS 0x0011       ; Print with newline
    RET
