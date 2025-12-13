; Test: Data segment placement example
; Source: BinaryFormat.md line 320
; Category: quick

DATA
byte1: DB [0x42]
word1: DW [0x1234]

CODE
    LDA AX, byte1
    LDA BX, word1
    HALT
