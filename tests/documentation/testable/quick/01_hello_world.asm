; ============================================
; TEST: hello_world
; SOURCE: Examples.md (line 32)
; DESCRIPTION: Basic program structure
; EXPECTED: Assembles successfully, AX = 0x48 ('H')
; ============================================

DATA
    message: DB "Hello, World!"
    msg_len: DW [13]

CODE
start:
    LD AX, [message]
    HALT
