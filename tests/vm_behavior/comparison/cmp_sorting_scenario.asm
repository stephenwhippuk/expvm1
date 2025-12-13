; Test: CMP for sorting algorithm validation
; Simulates comparison operations needed for bubble sort
; with mixed positive and negative numbers
;
; Array: [5, -3, 12, -8, 0, 1, -1]

DATA
array: DW [5, 0xFFFD, 12, 0xFFF8, 0, 1, 0xFFFF]
count: DW [7]
; Results of each comparison during first pass
cmp1: DW [0]   ; 5 vs -3
cmp2: DW [0]   ; 5 vs 12
cmp3: DW [0]   ; 12 vs -8
cmp4: DW [0]   ; 12 vs 0
cmp5: DW [0]   ; 12 vs 1
cmp6: DW [0]   ; 12 vs -1

CODE
    ; Compare pairs to see if swap needed (ascending order)
    
    ; 5 vs -3
    LD AX, [array]
    LD BX, [array + 2]
    CMP AX, BX
    LD [cmp1], AX
    
    ; 5 vs 12
    LD AX, [array]
    LD BX, [array + 4]
    CMP AX, BX
    LD [cmp2], AX
    
    ; 12 vs -8
    LD AX, [array + 4]
    LD BX, [array + 6]
    CMP AX, BX
    LD [cmp3], AX
    
    ; 12 vs 0
    LD AX, [array + 4]
    LD BX, [array + 8]
    CMP AX, BX
    LD [cmp4], AX
    
    ; 12 vs 1
    LD AX, [array + 4]
    LD BX, [array + 10]
    CMP AX, BX
    LD [cmp5], AX
    
    ; 12 vs -1
    LD AX, [array + 4]
    LD BX, [array + 12]
    CMP AX, BX
    LD [cmp6], AX
    
    HALT

; Expected Results (if SIGNED comparison for ascending sort):
; cmp1 = 0x0001 (5 > -3, no swap needed)
; cmp2 = 0xFFFF (5 < 12, no swap needed)
; cmp3 = 0x0001 (12 > -8, swap needed)
; cmp4 = 0x0001 (12 > 0, swap needed)
; cmp5 = 0x0001 (12 > 1, swap needed)
; cmp6 = 0x0001 (12 > -1, swap needed)
;
; If unsigned, negative numbers would sort incorrectly
; (they'd be treated as large positive numbers)
