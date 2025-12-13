; Test: Complete binary example with data and code
; Source: BinaryFormat.md line 245
; Category: quick

DATA
value: DW [0x1234]

CODE
    LD AX, [value]
    HALT
