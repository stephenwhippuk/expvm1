; Test: Meaningful Labels
; Source: Syntax.md line 589
; Purpose: Test descriptive label names for clarity
; Expected: Successful assembly with clear label names

DATA
    player_score: DW [0]
    enemy_count: DB [5]

CODE
initialize_game:
    LD AX, [player_score]
    LD BX, [enemy_count]
    HALT
