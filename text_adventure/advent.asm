DATA
    ; Game state
    current_location_id: DB [1]  ; Current location ID (1-5)
    input_buffer: DB [0,0,0,0,0,0,0,0,0,0]  ; Buffer for user input
    
    ; Location 1 - Start Room
    loc1_id: DB [1]
    loc1_text: DB "You are in the starting room. Exits lead in all directions."
    loc1_len: DW [61]
    loc1_exits: DB [2,3,4,5]  ; N, E, S, W

    ; Location 2 - North Room
    loc2_id: DB [2]
    loc2_text: DB "You are in the north room. The start room is to the south."
    loc2_len: DW [59]
    loc2_exits: DB [0,0,1,0]  ; N, E, S, W

    ; Location 3 - East Room
    loc3_id: DB [3]
    loc3_text: DB "You are in the east room. The start room is to the west."
    loc3_len: DW [57]
    loc3_exits: DB [0,0,0,1]  ; N, E, S, W

    ; Location 4 - South Room
    loc4_id: DB [4]
    loc4_text: DB "You are in the south room. The start room is to the north."
    loc4_len: DW [59]
    loc4_exits: DB [1,0,0,0]  ; N, E, S, W

    ; Location 5 - West Room
    loc5_id: DB [5]
    loc5_text: DB "You are in the west room. The start room is to the east."
    loc5_len: DW [57]
    loc5_exits: DB [0,1,0,0]  ; N, E, S, W

    ; Location structure pointers (id, text_ptr, text_len_ptr, exits_ptr)
    loc1: DA [loc1_id, loc1_text, loc1_len, loc1_exits]
    loc2: DA [loc2_id, loc2_text, loc2_len, loc2_exits]
    loc3: DA [loc3_id, loc3_text, loc3_len, loc3_exits]
    loc4: DA [loc4_id, loc4_text, loc4_len, loc4_exits]
    loc5: DA [loc5_id, loc5_text, loc5_len, loc5_exits]

    ; Array of all locations
    locations: DA [loc1, loc2, loc3, loc4, loc5]
    
    ; Messages
    prompt_msg: DB "Enter command (N/S/E/W or Q to quit): "
    prompt_len: DW [39]
    invalid_msg: DB "You can't go that way!"
    invalid_len: DW [23]
    quit_msg: DB "Thanks for playing!"
    quit_len: DW [19]
    newline: DB [10]
    
CODE
start:
    ; Main game loop
game_loop:
    CALL print_location
    CALL print_prompt
    CALL get_input
    CALL process_input
    ; If still running, loop
    JMP game_loop

; ===== PRINT LOCATION =====
; Prints the description of the current location
print_location:
    ; Get current location ID (1-5)
    LDAB AL, current_location_id
    
    ; Convert to array index (0-4)
    DEC AL
    
    ; Get location structure pointer
    ; locations is an array of addresses, each 2 bytes
    LD BX, AL
    SHL BX, 1  ; Multiply by 2 for word indexing
    LDA CX, (locations + BX)  ; CX = pointer to location structure
    
    ; Location structure: [id_ptr, text_ptr, len_ptr, exits_ptr]
    ; We want text_ptr (offset +2) and len_ptr (offset +4)
    LDA DX, (CX + 2)  ; DX = text pointer
    LDA BX, (CX + 4)  ; BX = length pointer
    LDA AX, BX      ; AX = length value
    
    ; Now push the string onto stack (in reverse)
    PUSH CX  ; Save location structure pointer
    PUSH AX  ; Save length
    
    ; DX = text pointer, AX = length
    LD CX, AX  ; CX = length (counter)
    ADD DX, AX ; DX = pointer to end of string
    DEC DX     ; DX = pointer to last character
    
print_char_loop:
    CMP CX, 0
    JPZ print_done
    LDAB AL, DX
    PUSHB AL
    DEC DX
    DEC CX
    JMP print_char_loop
    
print_done:
    POP AX  ; Restore length
    PUSH AX ; Push length for syscall
    SYS 0x0011  ; Print with newline
    POP CX  ; Restore location pointer
    RET

; ===== PRINT PROMPT =====
print_prompt:
    ; Print the prompt message
    LDA DX, prompt_msg
    LDA AX, prompt_len
    
    LD CX, AX
    ADD DX, AX
    DEC DX
    
