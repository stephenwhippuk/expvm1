# Hello World Program
# Prints "Hello!" to stdout

DATA
    message: DB "Hello!"
    msg_len: DW [6]

CODE
start:
    # Push message address
    PUSHW R0, message
    
    # Push message length
    PUSHW R1, 6
    
    # Print to stdout (syscall 0x01)
    SYSCALL 0x01
    
    # Exit program
    HALT
