; Test various instruction types

DATA
    num1: DW [0x0005]
    num2: DW [0x0003]
    result: DW [0x0000]

CODE
start:
    ; Load values into registers
    LD AX, 0x0005       ; Load 5 into AX
    LD BX, 0x0003       ; Load 3 into BX
    
    ; Arithmetic operations
    ADD AX              ; Add AX to itself (10)
    SUB BX              ; Subtract BX from AX (7)
    
    ; Store result using LDA
    ; Note: LDA with ADDR, REG stores REG into memory
    ; This would be opcode 0x0D
    
    ; Stack operations
    PUSH AX
    POP BX
    
    ; Compare and jump
    CMP BX, 0x0007
    JPZ equal
    JMP done
    
equal:
    INC CX
    
done:
    HALT
