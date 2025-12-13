; ============================================
; TEST: instruction_operands
; SOURCE: Syntax.md (line 229)
; DESCRIPTION: Various instruction operand types
; EXPECTED: All instruction formats work correctly
; ============================================

DATA
    value: DW [100]

CODE
start:
    HALT
    NOP
    RET
    
    PUSH AX
    PUSHW 100
    JMP start
    
    LD AX, 42
    LDA BX, value
    ADD AX, BX
    HALT
