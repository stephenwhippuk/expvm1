; Test: CMP with various negative numbers
; Tests multiple negative comparison scenarios
;
; This validates CMP behavior across range of negative values

DATA
test1: DW [0]  ; -1 vs 1
test2: DW [0]  ; -5 vs -2
test3: DW [0]  ; -100 vs 0
test4: DW [0]  ; -1 vs -1
test5: DW [0]  ; -32768 vs 1 (most negative vs positive)

CODE
    ; Test 1: -1 vs 1
    LD AX, 0xFFFF    ; -1
    CMP AX, 1
    LD [test1], AX
    
    ; Test 2: -5 vs -2 (-5 < -2, so should be -1)
    LD AX, 0xFFFB    ; -5 in two's complement
    LD BX, 0xFFFE    ; -2 in two's complement
    CMP AX, BX
    LD [test2], AX
    
    ; Test 3: -100 vs 0 (-100 < 0, so should be -1)
    LD AX, 0xFF9C    ; -100
    CMP AX, 0
    LD [test3], AX
    
    ; Test 4: -1 vs -1 (equal, so should be 0)
    LD AX, 0xFFFF
    CMP AX, 0xFFFF
    LD [test4], AX
    
    ; Test 5: -32768 vs 1 (most negative vs positive)
    LD AX, 0x8000    ; -32768 (most negative 16-bit signed)
    CMP AX, 1
    LD [test5], AX
    
    HALT

; Expected Results (if SIGNED comparison):
; test1 = 0xFFFF (-1 < 1)
; test2 = 0xFFFF (-5 < -2)
; test3 = 0xFFFF (-100 < 0)
; test4 = 0x0000 (-1 == -1)
; test5 = 0xFFFF (-32768 < 1)
;
; Expected Results (if UNSIGNED comparison):
; test1 = 0x0001 (65535 > 1)
; test2 = 0xFFFF (65531 < 65534)
; test3 = 0x0001 (65436 > 0)
; test4 = 0x0000 (65535 == 65535)
; test5 = 0x0001 (32768 > 1)
