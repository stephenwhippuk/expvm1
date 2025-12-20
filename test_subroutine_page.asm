DATA
    PAGE page1
    var1: DW [100]
    
    PAGE page2
    var2: DW [200]
    sub_var: DW [300]

CODE
start:
    ; Access page1
    LDA AX, var1        ; Should inject PAGE 1
    
    ; Call subroutine that accesses page2
    CALL my_subroutine
    
    ; After return, access page2
    ; PROBLEM: Assembler thinks we're still on page1 (from before CALL)
    ; But subroutine changed page to page2
    ; So this might incorrectly skip PAGE 2 injection
    LDA BX, var2        ; Should inject PAGE 2 after subroutine return
    
    HALT

my_subroutine:
    ; This subroutine accesses page2
    LDA CX, sub_var     ; Should inject PAGE 2
    RET
