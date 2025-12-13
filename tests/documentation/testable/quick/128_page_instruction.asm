; Test: PAGE Set Memory Page
; Source: InstructionSet.md line 719
; Purpose: Test setting memory page for paged architectures
; Expected: Successful assembly with page setting

CODE
    PAGE 0x0001         ; Switch to page 1
    LD AX, 100
    PAGE AX             ; Switch to page in AX
    HALT
