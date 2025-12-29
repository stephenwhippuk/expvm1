DATA
CODE
start:
    ; This should also be rejected - numeric address in parentheses
    LDAB AL, (100)
    HALT
