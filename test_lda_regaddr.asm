; Test LDA with register-indirect addressing
CODE
start:
    LD BX, 0x1000    ; Put an address in BX
    LDA AX, BX       ; Load from address in BX into AX
    HALT
