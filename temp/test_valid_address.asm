DATA
    mydata: DB [0x42]
CODE
start:
    ; Valid: using label
    LDAB AL, (mydata)
    ; Valid: using label with offset
    LDAB AL, (mydata+3)
    HALT
