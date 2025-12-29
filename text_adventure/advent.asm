DATA
    PAGE game_data
    
    ; Game state (moved to top to avoid DA alignment issues)
    current_location_id: DB [1]  ; Current location ID (1-5)
    newline: DB [10]  ; Newline character
    
    ; Location 1 - Start Room
    loc1_id: DB [1]
    loc1text: DB "You are in the starting room. Exits lead in all directions."
    loc1exits: DB [2,3,4,5]  ; N, E, S, W

    ; Location 2 - North Room
    loc2_id: DB [2]
    loc2text: DB "You are in the north room. The start room is to the south."
    loc2exits: DB [0,0,1,0]  ; N, E, S, W

    ; Location 3 - East Room
    loc3_id: DB [3]
    loc3text: DB "You are in the east room. The start room is to the west."
    loc3exits: DB [0,0,0,1]  ; N, E, S, W

    ; Location 4 - South Room
    loc4_id: DB [4]
    loc4text: DB "You are in the south room. The start room is to the north."
    loc4exits: DB [1,0,0,0]  ; N, E, S, W

    ; Location 5 - West Room
    loc5_id: DB [5]
    loc5text: DB "You are in the west room. The start room is to the east."
    loc5exits: DB [0,1,0,0]  ; N, E, S, W

    ; Location structure pointers (id, text, exits)
    loc1: DA [loc1_id, loc1text, loc1exits]
    loc2: DA [loc2_id, loc2text, loc2exits]
    loc3: DA [loc3_id, loc3text, loc3exits]
    loc4: DA [loc4_id, loc4text, loc4exits]
    loc5: DA [loc5_id, loc5text, loc5exits]

    ; Array of all locations
    locations: DA [loc1, loc2, loc3, loc4, loc5]
    
CODE
start:
    ; Test JPZ - subtract to get 0 and jump
    LD AX, 5
    SUB 5
    JPZ success
    
    ; If we get here, jump failed
    PUSHB 70  ; F
    LD AX, 1
    PUSH AX
    SYS 0x0010
    HALT
    
success:
    PUSHB 83  ; S
    LD AX, 1
    PUSH AX
    SYS 0x0010
    HALT
