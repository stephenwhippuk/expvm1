# Examples of proper syntax

LD AX, 1 ; immediate value into register
LD AX, BX ; register into register
LD AX, LABEL ; load the address represented by label into register
LD AX, (LABEL + 1) ; load the address offset by constant byte into register
LD AX, (LABEL + BX) ; load the address offset by BX into register
LD AX, (LABEL + BX + 1) ; load the address of label + constant+ BX into Register
LDA AX, LABEL ; load the value stored at label into register
LDA AX, (LABEL + 1) ; load the avlues stored at label offset by constant into register
LDA AX, BX ; load the value stores at the address stored at register into register
LDA AX, (LABEL + BX) ; load the value stored at the labelled address offset by the value in register into register
LDA AX, (LABEL + BX + 1) ; load the value stored at the alebl offset by constant byte and offset by register into register
LDA LABEL, AX ; store the value in register into labelled address
LDA (LABEL + 1), AX ; store the value in register into the label offset by constant

and appropriate for byte operators

# syntactic sugar operations
LD AX, label[1] ; this is syntactic sugar for LDA AX, (label + 1)
LD AX, label[BX] ; this is synatic sugar for LDA AX, (label + BX)
LD AX, label[BX + 1] ; this is syntactic sugar for LDA AX (label + BX + 1)

binary representation may require more than one op to perform a particular offset if needed, but assembler should resolve the address + constant.

for instrace LDA AX, (Label + BX + 1) or LD AX, label[BX + 1]

might become (in binary)
LD AX, (label + 1) ; resolved address
ADD AX, BX
LDA AX, AX

in so far as possible the assembler will provide 1:1 op mapping but this cannot be always achieved with the current instruction set 

