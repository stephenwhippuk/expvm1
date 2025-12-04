#pragma once
#include "memsize.h"

// Opcode naming convention to avoid conflicts:
// Base operation + operand type suffix
// _IMM = immediate value, _REG = register, _ADDR = address
// _W = word (16-bit), _B = byte (8-bit), _H = high byte, _L = low byte
// Numbers in parentheses indicate variants

// System operations
#define OPCODE_NOP              0x00
#define OPCODE_HALT             0x01

// Load operations
#define OPCODE_LD_REG_IMM_W     0x02  // LD (1) - Load immediate word to register
#define OPCODE_LD_REG_REG_W     0x03  // LD (2) - Load register to register
#define OPCODE_SWP_REG_REG      0x04  // Swap two registers
#define OPCODE_LDH_REG_IMM_B    0x05  // LDH (1) - Load immediate byte to high byte
#define OPCODE_LDH_REG_REG_B    0x06  // LDH (2) - Load register high to register high
#define OPCODE_LDL_REG_IMM_B    0x07  // LDL (1) - Load immediate byte to low byte
#define OPCODE_LDL_REG_REG_B    0x08  // LDL (2) - Load register low to register low
#define OPCODE_LDA_REG_ADDR_W   0x09  // LDA (1) - Load word from address to register
#define OPCODE_LDAB_REG_ADDR_B  0x0A  // Load byte from address to register
#define OPCODE_LDAH_REG_ADDR_B  0x0B  // LDAH (1) - Load byte from address to register high
#define OPCODE_LDAL_REG_ADDR_B  0x0C  // LDAL (1) - Load byte from address to register low
#define OPCODE_STA_ADDR_REG_W   0x0D  // Store word from register to address
#define OPCODE_STAH_ADDR_REG_B  0x0E  // STAH (1) - Store high byte to address
#define OPCODE_STAL_ADDR_REG_B  0x0F  // Store low byte to address

// Stack operations
#define OPCODE_PUSH_REG_W       0x10  // Push word register to stack
#define OPCODE_PUSHH_REG_B      0x11  // Push high byte to stack
#define OPCODE_PUSHL_REG_B      0x12  // Push low byte to stack
#define OPCODE_POP_REG_W        0x13  // Pop word from stack to register
#define OPCODE_POPH_REG_B       0x14  // Pop byte to register high
#define OPCODE_POPL_REG_B       0x15  // Pop byte to register low
#define OPCODE_PEEK_REG_OFF_W   0x16  // Peek word at stack base offset
#define OPCODE_PEEKF_REG_OFF_W  0x17  // Peek word at frame offset
#define OPCODE_PEEKB_REG_OFF_B  0x18  // Peek byte at stack base offset
#define OPCODE_PEEKFB_REG_OFF_B 0x19  // Peek byte at frame offset
#define OPCODE_FLSH             0x1A  // Flush stack to frame
#define OPCODE_PAGE_IMM_W       0x1B  // Set memory page with immediate word
#define OPCODE_PAGE_REG         0x1C  // Set memory page with register
#define OPCODE_SETF_ADDR        0x1D  // Set stack frame

// Jump/Branch operations
#define OPCODE_JMP_ADDR         0x1E  // Unconditional jump
#define OPCODE_JPZ_ADDR         0x1F  // Jump if zero
#define OPCODE_JPNZ_ADDR        0x20  // Jump if not zero
#define OPCODE_JPC_ADDR         0x21  // Jump if carry
#define OPCODE_JPNC_ADDR        0x22  // Jump if not carry
#define OPCODE_JPS_ADDR         0x23  // Jump if sign
#define OPCODE_JPNS_ADDR        0x24  // Jump if not sign
#define OPCODE_JPO_ADDR         0x25  // Jump if overflow
#define OPCODE_JPNO_ADDR        0x26  // Jump if not overflow
#define OPCODE_CALL_ADDR        0x27  // Call subroutine (was SUB)
#define OPCODE_RET              0x28  // Return from subroutine

