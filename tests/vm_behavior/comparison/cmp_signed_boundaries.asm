; Test: CMP edge cases with signed boundaries
; Tests comparison at signed integer boundaries
;
; Validates behavior at INT16 min/max values

DATA
test1: DW [0]  ; max positive (32767) vs min negative (-32768)
test2: DW [0]  ; min negative vs max positive
test3: DW [0]  ; max positive vs max positive
test4: DW [0]  ; min negative vs min negative
test5: DW [0]  ; 0 vs max positive
test6: DW [0]  ; 0 vs min negative

CODE
    ; Test 1: 32767 vs -32768
    LD AX, 0x7FFF    ; 32767 (max positive)
    LD BX, 0x8000    ; -32768 (min negative)
    CMP AX, BX
    LD [test1], AX
    
    ; Test 2: -32768 vs 32767
    LD AX, 0x8000
    LD BX, 0x7FFF
    CMP AX, BX
    LD [test2], AX
    
    ; Test 3: 32767 vs 32767
    LD AX, 0x7FFF
    CMP AX, 0x7FFF
    LD [test3], AX
    
    ; Test 4: -32768 vs -32768
    LD AX, 0x8000
    CMP AX, 0x8000
    LD [test4], AX
    
    ; Test 5: 0 vs 32767
    LD AX, 0
    CMP AX, 0x7FFF
    LD [test5], AX
    
    ; Test 6: 0 vs -32768
    LD AX, 0
    LD BX, 0x8000
    CMP AX, BX
    LD [test6], AX
    
    HALT

; Expected Results (if SIGNED comparison):
; test1 = 0x0001 (32767 > -32768)
; test2 = 0xFFFF (-32768 < 32767)
; test3 = 0x0000 (32767 == 32767)
; test4 = 0x0000 (-32768 == -32768)
; test5 = 0xFFFF (0 < 32767)
; test6 = 0x0001 (0 > -32768)
;
; Expected Results (if UNSIGNED comparison):
; test1 = 0xFFFF (32767 < 32768)
; test2 = 0x0001 (32768 > 32767)
; test3 = 0x0000 (32767 == 32767)
; test4 = 0x0000 (32768 == 32768)
; test5 = 0xFFFF (0 < 32767)
; test6 = 0xFFFF (0 < 32768)
