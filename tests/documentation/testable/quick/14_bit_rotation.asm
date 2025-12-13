; ============================================
; TEST: bit_rotation
; SOURCE: Examples.md (line 950)
; DESCRIPTION: Rotate bits left and right
; EXPECTED: rotated_left = 0x0003, rotated_right = 0xC000
; ============================================

DATA
    value: DW [0x8001]
    rotated_left: DW [0]
    rotated_right: DW [0]

CODE
    LD AX, [value]
    ROL AX, 1
    LD [rotated_left], AX
    
    LD AX, [value]
    ROR AX, 1
    LD [rotated_right], AX
    
    HALT