// ALU operations - Addition
#define OPCODE_ADD_IMM_W        0x29  // Add immediate word to AX
#define OPCODE_ADD_REG_W        0x2A  // Add register to AX
#define OPCODE_ADB_IMM_B        0x2B  // Add immediate byte to AX
#define OPCODE_ADH_REG_B        0x2C  // Add register high to AX high
#define OPCODE_ADL_REG_B        0x2D  // Add register low to AX low

// ALU operations - Subtraction
#define OPCODE_SUB_IMM_W        0x2E  // Subtract immediate word from AX
#define OPCODE_SUB_REG_W        0x2F  // Subtract register from AX
#define OPCODE_SBB_IMM_B        0x30  // Subtract immediate byte from AX
#define OPCODE_SBH_REG_B        0x31  // Subtract register high from AX high
#define OPCODE_SBL_REG_B        0x32  // Subtract register low from AX low

// ALU operations - Multiplication
#define OPCODE_MUL_IMM_W        0x33  // Multiply AX by immediate word
#define OPCODE_MUL_REG_W        0x34  // Multiply AX by register
#define OPCODE_MLB_IMM_B        0x35  // Multiply AX by immediate byte
#define OPCODE_MLH_REG_B        0x36  // Multiply AX high by register high
#define OPCODE_MLL_REG_B        0x37  // Multiply AX low by register low

// ALU operations - Division
#define OPCODE_DIV_IMM_W        0x38  // Divide AX by immediate word
#define OPCODE_DIV_REG_W        0x39  // Divide AX by register
#define OPCODE_DVB_IMM_B        0x3A  // Divide AX by immediate byte
#define OPCODE_DVH_REG_B        0x3B  // Divide AX high by register high
#define OPCODE_DVL_REG_B        0x3C  // Divide AX low by register low

// ALU operations - Remainder
#define OPCODE_REM_IMM_W        0x3D  // Remainder of AX divided by immediate word
#define OPCODE_REM_REG_W        0x3E  // Remainder of AX divided by register
#define OPCODE_RMB_IMM_B        0x3F  // Remainder of AX divided by immediate byte
#define OPCODE_RMH_REG_B        0x40  // Remainder of AX high divided by register high
#define OPCODE_RML_REG_B        0x41  // Remainder of AX low divided by register low

// ALU operations - Bitwise AND
#define OPCODE_AND_IMM_W        0x42  // AND AX with immediate word
#define OPCODE_AND_REG_W        0x43  // AND AX with register
#define OPCODE_ANB_IMM_B        0x44  // AND AX with immediate byte
#define OPCODE_ANH_REG_B        0x45  // AND AX high with register high
#define OPCODE_ANL_REG_B        0x46  // AND AX low with register low

// ALU operations - Bitwise OR
#define OPCODE_OR_IMM_W         0x47  // OR AX with immediate word
#define OPCODE_OR_REG_W         0x48  // OR AX with register
#define OPCODE_ORB_IMM_B        0x49  // OR AX with immediate byte
#define OPCODE_ORH_REG_B        0x4A  // OR AX high with register high
#define OPCODE_ORL_REG_B        0x4B  // OR AX low with register low

// ALU operations - Bitwise XOR
#define OPCODE_XOR_IMM_W        0x4C  // XOR AX with immediate word
#define OPCODE_XOR_REG_W        0x4D  // XOR AX with register
#define OPCODE_XOB_IMM_B        0x4E  // XOR AX with immediate byte
#define OPCODE_XOH_REG_B        0x4F  // XOR AX high with register high
#define OPCODE_XOL_REG_B        0x50  // XOR AX low with register low

