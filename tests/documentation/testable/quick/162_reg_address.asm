; Test: Register + address encoding
; Source: BinaryFormat.md line 415
; Category: quick

DATA
data1: DW [0x1234]
data2: DW [0x5678]

CODE
    LD AX, [data1]
    LD [data2], BX
    LDAB CX, [data1]
    HALT
