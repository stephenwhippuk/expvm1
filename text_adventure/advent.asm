DATA
    Loc1Text: DB "This is the start room"

CODE
    ; Load the address of the data
    LD AX, Loc1Text
    ; Load word from that address
    LDA BX, AX
    HALT