// ALU operations - Bitwise NOT
#define OPCODE_NOT_IMM_W        0x51  // NOT immediate word, store in AX
#define OPCODE_NOT_REG_W        0x52  // NOT register, store in AX
#define OPCODE_NOTB_IMM_B       0x53  // NOT immediate byte, store in AX
#define OPCODE_NOTH_REG_B       0x54  // NOT register high, store in AX
#define OPCODE_NOTL_REG_B       0x55  // NOT register low, store in AX

// ALU operations - Shift Left
#define OPCODE_SHL_IMM_W        0x56  // Shift left immediate word
#define OPCODE_SHL_REG_W        0x57  // Shift left register
#define OPCODE_SLB_IMM_B        0x58  // Shift left immediate byte
#define OPCODE_SLH_REG_B        0x59  // Shift left register high
#define OPCODE_SLL_REG_B        0x5A  // Shift left register low

// ALU operations - Shift Right
#define OPCODE_SHR_IMM_W        0x5B  // Shift right immediate word
#define OPCODE_SHR_REG_W        0x5C  // Shift right register
#define OPCODE_SHRB_IMM_B       0x5D  // Shift right immediate byte
#define OPCODE_SHRH_REG_B       0x5E  // Shift right register high
#define OPCODE_SHRL_REG_B       0x5F  // Shift right register low

// ALU operations - Rotate Left
#define OPCODE_ROL_IMM_W        0x60  // Rotate left immediate word
#define OPCODE_ROL_REG_W        0x61  // Rotate left register
#define OPCODE_ROLB_IMM_B       0x62  // Rotate left immediate byte
#define OPCODE_ROLH_REG_B       0x63  // Rotate left register high
#define OPCODE_ROLL_REG_B       0x64  // Rotate left register low

// ALU operations - Rotate Right
#define OPCODE_ROR_IMM_W        0x65  // Rotate right immediate word
#define OPCODE_ROR_REG_W        0x66  // Rotate right register
#define OPCODE_RORB_IMM_B       0x67  // Rotate right immediate byte
#define OPCODE_RORH_REG_B       0x68  // Rotate right register high
#define OPCODE_RORL_REG_B       0x69  // Rotate right register low

// ALU operations - Increment/Decrement
#define OPCODE_INC_REG          0x6A  // Increment register
#define OPCODE_DEC_REG          0x6B  // Decrement register

// Comparison operations
#define OPCODE_CMP_REG_REG      0x6C  // Compare two registers
#define OPCODE_CMP_REG_IMM_W    0x6D  // Compare register with immediate word
#define OPCODE_CPH_REG_REG      0x6E  // Compare high bytes of two registers
#define OPCODE_CPH_REG_IMM_B    0x6F  // Compare register high with immediate byte
#define OPCODE_CPL_REG_REG      0x70  // Compare low bytes of two registers
#define OPCODE_CPL_REG_IMM_B    0x71  // Compare register low with immediate byte

// System call
#define OPCODE_SYS_FUNC         0x7F  // Call system function

// Extended instruction set marker
#define OPCODE_EXTENDED         0x80  // All higher ops reserved for extended op sets

