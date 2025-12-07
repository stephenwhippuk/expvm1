#pragma once

// System call table
// reserved: 0x0000 - 0x000F for system exit modes
#define SYSCALL_PRINT_STRING_FROM_STACK      0x0010  // Print string from stack
#define SYSCALL_PRINT_LINE_FROM_STACK        0x0011  // Print line from stack
#define SYSCALL_READ_LINE_ONTO_STACK         0x0012  // Read line onto stack