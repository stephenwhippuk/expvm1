; Test: Structure Field Access
; Source: Expressions.md line 371
; Purpose: Test accessing fields of simulated structure
; Expected: Successful assembly with structure-like data access

DATA
    ; Simulated structure:
    ; struct Person {
    ;     word age;      // offset 0
    ;     byte initial;  // offset 2
    ;     byte grade;    // offset 3
    ; }
    person_age: DW [25]         ; age = 25
    person_initial: DB [0x4A]   ; initial = 'J'
    person_grade: DB [0x41]     ; grade = 'A'

CODE
    ; Access fields
    LD AX, [person_age + 0]         ; Load age (word at offset 0)
    LDAB BX, [person_age + 2]       ; Load initial (byte at offset 2)
    LDAB CX, [person_age + 3]       ; Load grade (byte at offset 3)
    HALT