namespace lvm {
 constexpr int get_additional_bytes(byte_t opcode) {
     // System operations
     if (opcode == OPCODE_NOP) return 0;
     if (opcode == OPCODE_HALT) return 0;
     // Load operations
     if (opcode == OPCODE_LD_REG_IMM_W) return 3;
     if (opcode == OPCODE_LD_REG_REG_W) return 2;
     if (opcode == OPCODE_SWP_REG_REG) return 2;
     if (opcode == OPCODE_LDH_REG_IMM_B) return 2;
     if (opcode == OPCODE_LDH_REG_REG_B) return 2;
     if (opcode == OPCODE_LDL_REG_IMM_B) return 2;
     if (opcode == OPCODE_LDL_REG_REG_B) return 2;
     if (opcode == OPCODE_LDA_REG_ADDR_W) return 3;
     if (opcode == OPCODE_LDAB_REG_ADDR_B) return 3;
     if (opcode == OPCODE_LDAH_REG_ADDR_B) return 3;
     if (opcode == OPCODE_LDAL_REG_ADDR_B) return 3;
     if (opcode == OPCODE_STA_ADDR_REG_W) return 3;
     if (opcode == OPCODE_STAH_ADDR_REG_B) return 3;
     if (opcode == OPCODE_STAL_ADDR_REG_B) return 3;
     // Stack operations
     if (opcode == OPCODE_PUSH_REG_W) return 1;
     if (opcode == OPCODE_PUSHH_REG_B) return 1;
     if (opcode == OPCODE_PUSHL_REG_B) return 1;
     if (opcode == OPCODE_POP_REG_W) return 1;
     if (opcode == OPCODE_POPH_REG_B) return 1;
     if (opcode == OPCODE_POPL_REG_B) return 1;
     if (opcode == OPCODE_PEEK_REG_OFF_W) return 3;
     if (opcode == OPCODE_PEEKF_REG_OFF_W) return 3;
     if (opcode == OPCODE_PEEKB_REG_OFF_B) return 3;
     if (opcode == OPCODE_PEEKFB_REG_OFF_B) return 3;
     if (opcode == OPCODE_FLSH) return 0;
     if (opcode == OPCODE_PAGE_IMM_W) return 2;
     if (opcode == OPCODE_PAGE_REG) return 1;
     if (opcode == OPCODE_SETF_ADDR) return 2;
     // Jump operations
     if (opcode == OPCODE_JMP_ADDR) return 2;
     if (opcode == OPCODE_JPZ_ADDR) return 2;
     if (opcode == OPCODE_JPNZ_ADDR) return 2;
     if (opcode == OPCODE_JPC_ADDR) return 2;
     if (opcode == OPCODE_JPNC_ADDR) return 2;
     if (opcode == OPCODE_JPS_ADDR) return 2;
     if (opcode == OPCODE_JPNS_ADDR) return 2;
     if (opcode == OPCODE_JPO_ADDR) return 2;
     if (opcode == OPCODE_JPNO_ADDR) return 2;
     if (opcode == OPCODE_CALL_ADDR) return 2;
     if (opcode == OPCODE_RET) return 0;
     // ALU - Addition
     if (opcode == OPCODE_ADD_IMM_W) return 2;
     if (opcode == OPCODE_ADD_REG_W) return 1;
     if (opcode == OPCODE_ADB_IMM_B) return 1;
     if (opcode == OPCODE_ADH_REG_B) return 1;
     if (opcode == OPCODE_ADL_REG_B) return 1;
     // ALU - Subtraction
     if (opcode == OPCODE_SUB_IMM_W) return 2;
     if (opcode == OPCODE_SUB_REG_W) return 1;
     if (opcode == OPCODE_SBB_IMM_B) return 1;
     if (opcode == OPCODE_SBH_REG_B) return 1;
     if (opcode == OPCODE_SBL_REG_B) return 1;
     // ALU - Multiplication
     if (opcode == OPCODE_MUL_IMM_W) return 2;
     if (opcode == OPCODE_MUL_REG_W) return 1;
     if (opcode == OPCODE_MLB_IMM_B) return 1;
     if (opcode == OPCODE_MLH_REG_B) return 1;
     if (opcode == OPCODE_MLL_REG_B) return 1;
     // ALU - Division
     if (opcode == OPCODE_DIV_IMM_W) return 2;
     if (opcode == OPCODE_DIV_REG_W) return 1;
     if (opcode == OPCODE_DVB_IMM_B) return 1;
     if (opcode == OPCODE_DVH_REG_B) return 1;
     if (opcode == OPCODE_DVL_REG_B) return 1;
     // ALU - Remainder
     if (opcode == OPCODE_REM_IMM_W) return 2;
     if (opcode == OPCODE_REM_REG_W) return 1;
     if (opcode == OPCODE_RMB_IMM_B) return 1;
     if (opcode == OPCODE_RMH_REG_B) return 1;
     if (opcode == OPCODE_RML_REG_B) return 1;
     // ALU - Bitwise AND
     if (opcode == OPCODE_AND_IMM_W) return 2;
     if (opcode == OPCODE_AND_REG_W) return 1;
     if (opcode == OPCODE_ANB_IMM_B) return 1;
     if (opcode == OPCODE_ANH_REG_B) return 1;
     if (opcode == OPCODE_ANL_REG_B) return 1;
     // ALU - Bitwise OR
     if (opcode == OPCODE_OR_IMM_W) return 2;
     if (opcode == OPCODE_OR_REG_W) return 1;
     if (opcode == OPCODE_ORB_IMM_B) return 1;
     if (opcode == OPCODE_ORH_REG_B) return 1;
     if (opcode == OPCODE_ORL_REG_B) return 1;
     // ALU - Bitwise XOR
     if (opcode == OPCODE_XOR_IMM_W) return 2;
     if (opcode == OPCODE_XOR_REG_W) return 1;
     if (opcode == OPCODE_XOB_IMM_B) return 1;
     if (opcode == OPCODE_XOH_REG_B) return 1;
     if (opcode == OPCODE_XOL_REG_B) return 1;
     // ALU - Bitwise NOT
     if (opcode == OPCODE_NOT_IMM_W) return 2;
     if (opcode == OPCODE_NOT_REG_W) return 1;
     if (opcode == OPCODE_NOTB_IMM_B) return 1;
     if (opcode == OPCODE_NOTH_REG_B) return 1;
     if (opcode == OPCODE_NOTL_REG_B) return 1;
     // ALU - Shift Left
     if (opcode == OPCODE_SHL_IMM_W) return 2;
     if (opcode == OPCODE_SHL_REG_W) return 1;
     if (opcode == OPCODE_SLB_IMM_B) return 1;
     if (opcode == OPCODE_SLH_REG_B) return 1;
     if (opcode == OPCODE_SLL_REG_B) return 1;
     // ALU - Shift Right
     if (opcode == OPCODE_SHR_IMM_W) return 2;
     if (opcode == OPCODE_SHR_REG_W) return 1;
     if (opcode == OPCODE_SHRB_IMM_B) return 1;
     if (opcode == OPCODE_SHRH_REG_B) return 1;
     if (opcode == OPCODE_SHRL_REG_B) return 1;
     // ALU - Rotate Left
     if (opcode == OPCODE_ROL_IMM_W) return 2;
     if (opcode == OPCODE_ROL_REG_W) return 1;
     if (opcode == OPCODE_ROLB_IMM_B) return 1;
     if (opcode == OPCODE_ROLH_REG_B) return 1;
     if (opcode == OPCODE_ROLL_REG_B) return 1;
     // ALU - Rotate Right
     if (opcode == OPCODE_ROR_IMM_W) return 2;
     if (opcode == OPCODE_ROR_REG_W) return 1;
     if (opcode == OPCODE_RORB_IMM_B) return 1;
     if (opcode == OPCODE_RORH_REG_B) return 1;
     if (opcode == OPCODE_RORL_REG_B) return 1;
     // ALU - Increment/Decrement
     if (opcode == OPCODE_INC_REG) return 1;
     if (opcode == OPCODE_DEC_REG) return 1;
     // Comparison
     if (opcode == OPCODE_CMP_REG_REG) return 2;
     if (opcode == OPCODE_CMP_REG_IMM_W) return 3;
     if (opcode == OPCODE_CPH_REG_REG) return 2;
     if (opcode == OPCODE_CPH_REG_IMM_B) return 2;
     if (opcode == OPCODE_CPL_REG_REG) return 2;
     if (opcode == OPCODE_CPL_REG_IMM_B) return 2;
     // System call
     if (opcode == OPCODE_SYS_FUNC) return 2;
     // All others
     return 0;
 }
}
