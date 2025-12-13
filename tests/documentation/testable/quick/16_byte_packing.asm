; ============================================
; TEST: byte_packing
; SOURCE: Registers.md (line 372)
; DESCRIPTION: Pack and unpack bytes in word
; EXPECTED: BL=0x12, CL=0x34, packed=0xABCD
; ============================================

DATA
    packed: DW [0x1234]

CODE
    LD AX, [packed]
    LD BL, AH
    LD CL, AL
    
    LD DH, 0xAB
    LD DL, 0xCD
    LD [packed], DX
    
    HALT
