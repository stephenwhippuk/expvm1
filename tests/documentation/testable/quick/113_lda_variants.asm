; Test: LDA Load from Address Variants
; Source: InstructionSet.md line 440
; Purpose: Test loading and storing words from memory
; Expected: Successful assembly with memory operations

DATA
    value: DW [0x1234]

CODE
    LD AX, [value]      ; Load word from 'value'
    LD BX, 0x5678
    LD [value], BX      ; Store BX to 'value'
    
    LDA CX, value       ; Load address of 'value' into CX
    LD DX, (CX)         ; Load word from address in CX
    HALT
