; Test: Debug system call 0x1500 - works around assembler optimization bug
; Use values > 255 to force 2-byte encoding

CODE
    ; Test 1: Print 300 (forces 2-byte encoding)
    PUSHW 300
    SYS 0x1500
    
    ; Test 2: Print 0x7FFF (32767)
    PUSHW 0x7FFF
    SYS 0x1500
    
    ; Test 3: Print 0xFFFF (65535 or -1)
    PUSHW 0xFFFF
    SYS 0x1500
    
    ; Test 4: Print 0x8000 (32768 or -32768)
    PUSHW 0x8000
    SYS 0x1500
    
    HALT

; Expected output (if unsigned):
; 300
; 32767
; 65535
; 32768
