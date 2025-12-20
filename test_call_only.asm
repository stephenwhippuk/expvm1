CODE
start:
    PUSHB 42
    CALL sub
    HALT

sub:
    PUSHB 99
    RET
