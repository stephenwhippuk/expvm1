; Test: CMP with negative number -1 vs positive 1
; Expected: -1 < 1 should set AX = -1 (0xFFFF)
; 
; Two's complement: -1 = 0xFFFF, 1 = 0x0001
; If treating as signed: -1 < 1 (AX should be 0xFFFF)
; If treating as unsigned: 0xFFFF > 0x0001 (AX should be 1)
;
; This test verifies current CMP behavior for version 1.1 design

DATA
result: DW [0]

CODE
    ; Load -1 (0xFFFF in two's complement)
    LD AX, 0xFFFF
    
    ; Compare with 1
    CMP AX, 1
    
    ; Store result (should be -1 if signed, 1 if unsigned)
    LD [result], AX
    
    HALT

; Expected Results:
; If CMP treats values as SIGNED:
;   AX = 0xFFFF (-1, meaning -1 < 1)
;   result = 0xFFFF
;
; If CMP treats values as UNSIGNED:
;   AX = 0x0001 (1, meaning 65535 > 1)
;   result = 0x0001
