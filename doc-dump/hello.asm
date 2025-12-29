; Hello World Program for Pendragon VM
; Uses PRINT_LINE_FROM_STACK system call (0x0011)
;
; System call convention:
;   Push characters in reverse order
;   Push character count (WORD)
;   Execute SYSCALL 0x0011
;
; Opcodes used:
;   0x76 = PUSHB (push immediate byte)
;   0x75 = PUSHW (push immediate word)
;   0x7F = SYSCALL (system call)
;   0x01 = HALT

DATA:
    ; No data segment needed - we'll push the string directly

CODE:
    ; Push "Hello, World!" onto stack in reverse order
    ; String is 13 characters
    
    PUSHB '!'          ; 0x76, 0x21
    PUSHB 'd'          ; 0x76, 0x64
    PUSHB 'l'          ; 0x76, 0x6C
    PUSHB 'r'          ; 0x76, 0x72
    PUSHB 'o'          ; 0x76, 0x6F
    PUSHB 'W'          ; 0x76, 0x57
    PUSHB ' '          ; 0x76, 0x20
    PUSHB ','          ; 0x76, 0x2C
    PUSHB 'o'          ; 0x76, 0x6F
    PUSHB 'l'          ; 0x76, 0x6C
    PUSHB 'l'          ; 0x76, 0x6C
    PUSHB 'e'          ; 0x76, 0x65
    PUSHB 'H'          ; 0x76, 0x48
    
    ; Push character count (13 = 0x000D)
    PUSHW 0x000D       ; 0x75, 0x0D, 0x00
    
    ; Call PRINT_LINE_FROM_STACK (0x0011)
    SYSCALL 0x0011     ; 0x7F, 0x11, 0x00
    
    ; Halt the VM
    HALT               ; 0x01

; Binary representation:
; 0x76, 0x21,  ; PUSHB '!'
; 0x76, 0x64,  ; PUSHB 'd'
; 0x76, 0x6C,  ; PUSHB 'l'
; 0x76, 0x72,  ; PUSHB 'r'
; 0x76, 0x6F,  ; PUSHB 'o'
; 0x76, 0x57,  ; PUSHB 'W'
; 0x76, 0x20,  ; PUSHB ' '
; 0x76, 0x2C,  ; PUSHB ','
; 0x76, 0x6F,  ; PUSHB 'o'
; 0x76, 0x6C,  ; PUSHB 'l'
; 0x76, 0x6C,  ; PUSHB 'l'
; 0x76, 0x65,  ; PUSHB 'e'
; 0x76, 0x48,  ; PUSHB 'H'
; 0x75, 0x0D, 0x00,  ; PUSHW 13
; 0x7F, 0x11, 0x00,  ; SYSCALL 0x0011
; 0x01         ; HALT