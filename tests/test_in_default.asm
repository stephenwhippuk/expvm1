DATA
PAGE page1

CODE
    ; Test inline data without IN keyword (should default to page 0)
    LDA AX, DW [100, 200]
    ; Test inline data with IN keyword
    LDA BX, DW [300, 400] IN page1
    HALT
