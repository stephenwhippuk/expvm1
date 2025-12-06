# Pendragon Virtual Machine - Operation Reference

## Instruction Set Architecture

This document describes the complete instruction set for the Pendragon Virtual Machine.

### Instruction Format

Each instruction consists of:
- **Opcode**: 1 byte operation code
- **Arguments**: 0-2 arguments, each can be BYTE (1 byte) or WORD (2 bytes)

### Registers

- **AX**: Accumulator register (16-bit) - primary register for arithmetic operations
- **BX**: Base register (16-bit) - general purpose
- **CX**: Counter register (16-bit) - general purpose
- **DX**: Data register (16-bit) - general purpose
- **EX**: Extended register (16-bit) - general purpose

### Flags

- **ZERO**: Set when result is zero
- **CARRY**: Set when arithmetic operation produces carry/borrow
- **SIGN**: Set when result is negative
- **OVERFLOW**: Set when arithmetic operation overflows

## Complete Instruction Set

| Dec | Hex  | Mnemonic | Arg1 | Size | Arg2 | Size | Description |
|-----|------|----------|------|------|------|------|-------------|
| 0   | 0x00 | NOP      | -    | -    | -    | -    | Empty operation |
| 1   | 0x01 | HALT     | -    | -    | -    | -    | Halts the machine (exiting execution) |
| 2   | 0x02 | LD (1)   | REG  | BYTE | VALUE | WORD | Loads a value into a 16 bit register |
| 3   | 0x03 | LD (2)   | REG  | BYTE | REG  | BYTE | Loads the value of 1 16 bit register into another |
| 4   | 0x04 | SWP      | REG  | BYTE | REG  | BYTE | Swap value of REG1 with REG2 |
| 5   | 0x05 | LDH (1)  | REG  | BYTE | VALUE | BYTE | Loads a byte value into the high byte of a register |
| 6   | 0x06 | LDH (2)  | REG  | BYTE | REG  | BYTE | Loads the high byte of 1 register into the high byte of another |
| 7   | 0x07 | LDL (1)  | REG  | BYTE | VALUE | BYTE | Loads a byte value into the low byte of a register |
| 8   | 0x08 | LDL (2)  | REG  | BYTE | REG  | BYTE | Loads the low byte of a register into the low byte of another |
| 9   | 0x09 | LDA (1)  | REG  | BYTE | ADDR | WORD | Loads a word from memory starting at address into a register |
| 10  | 0x0A | LDAB     | REG  | BYTE | ADDR | WORD | Loads a byte from memory at ADDR into a register |
| 11  | 0x0B | LDAH (1) | REG  | BYTE | ADDR | WORD | Loads a byte of memory stored at address into the high byte of a register |
| 12  | 0x0C | LDAL (1) | REG  | BYTE | ADDR | WORD | Loads a byte of memory stored at address into the low byte of a register |
| 13  | 0x0D | LDA      | ADDR | WORD | REG  | BYTE | Stores a word from a register into memory starting at ADDR |
| 14  | 0x0E | LDAH (1) | ADDR | WORD | REG  | BYTE | Stores a byte from high byte of register into memory at ADDR |
| 15  | 0x0F | LDAL     | ADDR | WORD | REG  | BYTE | Stores a byte from low byte of register into memory at ADDR |
| 16  | 0x10 | PUSH     | REG  | BYTE | -    | -    | Pushes the value of a register to the stack |
| 17  | 0x11 | PUSHH    | REG  | BYTE | -    | -    | Pushes the high byte of a register to the stack |
| 18  | 0x12 | PUSHL    | REG  | BYTE | -    | -    | Pushes low byte of register to the stack |
| 19  | 0x13 | POP      | REG  | BYTE | -    | -    | Pops a word from the stack into a register (cannot pop past SF) |
| 20  | 0x14 | POPH     | REG  | BYTE | -    | -    | Pops a byte from the stack into the high byte of a register (cannot pop past SF) |
| 21  | 0x15 | POPL     | REG  | BYTE | -    | -    | Pops a byte from the stack into the low byte of a register (cannot pop past SF) |
| 22  | 0x16 | PEEK     | REG  | BYTE | OFFSET | WORD | Sets a register equal to a word starting at the Stack BASE - OFFSET |
| 23  | 0x17 | PEEKF    | REG  | BYTE | OFFSET | WORD | Sets a register equal to a word starting at the SF - OFFSET |
| 24  | 0x18 | PEEKB    | REG  | BYTE | OFFSET | WORD | Sets the register equal to a byte stored at STACK BASE - OFFSET |
| 25  | 0x19 | PEEKFB   | REG  | BYTE | OFFSET | WORD | Sets the register equal to a byte stored at SF - OFFSET |
| 26  | 0x1A | FLSH     | -    | -    | -    | -    | Clears the Stack back to SF |
| 27  | 0x1B | PAGE     | VALUE | WORD | -    | -    | Sets the memory page |
| 28  | 0x1C | PAGE     | REG  | BYTE | -    | -    | Sets the memory page using the word value of a register |
| 29  | 0x1D | SETF     | ADDR | WORD | -    | -    | Sets SF to the specified address on the Stack |
| 30  | 0x1E | JMP      | ADDR | WORD | -    | -    | Sets the IR to the address |
| 31  | 0x1F | JPZ      | ADDR | WORD | -    | -    | If zero flag set then sets IR to the address |
| 32  | 0x20 | JPNZ     | ADDR | WORD | -    | -    | If zero flag not set then sets IR to the address |
| 33  | 0x21 | JPC      | ADDR | WORD | -    | -    | If carry flag set then sets IR to the address |
| 34  | 0x22 | JPNC     | ADDR | WORD | -    | -    | If carry flag not set then sets IR to the address |
| 35  | 0x23 | JPS      | ADDR | WORD | -    | -    | If sign flag set then sets IR to the address |
| 36  | 0x24 | JPNS     | ADDR | WORD | -    | -    | If sign flag not set then sets IR to the address |
| 37  | 0x25 | JPO      | ADDR | WORD | -    | -    | If overflow flag set then sets IR to the address |
| 38  | 0x26 | JPNO     | ADDR | WORD | -    | -    | If overflow flag not set then sets IR to the address |
| 39  | 0x27 | CALL     | ADDR | WORD | FLAG | BIT  | Pushes IR and SF to the return stack and sets SF to top of the stack; if bit is set then value at top of stack is kept on return |
| 40  | 0x28 | RET      | -    | -    | -    | -    | Flushes Stack, pops IR and SF off the Return Stack |
| 41  | 0x29 | ADD      | VALUE | WORD | -    | -    | Add WORD to AX; store result in AX and set flags |
| 42  | 0x2A | ADD      | REG  | BYTE | -    | -    | Add REG to AX; store result in AX and set flags |
| 43  | 0x2B | ADB      | VALUE | BYTE | -    | -    | Add BYTE to AX; store result in AX and set flags |
| 44  | 0x2C | ADH      | REG  | BYTE | -    | -    | Add high byte of REG to high byte of AX; store result in AX and set flags |
| 45  | 0x2D | ADL      | REG  | BYTE | -    | -    | Add low byte of REG to low byte of AX; store result in AX and set flags |
| 46  | 0x2E | SUB      | VALUE | WORD | -    | -    | Subtract WORD from AX; store result in AX and set flags |
| 47  | 0x2F | SUB      | REG  | BYTE | -    | -    | Subtract REG from AX; store result in AX and set flags |
| 48  | 0x30 | SBB      | VALUE | BYTE | -    | -    | Subtract BYTE from AX; store result in AX and set flags |
| 49  | 0x31 | SBH      | REG  | BYTE | -    | -    | Subtract high byte of REG from high byte of AX; store result in AX and set flags |
| 50  | 0x32 | SBL      | REG  | BYTE | -    | -    | Subtract low byte of REG from low byte of AX; store result in AX and set flags |
| 51  | 0x33 | MUL      | VALUE | WORD | -    | -    | Multiply WORD by AX; store result in AX and set flags |
| 52  | 0x34 | MUL      | REG  | BYTE | -    | -    | Multiply REG by AX; store result in AX and set flags |
| 53  | 0x35 | MLB      | VALUE | BYTE | -    | -    | Multiply BYTE by AX; store result in AX and set flags |
| 54  | 0x36 | MLH      | REG  | BYTE | -    | -    | Multiply high byte of REG by high byte of AX; store result in AX and set flags |
| 55  | 0x37 | MLL      | REG  | BYTE | -    | -    | Multiply low byte of REG by low byte of AX; store result in AX and set flags |
| 56  | 0x38 | DIV      | VALUE | WORD | -    | -    | Divide WORD by AX; store result in AX and set flags |
| 57  | 0x39 | DIV      | REG  | BYTE | -    | -    | Divide REG by AX; store result in AX and set flags |
| 58  | 0x3A | DVB      | VALUE | BYTE | -    | -    | Divide BYTE by AX; store result in AX and set flags |
| 59  | 0x3B | DVH      | REG  | BYTE | -    | -    | Divide high byte of REG by high byte of AX; store result in AX and set flags |
| 60  | 0x3C | DVL      | REG  | BYTE | -    | -    | Divide low byte of REG by low byte of AX; store result in AX and set flags |
| 61  | 0x3D | REM      | VALUE | WORD | -    | -    | Take remainder after dividing WORD by AX; store result in AX and set flags |
| 62  | 0x3E | REM      | REG  | BYTE | -    | -    | Take remainder after dividing REG by AX; store result in AX and set flags |
| 63  | 0x3F | RMB      | VALUE | BYTE | -    | -    | Take remainder after dividing BYTE by AX; store result in AX and set flags |
| 64  | 0x40 | RMH      | REG  | BYTE | -    | -    | Take remainder after dividing high byte of REG by high byte of AX; store result in AX and set flags |
| 65  | 0x41 | RML      | REG  | BYTE | -    | -    | Take remainder after dividing low byte of REG by low byte of AX; store result in AX and set flags |
| 66  | 0x42 | AND      | VALUE | WORD | -    | -    | And WORD with AX; store result in AX and set flags |
| 67  | 0x43 | AND      | REG  | BYTE | -    | -    | And REG with AX; store result in AX and set flags |
| 68  | 0x44 | ANB      | VALUE | BYTE | -    | -    | And BYTE with low byte of AX; store result in AX and set flags |
| 69  | 0x45 | ANH      | REG  | BYTE | -    | -    | And high byte of REG with high byte of AX; store result in AX and set flags |
| 70  | 0x46 | ANL      | REG  | BYTE | -    | -    | And low byte of REG with low byte of AX; store result in AX and set flags |
| 71  | 0x47 | OR       | VALUE | WORD | -    | -    | Or WORD with AX; store result in AX and set flags |
| 72  | 0x48 | OR       | REG  | BYTE | -    | -    | Or REG with AX; store result in AX and set flags |
| 73  | 0x49 | ORB      | VALUE | BYTE | -    | -    | Or BYTE with low byte of AX; store result in AX and set flags |
| 74  | 0x4A | ORH      | REG  | BYTE | -    | -    | Or high byte of REG with high byte of AX; store result in AX and set flags |
| 75  | 0x4B | ORL      | REG  | BYTE | -    | -    | Or low byte of REG with low byte of AX; store result in AX and set flags |
| 76  | 0x4C | XOR      | VALUE | WORD | -    | -    | XOR WORD with AX; store result in AX and set flags |
| 77  | 0x4D | XOR      | REG  | BYTE | -    | -    | XOR REG with AX; store result in AX and set flags |
| 78  | 0x4E | XOB      | VALUE | BYTE | -    | -    | XOR BYTE with low byte of AX; store result in AX and set flags |
| 79  | 0x4F | XOH      | REG  | BYTE | -    | -    | XOR high byte of REG with high byte of AX; store result in AX and set flags |
| 80  | 0x50 | XOL      | REG  | BYTE | -    | -    | XOR low byte of REG with low byte of AX; store result in AX and set flags |
| 81  | 0x51 | NOT      | VALUE | WORD | -    | -    | Not WORD; store result in AX and set flags |
| 82  | 0x52 | NOT      | REG  | BYTE | -    | -    | Not REG; store result in AX and set flags |
| 83  | 0x53 | NOTB     | VALUE | BYTE | -    | -    | Not BYTE; store result in AX and set flags |
| 84  | 0x54 | NOTH     | REG  | BYTE | -    | -    | Not high byte of REG; store result in AX and set flags |
| 85  | 0x55 | NOTL     | REG  | BYTE | -    | -    | Not low byte of REG; store result in AX and set flags |
| 86  | 0x56 | SHL      | VALUE | WORD | -    | -    | Left shift WORD; store result in AX and set flags |
| 87  | 0x57 | SHL      | REG  | BYTE | -    | -    | Left shift REG; store result in AX and set flags |
| 88  | 0x58 | SLB      | VALUE | BYTE | -    | -    | Left shift BYTE; store result in AX and set flags |
| 89  | 0x59 | SLH      | REG  | BYTE | -    | -    | Left shift high byte of REG; store result in AX and set flags |
| 90  | 0x5A | SLL      | REG  | BYTE | -    | -    | Left shift low byte of REG; store result in AX and set flags |
| 91  | 0x5B | SHR      | VALUE | WORD | -    | -    | Right shift WORD; store result in AX and set flags |
| 92  | 0x5C | SHR      | REG  | BYTE | -    | -    | Right shift REG; store result in AX and set flags |
| 93  | 0x5D | SHRB     | VALUE | BYTE | -    | -    | Right shift BYTE; store result in AX and set flags |
| 94  | 0x5E | SHRH     | REG  | BYTE | -    | -    | Right shift high byte of REG; store result in AX and set flags |
| 95  | 0x5F | SHRL     | REG  | BYTE | -    | -    | Right shift low byte of REG; store result in AX and set flags |
| 96  | 0x60 | ROL      | VALUE | WORD | -    | -    | Left rotate WORD; store result in AX and set flags |
| 97  | 0x61 | ROL      | REG  | BYTE | -    | -    | Left rotate REG; store result in AX and set flags |
| 98  | 0x62 | ROLB     | VALUE | BYTE | -    | -    | Left rotate BYTE; store result in AX and set flags |
| 99  | 0x63 | ROLH     | REG  | BYTE | -    | -    | Left rotate high byte of REG; store result in AX and set flags |
| 100 | 0x64 | ROLL     | REG  | BYTE | -    | -    | Left rotate low byte of REG; store result in AX and set flags |
| 101 | 0x65 | ROR      | VALUE | WORD | -    | -    | Right rotate WORD; store result in AX and set flags |
| 102 | 0x66 | ROR      | REG  | BYTE | -    | -    | Right rotate REG; store result in AX and set flags |
| 103 | 0x67 | RORB     | VALUE | BYTE | -    | -    | Right rotate BYTE; store result in AX and set flags |
| 104 | 0x68 | RORH     | REG  | BYTE | -    | -    | Right rotate high byte of REG; store result in AX and set flags |
| 105 | 0x69 | RORL     | REG  | BYTE | -    | -    | Right rotate low byte of REG; store result in AX and set flags |
| 106 | 0x6A | INC      | REG  | BYTE | -    | -    | Increment register; if AX then set flags |
| 107 | 0x6B | DEC      | REG  | BYTE | -    | -    | Decrement register; if AX then set flags |
| 108 | 0x6C | CMP      | REG  | BYTE | REG  | BYTE | Compare 2 16 bit registers. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 109 | 0x6D | CMP      | REG  | BYTE | VALUE | WORD | Compare 16 bit register with WORD. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 110 | 0x6E | CPH      | REG  | BYTE | REG  | BYTE | Compare high bytes of 2 registers. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 111 | 0x6F | CPH      | REG  | BYTE | VALUE | BYTE | Compare high byte of register with BYTE. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 112 | 0x70 | CPL      | REG  | BYTE | REG  | BYTE | Compare low bytes of 2 registers. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 113 | 0x71 | CPL      | REG  | BYTE | VALUE | BYTE | Compare low byte of register with BYTE. If a < b then set AX = -1, if a = b then set AX = 0, and if a > b set AX = 1 |
| 114 | 0x72 | LDA (2)  | REG  | BYTE | REG  | BYTE | Loads a word from memory at address stored in second register into first register |
| 115 | 0x73 | LDAH (2) | REG  | BYTE | REG  | BYTE | Loads a byte from memory at address stored in second register into high byte of first register |
| 116 | 0x74 | LDAL (2) | REG  | BYTE | REG  | BYTE | Loads a byte from memory at address stored in second register into low byte of first register |
| 127 | 0x7F | SYS      | FUNC | WORD | -    | -    | Call system routine |
| 128+ | 0x80+ | -       | -    | -    | -    | -    | All higher ops reserved for extended op sets |

## Notes

- **SF**: Stack Frame pointer - marks the base of the current stack frame
- **IR**: Instruction Register - program counter/instruction pointer
- **Stack Operations**: The stack grows upward from address 0
- **Little Endian**: All multi-byte values are stored in little-endian format (LSB first)
- **Register Encoding**: Registers are encoded as bytes: AX=0, BX=1, CX=2, DX=3, EX=4
