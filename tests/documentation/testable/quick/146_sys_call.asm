; Test: SYS/SYSCALL instruction
; Source: InstructionSet.md line 1700
; Category: quick

CODE
    LD AX, 1            ; Function parameter
    SYS 0x0001          ; Call system function 1
    HALT
