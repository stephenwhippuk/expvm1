; Test: CMP immediate vs register with negatives
; Tests both CMP variants with negative values
;
; Validates consistency between reg-reg and reg-imm variants

DATA
test1: DW [0]  ; Register vs Register: -1 vs 1
test2: DW [0]  ; Register vs Immediate: -1 vs 1
test3: DW [0]  ; Register vs Register: -100 vs -50
test4: DW [0]  ; Register vs Immediate: -100 vs -50
test5: DW [0]  ; Register vs Immediate: AX=-1 vs 0xFFFF

CODE
    ; Test 1: Reg-Reg: -1 vs 1
    LD AX, 0xFFFF
    LD BX, 1
    CMP AX, BX
    LD [test1], AX
    
    ; Test 2: Reg-Imm: -1 vs 1
    LD AX, 0xFFFF
    CMP AX, 1
    LD [test2], AX
    
    ; Test 3: Reg-Reg: -100 vs -50
    LD AX, 0xFF9C    ; -100
    LD BX, 0xFFCE    ; -50
    CMP AX, BX
    LD [test3], AX
    
    ; Test 4: Reg-Imm: -100 vs -50 (using hex immediate)
    LD AX, 0xFF9C
    CMP AX, 0xFFCE
    LD [test4], AX
    
    ; Test 5: Reg-Imm: -1 vs -1 (0xFFFF vs 0xFFFF)
    LD AX, 0xFFFF
    CMP AX, 0xFFFF
    LD [test5], AX
    
    HALT

; Expected Results (if SIGNED):
; test1 = 0xFFFF (-1 < 1)
; test2 = 0xFFFF (-1 < 1) - should match test1
; test3 = 0xFFFF (-100 < -50)
; test4 = 0xFFFF (-100 < -50) - should match test3
; test5 = 0x0000 (-1 == -1)
;
; Key validation: test1 should equal test2, test3 should equal test4
; This confirms both CMP variants handle negatives consistently