prompt_loop:
    CMP CX, 0
    JPZ prompt_done
    LDAB AL, DX
    PUSHB AL
    DEC DX
    DEC CX
    JMP prompt_loop
    
prompt_done:
    LDA AX, prompt_len
    PUSH AX
    SYS 0x0010  ; Print without newline
    RET

; ===== GET INPUT =====
get_input:
    ; Read a line from user (max 10 chars)
    LD AX, 10
    PUSH AX
    SYS 0x0012  ; Read line
    
    ; Result: [count] [char_1] [char_2] ...
    POP AX  ; Get count
    
    ; For simplicity, just get first character
    ; If count > 0, pop first char, discard rest
    CMP AX, 0
    JPZ input_empty
    
    ; Save the count
    PUSH AX
    
    ; Pop first character into BL
    POP BX
    
    ; Store it in input_buffer
    LDAB input_buffer, BL
    
    ; Discard remaining characters
    POP AX  ; Restore count
    DEC AX
    
discard_loop:
    CMP AX, 0
    JPZ input_done
    POP BX  ; Discard character
    DEC AX
    JMP discard_loop
    
input_empty:
    ; No input, store 0
    LD BL, 0
    LDAB input_buffer, BL
    
input_done:
    RET

; ===== PROCESS INPUT =====
process_input:
    ; Get the first character from input buffer
    LDAB AL, input_buffer
    
    ; Convert to uppercase if lowercase
    CMP AL, 97  ; 'a'
    JPN check_command
    CMP AL, 122  ; 'z'
    JPP check_command
    SUB AL, 32  ; Convert to uppercase
    
check_command:
    ; Check for Q (quit)
    CMP AL, 81  ; 'Q'
    JPZ quit_game
    
    ; Check for N (north)
    CMP AL, 78  ; 'N'
    JPZ try_north
    
    ; Check for S (south)
    CMP AL, 83  ; 'S'
    JPZ try_south
    
    ; Check for E (east)
    CMP AL, 69  ; 'E'
    JPZ try_east
    
    ; Check for W (west)
    CMP AL, 87  ; 'W'
    JPZ try_west
    
    ; Invalid command - just return
    RET

try_north:
    LD BX, 0
    JMP try_move
    
try_south:
    LD BX, 2
    JMP try_move
    
try_east:
    LD BX, 1
    JMP try_move
    
try_west:
    LD BX, 3
    JMP try_move

; ===== TRY MOVE =====
; BX = direction index (0=N, 1=E, 2=S, 3=W)
try_move:
    ; Get current location
    LDAB AL, current_location_id
    DEC AL  ; Convert to index
    
    ; Get location structure
    LD CX, AL
    SHL CX, 1
    LDA DX, (locations + CX)  ; DX = location structure pointer
    
    ; Get exits pointer (offset +6 in structure)
    LDA CX, (DX + 6)  ; CX = exits array pointer
    
    ; Get exit for direction BX
    ADD CX, BX
    LDAB AL, CX  ; AL = destination location ID
    
    ; Check if valid (non-zero)
    CMP AL, 0
    JPZ invalid_move
    
    ; Valid move - update location
    LDAB current_location_id, AL
    RET
    
invalid_move:
    ; Print "can't go that way" message
    LDA DX, invalid_msg
    LDA AX, invalid_len
    
    LD CX, AX
    ADD DX, AX
    DEC DX
    
invalid_loop:
    CMP CX, 0
    JPZ invalid_done
    LDAB AL, DX
    PUSHB AL
    DEC DX
    DEC CX
    JMP invalid_loop
    
invalid_done:
    LDA AX, invalid_len
    PUSH AX
    SYS 0x0011
    RET

; ===== QUIT GAME =====
quit_game:
    ; Print goodbye message
    LDA DX, quit_msg
    LDA AX, quit_len
    
    LD CX, AX
    ADD DX, AX
    DEC DX
    
quit_loop:
    CMP CX, 0
    JPZ quit_done
    LDAB AL, DX
    PUSHB AL
    DEC DX
    DEC CX
    JMP quit_loop
    
quit_done:
    LDA AX, quit_len
    PUSH AX
    SYS 0x0011
    HALT
