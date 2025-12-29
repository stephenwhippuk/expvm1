; Minimal test for JPZ instruction
.code
    ; Set zero flag by subtracting a value from itself
    LD AX, 5
    SUB 5      ; AX = 5 - 5 = 0, should set ZERO flag
    JPZ target ; Should jump to target
    
    ; This should not execute
    PUSHB 'X'
    SYS 1
    HALT
    
target:
    ; This should execute
    PUSHB 'Y'
    SYS 1
    HALT

.data
