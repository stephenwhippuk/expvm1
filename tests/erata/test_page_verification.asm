; Test to verify PAGE instructions are injected correctly after CALL
DATA
    PAGE graphics
    sprite1: DW [10]
    sprite2: DW [20]
    
    PAGE sound  
    sample1: DW [30]
    sample2: DW [40]

CODE
start:
    ; Access sprite1 - should inject PAGE graphics
    LDA AX, sprite1
    
    ; Access sprite2 - should NOT inject PAGE (already on graphics page)
    LDA BX, sprite2
    
    ; Call subroutine - page becomes unknown
    CALL play_sound
    
    ; Access sprite1 again - SHOULD inject PAGE graphics (because page is unknown after CALL)
    LDA CX, sprite1
    
    HALT

play_sound:
    ; Access sample1 - should inject PAGE sound
    LDA AX, sample1
    
    ; Access sample2 - should NOT inject PAGE (already on sound page)
    LDA BX, sample2
    
    RET
