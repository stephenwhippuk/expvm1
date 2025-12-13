; Test: PUSHW Push Immediate Word
; Source: InstructionSet.md line 610
; Purpose: Test pushing immediate word values onto stack
; Expected: Successful assembly with immediate word push

CODE
    PUSHW 0x1234        ; Push constant
    PUSHW 42            ; Push decimal value
    HALT
