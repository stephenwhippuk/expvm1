; Test: Debug system call 0x1500 - print word as number
; Undocumented experimental feature for debugging

CODE
    ; Test 1: Print positive number 42
    PUSHW 42
    SYS 0x1500
    
    ; Test 2: Print 0
    PUSHW 0
    SYS 0x1500
    
    ; Test 3: Print large positive (32767)
    PUSHW 0x7FFF
    SYS 0x1500
    
    ; Test 4: Print negative number as two's complement (0xFFFF = -1 or 65535)
    PUSHW 0xFFFF
    SYS 0x1500
    
    ; Test 5: Print -32768 / 0x8000
    PUSHW 0x8000
    SYS 0x1500
    
    HALT

; Expected output:
; 42
; 0
; 32767
; 65535 (or -1 if printed as signed)
; 32768 (or -32768 if printed as signed)
