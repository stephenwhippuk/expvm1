; Test: Assembly File Structure
; Source: Overview.md line 91
; Purpose: Test proper DATA and CODE section structure
; Expected: Successful assembly with both sections

DATA
    label1: DB [10, 20, 30]
    label2: DW [1000, 2000]

CODE
label3:
    LDA AX, label1
    LDA BX, label2
    HALT
