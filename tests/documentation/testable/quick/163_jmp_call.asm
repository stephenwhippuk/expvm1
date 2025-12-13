; Test: Jump and call instruction encoding
; Source: BinaryFormat.md line 426
; Category: quick

CODE
start:
    JMP next
skip:
    HALT
next:
    JPZ end
    JPNZ skip
    CALL func
end:
    HALT
func:
    RET
