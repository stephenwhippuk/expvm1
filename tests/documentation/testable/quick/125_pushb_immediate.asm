; Test: PUSHB Push Immediate Byte
; Source: InstructionSet.md line 626
; Purpose: Test pushing immediate byte values onto stack
; Expected: Successful assembly with immediate byte push

CODE
    PUSHB 0x42          ; Push byte constant
    PUSHB 10            ; Push decimal byte
    HALT
