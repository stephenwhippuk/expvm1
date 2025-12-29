; Test JPZ (Jump if Zero) instruction
; Expected: Should print 'S' for success

CODE
start:
    ; Set AX to 0 (should set ZERO flag)
    LD AX, 0
    
    ; This should jump to success
    JPZ success
    
    ; If we reach here, JPZ failed
    PUSHB 70  ; 'F' for fail
    LD AX, 1
    PUSH AX
    SYS 0x0010
    HALT
    
success:
    PUSHB 83  ; 'S' for success
    LD AX, 1
    PUSH AX
    SYS 0x0010
    HALT
