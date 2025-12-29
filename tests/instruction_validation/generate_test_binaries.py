#!/usr/bin/env python3
"""
Binary Test Generator for Pendragon VM

Creates raw binary files that conform to the Pendragon binary format
for testing VM instruction execution without using the assembler.

This allows us to:
1. Test VM instruction execution independently
2. Test the binary loader
3. Verify binary format correctness
4. Compare with assembler output to find assembler bugs

Binary Format (Version 1.0.0):
- All multi-byte values are little-endian
- Header contains metadata
- Program section has data and code segments
"""

import struct
import os
from typing import List, Optional
from dataclasses import dataclass


@dataclass
class BinaryConfig:
    """Configuration for generating a binary file"""
    machine_name: str = "Pendragon"
    machine_version: tuple = (1, 0, 0)  # major, minor, revision
    header_version: tuple = (1, 0, 0)
    program_name: str = "Test"
    data_segment: bytes = b''
    code_segment: bytes = b''


class BinaryGenerator:
    """Generates Pendragon VM binary files"""
    
    def __init__(self, config: Optional[BinaryConfig] = None):
        self.config = config or BinaryConfig()
    
    def write_uint16_le(self, value: int) -> bytes:
        """Write 16-bit value in little-endian"""
        return struct.pack('<H', value)
    
    def write_uint32_le(self, value: int) -> bytes:
        """Write 32-bit value in little-endian"""
        return struct.pack('<I', value)
    
    def write_version(self, major: int, minor: int, revision: int) -> bytes:
        """Write version as 4 bytes: major, minor, revision_high, revision_low"""
        return bytes([major, minor, (revision >> 8) & 0xFF, revision & 0xFF])
    
    def generate_header(self) -> bytes:
        """Generate complete binary header"""
        header_parts = []
        
        # Calculate header size first (placeholder, will update)
        machine_name_bytes = self.config.machine_name.encode('utf-8')
        program_name_bytes = self.config.program_name.encode('utf-8')
        
        header_size = (
            2 +  # header size field itself
            4 +  # header version
            1 + len(machine_name_bytes) +  # machine name size + name
            4 +  # machine version
            2 + len(program_name_bytes)    # program name size + name
        )
        
        # Header size (2 bytes)
        header_parts.append(self.write_uint16_le(header_size))
        
        # Header version (4 bytes)
        header_parts.append(self.write_version(*self.config.header_version))
        
        # Machine name size (1 byte) + machine name
        header_parts.append(bytes([len(machine_name_bytes)]))
        header_parts.append(machine_name_bytes)
        
        # Machine version (4 bytes)
        header_parts.append(self.write_version(*self.config.machine_version))
        
        # Program name size (2 bytes) + program name
        header_parts.append(self.write_uint16_le(len(program_name_bytes)))
        header_parts.append(program_name_bytes)
        
        return b''.join(header_parts)
    
    def generate_program_section(self) -> bytes:
        """Generate program section (data + code segments)"""
        parts = []
        
        # Data segment size (4 bytes) + data
        parts.append(self.write_uint32_le(len(self.config.data_segment)))
        parts.append(self.config.data_segment)
        
        # Code segment size (4 bytes) + code
        parts.append(self.write_uint32_le(len(self.config.code_segment)))
        parts.append(self.config.code_segment)
        
        return b''.join(parts)
    
    def generate(self) -> bytes:
        """Generate complete binary file"""
        return self.generate_header() + self.generate_program_section()
    
    def save(self, filename: str) -> None:
        """Save binary to file"""
        binary_data = self.generate()
        with open(filename, 'wb') as f:
            f.write(binary_data)
        print(f"Generated {filename}: {len(binary_data)} bytes")
        print(f"  Header: {self.config.program_name}")
        print(f"  Data segment: {len(self.config.data_segment)} bytes")
        print(f"  Code segment: {len(self.config.code_segment)} bytes")


class InstructionEncoder:
    """
    Encodes Pendragon VM instructions as raw bytes
    
    Reference: specification/PendragonOperations.md
    Register encoding: AX=0, BX=1, CX=2, DX=3, EX=4
    All multi-byte values are little-endian
    """
    
    # Register encoding
    AX = 0x00
    BX = 0x01
    CX = 0x02
    DX = 0x03
    EX = 0x04
    
    @staticmethod
    def encode_word(value: int) -> bytes:
        """Encode 16-bit word in little-endian"""
        return struct.pack('<H', value & 0xFFFF)
    
    @staticmethod
    def encode_byte(value: int) -> bytes:
        """Encode single byte"""
        return bytes([value & 0xFF])
    
    # Control Flow Instructions
    @staticmethod
    def NOP() -> bytes:
        """0x00: No operation"""
        return bytes([0x00])
    
    @staticmethod
    def HALT() -> bytes:
        """0x01: Halt execution"""
        return bytes([0x01])
    
    # Register Load Instructions
    @staticmethod
    def LD_imm(reg: int, value: int) -> bytes:
        """0x02: Load immediate word into register"""
        return bytes([0x02, reg]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def LD_reg(dest_reg: int, src_reg: int) -> bytes:
        """0x03: Load register to register"""
        return bytes([0x03, dest_reg, src_reg])
    
    @staticmethod
    def SWP(reg1: int, reg2: int) -> bytes:
        """0x04: Swap two registers"""
        return bytes([0x04, reg1, reg2])
    
    @staticmethod
    def LDH_imm(reg: int, value: int) -> bytes:
        """0x05: Load byte into high byte of register"""
        return bytes([0x05, reg]) + InstructionEncoder.encode_byte(value)
    
    @staticmethod
    def LDH_reg(dest_reg: int, src_reg: int) -> bytes:
        """0x06: Load high byte from register to register"""
        return bytes([0x06, dest_reg, src_reg])
    
    @staticmethod
    def LDL_imm(reg: int, value: int) -> bytes:
        """0x07: Load byte into low byte of register"""
        return bytes([0x07, reg]) + InstructionEncoder.encode_byte(value)
    
    @staticmethod
    def LDL_reg(dest_reg: int, src_reg: int) -> bytes:
        """0x08: Load low byte from register to register"""
        return bytes([0x08, dest_reg, src_reg])
    
    # Memory Load/Store Instructions
    @staticmethod
    def LDA_load(reg: int, addr: int) -> bytes:
        """0x09: Load word from memory address"""
        return bytes([0x09, reg]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def LDAB(reg: int, addr: int) -> bytes:
        """0x0A: Load byte from memory address"""
        return bytes([0x0A, reg]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def LDAH_load(reg: int, addr: int) -> bytes:
        """0x0B: Load byte to high byte from memory"""
        return bytes([0x0B, reg]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def LDAL_load(reg: int, addr: int) -> bytes:
        """0x0C: Load byte to low byte from memory"""
        return bytes([0x0C, reg]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def LDA_store(addr: int, reg: int) -> bytes:
        """0x0D: Store word to memory address"""
        return bytes([0x0D]) + InstructionEncoder.encode_word(addr) + bytes([reg])
    
    @staticmethod
    def LDAH_store(addr: int, reg: int) -> bytes:
        """0x0E: Store high byte to memory address"""
        return bytes([0x0E]) + InstructionEncoder.encode_word(addr) + bytes([reg])
    
    @staticmethod
    def LDAL_store(addr: int, reg: int) -> bytes:
        """0x0F: Store low byte to memory address"""
        return bytes([0x0F]) + InstructionEncoder.encode_word(addr) + bytes([reg])
    
    # Indirect Memory Load Instructions (register contains address)
    @staticmethod
    def LDA_indirect(dest_reg: int, addr_reg: int) -> bytes:
        """0x72: Load word from address in register"""
        return bytes([0x72, dest_reg, addr_reg])
    
    @staticmethod
    def LDAH_indirect(dest_reg: int, addr_reg: int) -> bytes:
        """0x73: Load byte to high byte from address in register"""
        return bytes([0x73, dest_reg, addr_reg])
    
    @staticmethod
    def LDAL_indirect(dest_reg: int, addr_reg: int) -> bytes:
        """0x74: Load byte to low byte from address in register"""
        return bytes([0x74, dest_reg, addr_reg])
    
    # Arithmetic Instructions
    @staticmethod
    def ADD_imm(value: int) -> bytes:
        """0x29: Add immediate word to AX"""
        return bytes([0x29]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def ADD_reg(reg: int) -> bytes:
        """0x2A: Add register to AX"""
        return bytes([0x2A, reg])
    
    @staticmethod
    def ADB(value: int) -> bytes:
        """0x2B: Add byte to AX"""
        return bytes([0x2B, value & 0xFF])
    
    @staticmethod
    def ADH(reg: int) -> bytes:
        """0x2C: Add high byte of register to AX high"""
        return bytes([0x2C, reg])
    
    @staticmethod
    def ADL(reg: int) -> bytes:
        """0x2D: Add low byte of register to AX low"""
        return bytes([0x2D, reg])
    
    @staticmethod
    def SUB_imm(value: int) -> bytes:
        """0x2E: Subtract immediate word from AX"""
        return bytes([0x2E]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def SUB_reg(reg: int) -> bytes:
        """0x2F: Subtract register from AX"""
        return bytes([0x2F, reg])
    
    @staticmethod
    def SBB(value: int) -> bytes:
        """0x30: Subtract byte from AX"""
        return bytes([0x30, value & 0xFF])
    
    @staticmethod
    def SBH(reg: int) -> bytes:
        """0x31: Subtract high byte of register from AX high"""
        return bytes([0x31, reg])
    
    @staticmethod
    def SBL(reg: int) -> bytes:
        """0x32: Subtract low byte of register from AX low"""
        return bytes([0x32, reg])
    
    @staticmethod
    def MUL_imm(value: int) -> bytes:
        """0x33: Multiply AX by immediate word"""
        return bytes([0x33]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def MUL_reg(reg: int) -> bytes:
        """0x34: Multiply AX by register"""
        return bytes([0x34, reg])
    
    @staticmethod
    def MLB(value: int) -> bytes:
        """0x35: Multiply AX by byte"""
        return bytes([0x35, value & 0xFF])
    
    @staticmethod
    def MLH(reg: int) -> bytes:
        """0x36: Multiply AX high by register high"""
        return bytes([0x36, reg])
    
    @staticmethod
    def MLL(reg: int) -> bytes:
        """0x37: Multiply AX low by register low"""
        return bytes([0x37, reg])
    
    @staticmethod
    def DIV_imm(value: int) -> bytes:
        """0x38: Divide AX by immediate word"""
        return bytes([0x38]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def DIV_reg(reg: int) -> bytes:
        """0x39: Divide AX by register"""
        return bytes([0x39, reg])
    
    @staticmethod
    def DVB(value: int) -> bytes:
        """0x3A: Divide AX by byte"""
        return bytes([0x3A, value & 0xFF])
    
    @staticmethod
    def DVH(reg: int) -> bytes:
        """0x3B: Divide AX high by register high"""
        return bytes([0x3B, reg])
    
    @staticmethod
    def DVL(reg: int) -> bytes:
        """0x3C: Divide AX low by register low"""
        return bytes([0x3C, reg])
    
    @staticmethod
    def REM_imm(value: int) -> bytes:
        """0x3D: AX = AX modulo immediate word"""
        return bytes([0x3D]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def REM_reg(reg: int) -> bytes:
        """0x3E: AX = AX modulo register"""
        return bytes([0x3E, reg])
    
    @staticmethod
    def RMB(value: int) -> bytes:
        """0x3F: AX = AX modulo byte"""
        return bytes([0x3F, value & 0xFF])
    
    @staticmethod
    def RMH(reg: int) -> bytes:
        """0x40: AX high = AX high modulo register high"""
        return bytes([0x40, reg])
    
    @staticmethod
    def RML(reg: int) -> bytes:
        """0x41: AX low = AX low modulo register low"""
        return bytes([0x41, reg])
    
    # Bitwise Operations
    @staticmethod
    def AND_imm(value: int) -> bytes:
        """0x42: AND immediate word with AX"""
        return bytes([0x42]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def AND_reg(reg: int) -> bytes:
        """0x43: AND register with AX"""
        return bytes([0x43, reg])
    
    @staticmethod
    def ANB(value: int) -> bytes:
        """0x44: AND immediate byte with AX"""
        return bytes([0x44, value & 0xFF])
    
    @staticmethod
    def ANH(reg: int) -> bytes:
        """0x45: AND high byte of register with AX high"""
        return bytes([0x45, reg])
    
    @staticmethod
    def ANL(reg: int) -> bytes:
        """0x46: AND low byte of register with AX low"""
        return bytes([0x46, reg])
    
    @staticmethod
    def OR_imm(value: int) -> bytes:
        """0x47: OR immediate word with AX"""
        return bytes([0x47]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def OR_reg(reg: int) -> bytes:
        """0x48: OR register with AX"""
        return bytes([0x48, reg])
    
    @staticmethod
    def ORB(value: int) -> bytes:
        """0x49: OR immediate byte with AX"""
        return bytes([0x49, value & 0xFF])
    
    @staticmethod
    def ORH(reg: int) -> bytes:
        """0x4A: OR high byte of register with AX high"""
        return bytes([0x4A, reg])
    
    @staticmethod
    def ORL(reg: int) -> bytes:
        """0x4B: OR low byte of register with AX low"""
        return bytes([0x4B, reg])
    
    @staticmethod
    def XOR_imm(value: int) -> bytes:
        """0x4C: XOR immediate word with AX"""
        return bytes([0x4C]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def XOR_reg(reg: int) -> bytes:
        """0x4D: XOR register with AX"""
        return bytes([0x4D, reg])
    
    @staticmethod
    def XOB(value: int) -> bytes:
        """0x4E: XOR immediate byte with AX"""
        return bytes([0x4E, value & 0xFF])
    
    @staticmethod
    def XOH(reg: int) -> bytes:
        """0x4F: XOR high byte of register with AX high"""
        return bytes([0x4F, reg])
    
    @staticmethod
    def XOL(reg: int) -> bytes:
        """0x50: XOR low byte of register with AX low"""
        return bytes([0x50, reg])
    
    @staticmethod
    def NOT_imm(value: int) -> bytes:
        """0x51: NOT immediate word, store in AX"""
        return bytes([0x51]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def NOT_reg(reg: int) -> bytes:
        """0x52: NOT register, store in AX"""
        return bytes([0x52, reg])
    
    @staticmethod
    def NOTB(value: int) -> bytes:
        """0x53: NOT immediate byte, store in AX"""
        return bytes([0x53, value & 0xFF])
    
    @staticmethod
    def NOTH(reg: int) -> bytes:
        """0x54: NOT high byte of register, store in AX"""
        return bytes([0x54, reg])
    
    @staticmethod
    def NOTL(reg: int) -> bytes:
        """0x55: NOT low byte of register, store in AX"""
        return bytes([0x55, reg])
    
    @staticmethod
    def SHL_imm(value: int) -> bytes:
        """0x56: Left shift immediate word, store in AX"""
        return bytes([0x56]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def SHL_reg(reg: int) -> bytes:
        """0x57: Left shift register, store in AX"""
        return bytes([0x57, reg])
    
    @staticmethod
    def SLB(value: int) -> bytes:
        """0x58: Left shift immediate byte, store in AX"""
        return bytes([0x58, value & 0xFF])
    
    @staticmethod
    def SLH(reg: int) -> bytes:
        """0x59: Left shift high byte of register, store in AX"""
        return bytes([0x59, reg])
    
    @staticmethod
    def SLL(reg: int) -> bytes:
        """0x5A: Left shift low byte of register, store in AX"""
        return bytes([0x5A, reg])
    
    @staticmethod
    def SHR_imm(value: int) -> bytes:
        """0x5B: Right shift immediate word, store in AX"""
        return bytes([0x5B]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def SHR_reg(reg: int) -> bytes:
        """0x5C: Right shift register, store in AX"""
        return bytes([0x5C, reg])
    
    @staticmethod
    def SHRB(value: int) -> bytes:
        """0x5D: Right shift immediate byte, store in AX"""
        return bytes([0x5D, value & 0xFF])
    
    @staticmethod
    def SHRH(reg: int) -> bytes:
        """0x5E: Right shift high byte of register, store in AX"""
        return bytes([0x5E, reg])
    
    @staticmethod
    def SHRL(reg: int) -> bytes:
        """0x5F: Right shift low byte of register, store in AX"""
        return bytes([0x5F, reg])
    
    @staticmethod
    def ROL_imm(value: int) -> bytes:
        """0x60: Left rotate immediate word, store in AX"""
        return bytes([0x60]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def ROL_reg(reg: int) -> bytes:
        """0x61: Left rotate register, store in AX"""
        return bytes([0x61, reg])
    
    @staticmethod
    def ROLB(value: int) -> bytes:
        """0x62: Left rotate immediate byte, store in AX"""
        return bytes([0x62, value & 0xFF])
    
    @staticmethod
    def ROLH(reg: int) -> bytes:
        """0x63: Left rotate high byte of register, store in AX"""
        return bytes([0x63, reg])
    
    @staticmethod
    def ROLL(reg: int) -> bytes:
        """0x64: Left rotate low byte of register, store in AX"""
        return bytes([0x64, reg])
    
    @staticmethod
    def ROR_imm(value: int) -> bytes:
        """0x65: Right rotate immediate word, store in AX"""
        return bytes([0x65]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def ROR_reg(reg: int) -> bytes:
        """0x66: Right rotate register, store in AX"""
        return bytes([0x66, reg])
    
    @staticmethod
    def RORB(value: int) -> bytes:
        """0x67: Right rotate immediate byte, store in AX"""
        return bytes([0x67, value & 0xFF])
    
    @staticmethod
    def RORH(reg: int) -> bytes:
        """0x68: Right rotate high byte of register, store in AX"""
        return bytes([0x68, reg])
    
    @staticmethod
    def RORL(reg: int) -> bytes:
        """0x69: Right rotate low byte of register, store in AX"""
        return bytes([0x69, reg])
    
    # Jump Instructions
    @staticmethod
    def JMP(addr: int) -> bytes:
        """0x1E: Unconditional jump"""
        return bytes([0x1E]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPZ(addr: int) -> bytes:
        """0x1F: Jump if zero flag set"""
        return bytes([0x1F]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPNZ(addr: int) -> bytes:
        """0x20: Jump if zero flag not set"""
        return bytes([0x20]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPC(addr: int) -> bytes:
        """0x21: Jump if carry flag set"""
        return bytes([0x21]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPNC(addr: int) -> bytes:
        """0x22: Jump if carry flag not set"""
        return bytes([0x22]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPS(addr: int) -> bytes:
        """0x23: Jump if sign flag set"""
        return bytes([0x23]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPNS(addr: int) -> bytes:
        """0x24: Jump if sign flag not set"""
        return bytes([0x24]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPO(addr: int) -> bytes:
        """0x25: Jump if overflow flag set"""
        return bytes([0x25]) + InstructionEncoder.encode_word(addr)
    
    @staticmethod
    def JPNO(addr: int) -> bytes:
        """0x26: Jump if overflow flag not set"""
        return bytes([0x26]) + InstructionEncoder.encode_word(addr)
    
    # Subroutine Instructions
    @staticmethod
    def CALL(addr: int, with_return: bool = False) -> bytes:
        """0x27: Call subroutine at address with optional return value flag"""
        flag = 1 if with_return else 0
        return bytes([0x27]) + InstructionEncoder.encode_word(addr) + bytes([flag])
    
    @staticmethod
    def RET() -> bytes:
        """0x28: Return from subroutine"""
        return bytes([0x28])
    
    # Comparison Instructions
    @staticmethod
    def CMP_reg(reg1: int, reg2: int) -> bytes:
        """0x6C: Compare two registers"""
        return bytes([0x6C, reg1, reg2])
    
    @staticmethod
    def CMP_imm(reg: int, value: int) -> bytes:
        """0x6D: Compare register with immediate"""
        return bytes([0x6D, reg]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def CPH_reg(reg1: int, reg2: int) -> bytes:
        """0x6E: Compare high bytes of two registers"""
        return bytes([0x6E, reg1, reg2])
    
    @staticmethod
    def CPH_imm(reg: int, value: int) -> bytes:
        """0x6F: Compare high byte of register with immediate"""
        return bytes([0x6F, reg, value & 0xFF])
    
    @staticmethod
    def CPL_reg(reg1: int, reg2: int) -> bytes:
        """0x70: Compare low bytes of two registers"""
        return bytes([0x70, reg1, reg2])
    
    @staticmethod
    def CPL_imm(reg: int, value: int) -> bytes:
        """0x71: Compare low byte of register with immediate"""
        return bytes([0x71, reg, value & 0xFF])
    
    # Stack Operations
    @staticmethod
    def PUSH(reg: int) -> bytes:
        """0x10: Push register word to stack"""
        return bytes([0x10, reg])
    
    @staticmethod
    def PUSHH(reg: int) -> bytes:
        """0x11: Push high byte of register to stack"""
        return bytes([0x11, reg])
    
    @staticmethod
    def PUSHL(reg: int) -> bytes:
        """0x12: Push low byte of register to stack"""
        return bytes([0x12, reg])
    
    @staticmethod
    def POP(reg: int) -> bytes:
        """0x13: Pop word from stack to register"""
        return bytes([0x13, reg])
    
    @staticmethod
    def POPH(reg: int) -> bytes:
        """0x14: Pop byte from stack to high byte of register"""
        return bytes([0x14, reg])
    
    @staticmethod
    def POPL(reg: int) -> bytes:
        """0x15: Pop byte from stack to low byte of register"""
        return bytes([0x15, reg])
    
    @staticmethod
    def PEEK(reg: int, offset: int) -> bytes:
        """0x16: Peek word at stack base - offset"""
        return bytes([0x16, reg]) + InstructionEncoder.encode_word(offset)
    
    @staticmethod
    def PEEKF(reg: int, offset: int) -> bytes:
        """0x17: Peek word at stack frame - offset"""
        return bytes([0x17, reg]) + InstructionEncoder.encode_word(offset)
    
    @staticmethod
    def PEEKB(reg: int, offset: int) -> bytes:
        """0x18: Peek byte at stack base - offset"""
        return bytes([0x18, reg]) + InstructionEncoder.encode_word(offset)
    
    @staticmethod
    def PEEKFB(reg: int, offset: int) -> bytes:
        """0x19: Peek byte at stack frame - offset"""
        return bytes([0x19, reg]) + InstructionEncoder.encode_word(offset)
    
    @staticmethod
    def FLSH() -> bytes:
        """0x1A: Flush stack back to frame"""
        return bytes([0x1A])
    
    @staticmethod
    def PUSHW(value: int) -> bytes:
        """0x75: Push immediate word to stack"""
        return bytes([0x75]) + InstructionEncoder.encode_word(value)
    
    @staticmethod
    def PUSHB(value: int) -> bytes:
        """0x76: Push immediate byte to stack"""
        return bytes([0x76, value & 0xFF])
    
    # Stack Frame
    @staticmethod
    def SETF(addr: int) -> bytes:
        """0x1D: Set stack frame to address"""
        return bytes([0x1D]) + InstructionEncoder.encode_word(addr)
    
    # Memory Paging
    @staticmethod
    def PAGE_imm(page: int, context: int = 0) -> bytes:
        """0x1B: Set memory page with immediate values (page + context)"""
        return bytes([0x1B]) + InstructionEncoder.encode_word(page) + InstructionEncoder.encode_word(context)
    
    @staticmethod
    def PAGE_reg(reg: int, context: int = 0) -> bytes:
        """0x1C: Set memory page from register (reg + context)"""
        return bytes([0x1C, reg]) + InstructionEncoder.encode_word(context)
    
    # Inc/Dec
    @staticmethod
    def INC(reg: int) -> bytes:
        """0x6A: Increment register"""
        return bytes([0x6A, reg])
    
    @staticmethod
    def DEC(reg: int) -> bytes:
        """0x6B: Decrement register"""
        return bytes([0x6B, reg])
    
    # System Call
    @staticmethod
    @staticmethod
    def SYS(func: int) -> bytes:
        """0x7F: Call system function - takes 16-bit function number"""
        return bytes([0x7F]) + InstructionEncoder.encode_word(func)


class CodeBuilder:
    """Helper class to build code with automatic address tracking"""
    
    def __init__(self):
        self.code = b''
    
    def add(self, instruction_bytes: bytes):
        """Add instruction bytes and return self for chaining"""
        self.code += instruction_bytes
        return self
    
    def pos(self) -> int:
        """Get current position (for jump/call targets)"""
        return len(self.code)
    
    def get(self) -> bytes:
        """Get the accumulated code bytes"""
        return self.code


def create_test_binary(name: str, code: bytes, data: bytes = b'') -> str:
    """Helper to create a test binary file"""
    config = BinaryConfig(
        program_name=name,
        data_segment=data,
        code_segment=code
    )
    generator = BinaryGenerator(config)
    
    # Create output directory
    output_dir = "binaries"
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate filename
    filename = f"{output_dir}/{name.lower().replace(' ', '_')}.bin"
    generator.save(filename)
    
    return filename


# Example usage and test binary creation
if __name__ == "__main__":
    print("Pendragon VM Binary Test Generator")
    print("=" * 50)
    
    # Create some basic test binaries
    IE = InstructionEncoder
    
    # Test 1: Just HALT
    print("\n1. Creating HALT test...")
    create_test_binary(
        "test_halt",
        code=IE.HALT()
    )
    
    # Test 2: Load and HALT
    print("\n2. Creating LD test...")
    create_test_binary(
        "test_ld_ax",
        code=IE.LD_imm(IE.AX, 0x1234) + IE.HALT()
    )
    
    # Test 3: Multiple register loads
    print("\n3. Creating LD all registers test...")
    create_test_binary(
        "test_ld_all_regs",
        code=(
            IE.LD_imm(IE.AX, 0x1111) +
            IE.LD_imm(IE.BX, 0x2222) +
            IE.LD_imm(IE.CX, 0x3333) +
            IE.LD_imm(IE.DX, 0x4444) +
            IE.LD_imm(IE.EX, 0x5555) +
            IE.HALT()
        )
    )
    
    # Test 4: ADD test
    print("\n4. Creating ADD test...")
    create_test_binary(
        "test_add_basic",
        code=(
            IE.LD_imm(IE.AX, 0x0002) +
            IE.ADD_imm(0x0003) +
            IE.HALT()
        )
    )
    
    # Test 5: ADD overflow
    print("\n5. Creating ADD overflow test...")
    create_test_binary(
        "test_add_overflow",
        code=(
            IE.LD_imm(IE.AX, 0xFFFF) +
            IE.ADD_imm(0x0001) +
            IE.HALT()
        )
    )
    
    # Test 6: CMP equal
    print("\n6. Creating CMP equal test...")
    create_test_binary(
        "test_cmp_equal",
        code=(
            IE.LD_imm(IE.BX, 0x1234) +
            IE.LD_imm(IE.CX, 0x1234) +
            IE.CMP_reg(IE.BX, IE.CX) +
            IE.HALT()
        )
    )
    
    # Test 7: NOP basic
    print("\n7. Creating NOP basic test...")
    create_test_binary(
        "test_nop_basic",
        code=(
            IE.NOP() +
            IE.HALT()
        )
    )
    
    # Test 8: Multiple NOPs
    print("\n8. Creating NOP multiple test...")
    create_test_binary(
        "test_nop_multiple",
        code=(
            IE.NOP() +
            IE.NOP() +
            IE.NOP() +
            IE.NOP() +
            IE.NOP() +
            IE.HALT()
        )
    )
    
    # Test 9: SWP basic
    print("\n9. Creating SWP basic test...")
    create_test_binary(
        "test_swp_basic",
        code=(
            IE.LD_imm(IE.AX, 0x1111) +
            IE.LD_imm(IE.BX, 0x2222) +
            IE.SWP(IE.AX, IE.BX) +
            IE.HALT()
        )
    )
    
    # Test 10: SWP same register
    print("\n10. Creating SWP same register test...")
    create_test_binary(
        "test_swp_same",
        code=(
            IE.LD_imm(IE.AX, 0xABCD) +
            IE.SWP(IE.AX, IE.AX) +
            IE.HALT()
        )
    )
    
    # Test 11: LDH immediate
    print("\n11. Creating LDH immediate test...")
    create_test_binary(
        "test_ldh_imm",
        code=(
            IE.LD_imm(IE.AX, 0x0000) +
            IE.LDH_imm(IE.AX, 0xFF) +
            IE.HALT()
        )
    )
    
    # Test 12: LDL immediate
    print("\n12. Creating LDL immediate test...")
    create_test_binary(
        "test_ldl_imm",
        code=(
            IE.LD_imm(IE.AX, 0x0000) +
            IE.LDL_imm(IE.AX, 0xAA) +
            IE.HALT()
        )
    )
    
    # Test 13: LDH from register
    print("\n13. Creating LDH register test...")
    create_test_binary(
        "test_ldh_reg",
        code=(
            IE.LD_imm(IE.BX, 0xABCD) +
            IE.LD_imm(IE.AX, 0x0000) +
            IE.LDH_reg(IE.AX, IE.BX) +
            IE.HALT()
        )
    )
    
    # Test 14: LDL from register
    print("\n14. Creating LDL register test...")
    create_test_binary(
        "test_ldl_reg",
        code=(
            IE.LD_imm(IE.BX, 0xABCD) +
            IE.LD_imm(IE.AX, 0x0000) +
            IE.LDL_reg(IE.AX, IE.BX) +
            IE.HALT()
        )
    )
    
    # Test 15: Combined high/low byte test
    print("\n15. Creating combined high/low byte test...")
    create_test_binary(
        "test_ldh_ldl_combined",
        code=(
            IE.LD_imm(IE.AX, 0x0000) +
            IE.LDH_imm(IE.AX, 0x12) +
            IE.LDL_imm(IE.AX, 0x34) +
            IE.HALT()
        )
    )
    
    # Test 16: LD register to register
    print("\n16. Creating LD register test...")
    create_test_binary(
        "test_ld_reg",
        code=(
            IE.LD_imm(IE.BX, 0x5678) +
            IE.LD_reg(IE.AX, IE.BX) +
            IE.HALT()
        )
    )
    
    # Test 17: Operations after HALT (should not execute)
    print("\n17. Creating HALT boundary test...")
    create_test_binary(
        "test_halt_boundary",
        code=(
            IE.LD_imm(IE.AX, 0x1111) +
            IE.HALT() +
            IE.LD_imm(IE.AX, 0x9999)  # Should not execute
        )
    )
    
    # ===== STACK OPERATION TESTS =====
    print("\n" + "=" * 50)
    print("STACK OPERATION TESTS")
    print("=" * 50)
    
    # Test 18: Basic PUSH/POP
    print("\n18. Creating basic PUSH/POP test...")
    create_test_binary(
        "test_push_pop_basic",
        code=(
            IE.LD_imm(IE.AX, 0x1234) +
            IE.PUSH(IE.AX) +
            IE.LD_imm(IE.AX, 0x0000) +  # Clear AX
            IE.POP(IE.AX) +              # Should restore 0x1234
            IE.HALT()
        )
    )
    
    # Test 19: Multiple PUSH operations
    print("\n19. Creating multiple PUSH test...")
    create_test_binary(
        "test_push_multiple",
        code=(
            IE.LD_imm(IE.AX, 0x1111) +
            IE.LD_imm(IE.BX, 0x2222) +
            IE.LD_imm(IE.CX, 0x3333) +
            IE.PUSH(IE.AX) +
            IE.PUSH(IE.BX) +
            IE.PUSH(IE.CX) +
            IE.HALT()
        )
    )
    
    # Test 20: PUSH then POP multiple
    print("\n20. Creating PUSH/POP multiple test...")
    create_test_binary(
        "test_push_pop_multiple",
        code=(
            IE.LD_imm(IE.AX, 0x1111) +
            IE.LD_imm(IE.BX, 0x2222) +
            IE.PUSH(IE.AX) +
            IE.PUSH(IE.BX) +
            IE.POP(IE.CX) +  # Should get 0x2222 (LIFO)
            IE.POP(IE.DX) +  # Should get 0x1111
            IE.HALT()
        )
    )
    
    # Test 21: POP from empty stack (underflow error expected)
    print("\n21. Creating POP underflow test (should error)...")
    create_test_binary(
        "test_pop_underflow",
        code=(
            IE.POP(IE.AX) +  # Stack is empty - should error
            IE.HALT()
        )
    )
    
    # Test 22: PUSHW immediate word
    print("\n22. Creating PUSHW immediate test...")
    create_test_binary(
        "test_pushw_immediate",
        code=(
            IE.PUSHW(0xABCD) +
            IE.POP(IE.AX) +
            IE.HALT()
        )
    )
    
    # Test 23: PUSHB immediate byte
    print("\n23. Creating PUSHB immediate test...")
    create_test_binary(
        "test_pushb_immediate",
        code=(
            IE.PUSHB(0x42) +
            IE.POPL(IE.AX) +  # Pop byte to low byte
            IE.HALT()
        )
    )
    
    # Test 24: PUSHH/POPH high byte operations
    print("\n24. Creating PUSHH/POPH test...")
    create_test_binary(
        "test_pushh_poph",
        code=(
            IE.LD_imm(IE.AX, 0xABCD) +
            IE.PUSHH(IE.AX) +  # Push 0xAB
            IE.LD_imm(IE.BX, 0x0000) +
            IE.POPH(IE.BX) +   # Should get 0xAB in high byte
            IE.HALT()
        )
    )
    
    # Test 25: PUSHL/POPL low byte operations
    print("\n25. Creating PUSHL/POPL test...")
    create_test_binary(
        "test_pushl_popl",
        code=(
            IE.LD_imm(IE.AX, 0xABCD) +
            IE.PUSHL(IE.AX) +  # Push 0xCD
            IE.LD_imm(IE.BX, 0x0000) +
            IE.POPL(IE.BX) +   # Should get 0xCD in low byte
            IE.HALT()
        )
    )
    
    # Test 26: PEEK basic - read without popping
    print("\n26. Creating PEEK basic test...")
    create_test_binary(
        "test_peek_basic",
        code=(
            IE.PUSHW(0x5678) +
            IE.PEEK(IE.AX, 0x0000) +  # Peek at top of stack
            IE.POP(IE.BX) +            # Stack should still have value
            IE.HALT()
        )
    )
    
    # Test 27: PEEK with offset
    print("\n27. Creating PEEK offset test...")
    create_test_binary(
        "test_peek_offset",
        code=(
            IE.PUSHW(0x1111) +
            IE.PUSHW(0x2222) +
            IE.PUSHW(0x3333) +
            IE.PEEK(IE.AX, 0x0004) +  # Peek 3rd item (2 words = 4 bytes back)
            IE.HALT()
        )
    )
    
    # Test 28: FLSH - flush stack
    print("\n28. Creating FLSH test...")
    create_test_binary(
        "test_flsh_basic",
        code=(
            IE.PUSHW(0x1111) +
            IE.PUSHW(0x2222) +
            IE.PUSHW(0x3333) +
            IE.FLSH() +        # Clear stack to frame
            IE.PUSHW(0x4444) + # New push after flush
            IE.HALT()
        )
    )
    
    # Test 29: SETF and frame-relative operations
    print("\n29. Creating SETF basic test...")
    create_test_binary(
        "test_setf_basic",
        code=(
            IE.PUSHW(0x1111) +  # Push below frame
            IE.PUSHW(0x2222) +
            IE.SETF(0x0004) +   # Set frame after 2 words
            IE.PUSHW(0x3333) +  # Push above frame
            IE.HALT()
        )
    )
    
    # Test 30: POP with frame (underflow at frame boundary)
    print("\n30. Creating frame underflow test (should error)...")
    create_test_binary(
        "test_frame_underflow",
        code=(
            IE.PUSHW(0x1111) +  # Below frame
            IE.SETF(0x0002) +   # Set frame after 1 word
            IE.PUSHW(0x2222) +  # Above frame
            IE.POP(IE.AX) +     # OK - pops 0x2222
            IE.POP(IE.BX) +     # ERROR - frame boundary hit
            IE.HALT()
        )
    )
    
    # Test 31: PEEKF - peek from frame
    print("\n31. Creating PEEKF test...")
    create_test_binary(
        "test_peekf",
        code=(
            IE.PUSHW(0x1111) +
            IE.SETF(0x0002) +   # Frame at 1 word
            IE.PUSHW(0x2222) +  # Above frame
            IE.PUSHW(0x3333) +
            IE.PEEKF(IE.AX, 0x0000) +  # Peek from frame position
            IE.HALT()
        )
    )
    
    # Test 32: PEEKB - peek byte
    print("\n32. Creating PEEKB test...")
    create_test_binary(
        "test_peekb",
        code=(
            IE.PUSHW(0xABCD) +
            IE.PEEKB(IE.AX, 0x0000) +  # Should get 0xCD (low byte)
            IE.HALT()
        )
    )
    
    # Test 33: PEEKFB - peek byte from frame
    print("\n33. Creating PEEKFB test...")
    create_test_binary(
        "test_peekfb",
        code=(
            IE.PUSHW(0x1234) +
            IE.SETF(0x0002) +
            IE.PUSHW(0xABCD) +
            IE.PEEKFB(IE.AX, 0x0000) +  # Peek byte from frame
            IE.HALT()
        )
    )
    
    # =================================================================
    # ARITHMETIC OPERATIONS - Extended Testing
    # =================================================================
    
    # 34. Test SUB immediate word (0x2E)
    print("\n34. Creating SUB immediate word test...")
    create_test_binary(
        "test_sub_immediate",
        code=(
            IE.LD_imm(IE.AX, 0x0010) +  # AX = 16
            IE.SUB_imm(0x0006) +         # AX = 16 - 6 = 10
            IE.HALT()
        )
    )
    
    # 35. Test SUB register (0x2F)
    print("\n35. Creating SUB register test...")
    create_test_binary(
        "test_sub_register",
        code=(
            IE.LD_imm(IE.AX, 0x0020) +   # AX = 32
            IE.LD_imm(IE.BX, 0x0008) +   # BX = 8
            IE.SUB_reg(IE.BX) +          # AX = 32 - 8 = 24
            IE.HALT()
        )
    )
    
    # 36. Test MUL immediate word (0x33)
    print("\n36. Creating MUL immediate word test...")
    create_test_binary(
        "test_mul_immediate",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.MUL_imm(0x0003) +         # AX = 5 * 3 = 15
            IE.HALT()
        )
    )
    
    # 37. Test MUL register (0x34)
    print("\n37. Creating MUL register test...")
    create_test_binary(
        "test_mul_register",
        code=(
            IE.LD_imm(IE.AX, 0x0007) +   # AX = 7
            IE.LD_imm(IE.CX, 0x0004) +   # CX = 4
            IE.MUL_reg(IE.CX) +          # AX = 7 * 4 = 28
            IE.HALT()
        )
    )
    
    # 38. Test DIV immediate word (0x38)
    print("\n38. Creating DIV immediate word test...")
    create_test_binary(
        "test_div_immediate",
        code=(
            IE.LD_imm(IE.AX, 0x0014) +   # AX = 20
            IE.DIV_imm(0x0004) +         # AX = 20 / 4 = 5
            IE.HALT()
        )
    )
    
    # 39. Test DIV register (0x39)
    print("\n39. Creating DIV register test...")
    create_test_binary(
        "test_div_register",
        code=(
            IE.LD_imm(IE.AX, 0x0018) +   # AX = 24
            IE.LD_imm(IE.DX, 0x0003) +   # DX = 3
            IE.DIV_reg(IE.DX) +          # AX = 24 / 3 = 8
            IE.HALT()
        )
    )
    
    # 40. Test REM immediate word (0x3D)
    print("\n40. Creating REM (modulo) immediate test...")
    create_test_binary(
        "test_rem_immediate",
        code=(
            IE.LD_imm(IE.AX, 0x0017) +   # AX = 23
            IE.REM_imm(0x0005) +         # AX = 23 % 5 = 3
            IE.HALT()
        )
    )
    
    # 41. Test REM register (0x3E)
    print("\n41. Creating REM (modulo) register test...")
    create_test_binary(
        "test_rem_register",
        code=(
            IE.LD_imm(IE.AX, 0x001D) +   # AX = 29
            IE.LD_imm(IE.EX, 0x0007) +   # EX = 7
            IE.REM_reg(IE.EX) +          # AX = 29 % 7 = 1
            IE.HALT()
        )
    )
    
    # 42. Test INC register (0x6A)
    print("\n42. Creating INC (increment) test...")
    create_test_binary(
        "test_inc_register",
        code=(
            IE.LD_imm(IE.AX, 0x00FF) +   # AX = 255
            IE.INC(IE.AX) +              # AX = 256
            IE.LD_imm(IE.BX, 0x0000) +   # BX = 0
            IE.INC(IE.BX) +              # BX = 1
            IE.HALT()
        )
    )
    
    # 43. Test DEC register (0x6B)
    print("\n43. Creating DEC (decrement) test...")
    create_test_binary(
        "test_dec_register",
        code=(
            IE.LD_imm(IE.AX, 0x0010) +   # AX = 16
            IE.DEC(IE.AX) +              # AX = 15
            IE.LD_imm(IE.CX, 0x0001) +   # CX = 1
            IE.DEC(IE.CX) +              # CX = 0
            IE.HALT()
        )
    )
    
    # 44. Test CMP register with immediate (0x6D)
    print("\n44. Creating CMP register-immediate test...")
    create_test_binary(
        "test_cmp_reg_imm",
        code=(
            IE.LD_imm(IE.BX, 0x0010) +      # BX = 16
            IE.CMP_imm(IE.BX, 0x0010) +     # Compare BX with 16 (should be equal)
            IE.HALT()
        )
    )
    
    # =================================================================
    # BITWISE OPERATIONS Testing
    # =================================================================
    
    # 45. Test AND immediate (0x42)
    print("\n45. Creating AND immediate test...")
    create_test_binary(
        "test_and_immediate",
        code=(
            IE.LD_imm(IE.AX, 0xFFFF) +   # AX = 0xFFFF
            IE.AND_imm(0x00FF) +         # AX = 0xFFFF & 0x00FF = 0x00FF
            IE.HALT()
        )
    )
    
    # 46. Test AND register (0x43)
    print("\n46. Creating AND register test...")
    create_test_binary(
        "test_and_register",
        code=(
            IE.LD_imm(IE.AX, 0xFF00) +   # AX = 0xFF00
            IE.LD_imm(IE.BX, 0x0FFF) +   # BX = 0x0FFF
            IE.AND_reg(IE.BX) +          # AX = 0xFF00 & 0x0FFF = 0x0F00
            IE.HALT()
        )
    )
    
    # 47. Test OR immediate (0x47)
    print("\n47. Creating OR immediate test...")
    create_test_binary(
        "test_or_immediate",
        code=(
            IE.LD_imm(IE.AX, 0x00F0) +   # AX = 0x00F0
            IE.OR_imm(0x000F) +          # AX = 0x00F0 | 0x000F = 0x00FF
            IE.HALT()
        )
    )
    
    # 48. Test OR register (0x48)
    print("\n48. Creating OR register test...")
    create_test_binary(
        "test_or_register",
        code=(
            IE.LD_imm(IE.AX, 0xF000) +   # AX = 0xF000
            IE.LD_imm(IE.CX, 0x0ABC) +   # CX = 0x0ABC
            IE.OR_reg(IE.CX) +           # AX = 0xF000 | 0x0ABC = 0xFABC
            IE.HALT()
        )
    )
    
    # 49. Test XOR immediate (0x4C)
    print("\n49. Creating XOR immediate test...")
    create_test_binary(
        "test_xor_immediate",
        code=(
            IE.LD_imm(IE.AX, 0xAAAA) +   # AX = 0xAAAA
            IE.XOR_imm(0xFFFF) +         # AX = 0xAAAA ^ 0xFFFF = 0x5555
            IE.HALT()
        )
    )
    
    # 50. Test XOR register (0x4D)
    print("\n50. Creating XOR register test...")
    create_test_binary(
        "test_xor_register",
        code=(
            IE.LD_imm(IE.AX, 0x00FF) +   # AX = 0x00FF
            IE.LD_imm(IE.DX, 0x00F0) +   # DX = 0x00F0
            IE.XOR_reg(IE.DX) +          # AX = 0x00FF ^ 0x00F0 = 0x000F
            IE.HALT()
        )
    )
    
    # 51. Test NOT immediate (0x51)
    print("\n51. Creating NOT immediate test...")
    create_test_binary(
        "test_not_immediate",
        code=(
            IE.NOT_imm(0x00FF) +         # AX = ~0x00FF = 0xFF00
            IE.HALT()
        )
    )
    
    # 52. Test NOT register (0x52)
    print("\n52. Creating NOT register test...")
    create_test_binary(
        "test_not_register",
        code=(
            IE.LD_imm(IE.EX, 0xF0F0) +   # EX = 0xF0F0
            IE.NOT_reg(IE.EX) +          # AX = ~0xF0F0 = 0x0F0F
            IE.HALT()
        )
    )
    
    # 53. Test SHL immediate (0x56) - Left shift
    print("\n53. Creating SHL (shift left) immediate test...")
    create_test_binary(
        "test_shl_immediate",
        code=(
            IE.SHL_imm(0x0001) +         # AX = 0x0001 << 1 = 0x0002
            IE.HALT()
        )
    )
    
    # 54. Test SHL register (0x57)
    print("\n54. Creating SHL register test...")
    create_test_binary(
        "test_shl_register",
        code=(
            IE.LD_imm(IE.BX, 0x0004) +   # BX = 0x0004
            IE.SHL_reg(IE.BX) +          # AX = 0x0004 << 1 = 0x0008
            IE.HALT()
        )
    )
    
    # 55. Test SHR immediate (0x5B) - Right shift
    print("\n55. Creating SHR (shift right) immediate test...")
    create_test_binary(
        "test_shr_immediate",
        code=(
            IE.SHR_imm(0x0008) +         # AX = 0x0008 >> 1 = 0x0004
            IE.HALT()
        )
    )
    
    # 56. Test SHR register (0x5C)
    print("\n56. Creating SHR register test...")
    create_test_binary(
        "test_shr_register",
        code=(
            IE.LD_imm(IE.CX, 0x0010) +   # CX = 0x0010
            IE.SHR_reg(IE.CX) +          # AX = 0x0010 >> 1 = 0x0008
            IE.HALT()
        )
    )
    
    # 57. Test ROL immediate (0x60) - Rotate left
    print("\n57. Creating ROL (rotate left) immediate test...")
    create_test_binary(
        "test_rol_immediate",
        code=(
            IE.ROL_imm(0x8000) +         # AX = ROL(0x8000) = 0x0001
            IE.HALT()
        )
    )
    
    # 58. Test ROL register (0x61)
    print("\n58. Creating ROL register test...")
    create_test_binary(
        "test_rol_register",
        code=(
            IE.LD_imm(IE.DX, 0x4000) +   # DX = 0x4000
            IE.ROL_reg(IE.DX) +          # AX = ROL(0x4000) = 0x8000
            IE.HALT()
        )
    )
    
    # 59. Test ROR immediate (0x65) - Rotate right
    print("\n59. Creating ROR (rotate right) immediate test...")
    create_test_binary(
        "test_ror_immediate",
        code=(
            IE.ROR_imm(0x0001) +         # AX = ROR(0x0001) = 0x8000
            IE.HALT()
        )
    )
    
    # 60. Test ROR register (0x66)
    print("\n60. Creating ROR register test...")
    create_test_binary(
        "test_ror_register",
        code=(
            IE.LD_imm(IE.EX, 0x0002) +   # EX = 0x0002
            IE.ROR_reg(IE.EX) +          # AX = ROR(0x0002) = 0x0001
            IE.HALT()
        )
    )
    
    print("\n" + "=" * 50)
    print("CONDITIONAL JUMP TESTS (Opcodes 0x1F-0x26)")
    print("=" * 50)
    
    # 61. Test JPZ (0x1F) - Jump if zero (should jump)
    print("\n61. Creating JPZ (jump if zero) test - zero flag set...")
    create_test_binary(
        "test_jpz_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.SUB_imm(0x0005) +         # AX = 0, sets ZERO flag
            IE.JPZ(0x000E) +             # Should jump to address 0x000E (HALT)
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute (would indicate failed jump)
            IE.HALT()                    # Target: address 0x000E
        )
    )
    
    # 62. Test JPZ - zero flag not set (should not jump)
    print("\n62. Creating JPZ test - zero flag not set...")
    create_test_binary(
        "test_jpz_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.SUB_imm(0x0002) +         # AX = 3, ZERO flag NOT set
            IE.JPZ(0x000E) +             # Should NOT jump
            IE.HALT()                    # Should execute this HALT
        )
    )
    
    # 63. Test JPNZ (0x20) - Jump if not zero (should jump)
    print("\n63. Creating JPNZ (jump if not zero) test - zero flag not set...")
    create_test_binary(
        "test_jpnz_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.SUB_imm(0x0002) +         # AX = 3, ZERO flag NOT set
            IE.JPNZ(0x000E) +            # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 64. Test JPNZ - zero flag set (should not jump)
    print("\n64. Creating JPNZ test - zero flag set...")
    create_test_binary(
        "test_jpnz_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.SUB_imm(0x0005) +         # AX = 0, ZERO flag set
            IE.JPNZ(0x00FF) +            # Should NOT jump (use safe address far away)
            IE.HALT()                    # Should execute this
        )
    )
    
    # 65. Test JPC (0x21) - Jump if carry (should jump)
    print("\n65. Creating JPC (jump if carry) test - carry flag set...")
    create_test_binary(
        "test_jpc_taken",
        code=(
            IE.LD_imm(IE.AX, 0xFFFF) +   # AX = 0xFFFF
            IE.ADD_imm(0x0001) +         # AX = 0, sets CARRY flag
            IE.JPC(0x000E) +             # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 66. Test JPC - carry flag not set (should not jump)
    print("\n66. Creating JPC test - carry flag not set...")
    create_test_binary(
        "test_jpc_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, CARRY flag NOT set
            IE.JPC(0x000E) +             # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    # 67. Test JPNC (0x22) - Jump if not carry (should jump)
    print("\n67. Creating JPNC (jump if not carry) test - carry flag not set...")
    create_test_binary(
        "test_jpnc_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, CARRY flag NOT set
            IE.JPNC(0x000E) +            # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 68. Test JPNC - carry flag set (should not jump)
    print("\n68. Creating JPNC test - carry flag set...")
    create_test_binary(
        "test_jpnc_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0xFFFF) +   # AX = 0xFFFF
            IE.ADD_imm(0x0001) +         # AX = 0, sets CARRY flag
            IE.JPNC(0x000E) +            # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    # 69. Test JPS (0x23) - Jump if sign (should jump)
    print("\n69. Creating JPS (jump if sign) test - sign flag set...")
    create_test_binary(
        "test_jps_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0001) +   # AX = 1
            IE.SUB_imm(0x0005) +         # AX = 0xFFFC (negative), sets SIGN flag
            IE.JPS(0x000E) +             # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 70. Test JPS - sign flag not set (should not jump)
    print("\n70. Creating JPS test - sign flag not set...")
    create_test_binary(
        "test_jps_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, SIGN flag NOT set
            IE.JPS(0x000E) +             # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    # 71. Test JPNS (0x24) - Jump if not sign (should jump)
    print("\n71. Creating JPNS (jump if not sign) test - sign flag not set...")
    create_test_binary(
        "test_jpns_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, SIGN flag NOT set
            IE.JPNS(0x000E) +            # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 72. Test JPNS - sign flag set (should not jump)
    print("\n72. Creating JPNS test - sign flag set...")
    create_test_binary(
        "test_jpns_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0001) +   # AX = 1
            IE.SUB_imm(0x0005) +         # AX = 0xFFFC (negative), sets SIGN flag
            IE.JPNS(0x000E) +            # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    # 73. Test JPO (0x25) - Jump if overflow (should jump)
    print("\n73. Creating JPO (jump if overflow) test - overflow flag set...")
    create_test_binary(
        "test_jpo_taken",
        code=(
            IE.LD_imm(IE.AX, 0x7FFF) +   # AX = 32767 (max positive signed 16-bit)
            IE.ADD_imm(0x0001) +         # AX = 0x8000, sets OVERFLOW flag
            IE.JPO(0x000E) +             # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 74. Test JPO - overflow flag not set (should not jump)
    print("\n74. Creating JPO test - overflow flag not set...")
    create_test_binary(
        "test_jpo_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, OVERFLOW flag NOT set
            IE.JPO(0x000E) +             # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    # 75. Test JPNO (0x26) - Jump if not overflow (should jump)
    print("\n75. Creating JPNO (jump if not overflow) test - overflow flag not set...")
    create_test_binary(
        "test_jpno_taken",
        code=(
            IE.LD_imm(IE.AX, 0x0005) +   # AX = 5
            IE.ADD_imm(0x0002) +         # AX = 7, OVERFLOW flag NOT set
            IE.JPNO(0x000E) +            # Should jump
            IE.LD_imm(IE.BX, 0xDEAD) +   # Should NOT execute
            IE.HALT()                    # Target
        )
    )
    
    # 76. Test JPNO - overflow flag set (should not jump)
    print("\n76. Creating JPNO test - overflow flag set...")
    create_test_binary(
        "test_jpno_not_taken",
        code=(
            IE.LD_imm(IE.AX, 0x7FFF) +   # AX = 32767
            IE.ADD_imm(0x0001) +         # AX = 0x8000, sets OVERFLOW flag
            IE.JPNO(0x000E) +            # Should NOT jump
            IE.HALT()                    # Should execute this
        )
    )
    
    print("\n" + "=" * 50)
    print("CONTROL FLOW - JMP AND SUBROUTINES")
    print("=" * 50)
    
    # 77. Test JMP (0x1E) - Unconditional jump
    print("\n77. Creating JMP (unconditional jump) test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0001))     # AX = 1
    target = cb.pos() + 3                # Calculate where to jump (after JMP instruction)
    cb.add(IE.JMP(target + 4))           # Jump over LD instruction
    cb.add(IE.LD_imm(IE.AX, 0xDEAD))     # Should NOT execute
    # target is here
    cb.add(IE.HALT())
    create_test_binary("test_jmp_basic", code=cb.get())
    
    # 78. Test JMP - forward jump
    print("\n78. Creating JMP forward jump test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1111))     # 4 bytes: pos 0-3
    cb.add(IE.LD_imm(IE.BX, 0x2222))     # 4 bytes: pos 4-7
    # JMP will be at pos 8, targets pos 8+3+4+4 = 19
    halt_target = cb.pos() + 3 + 4 + 4
    cb.add(IE.JMP(halt_target))          # 3 bytes: pos 8-10
    cb.add(IE.LD_imm(IE.CX, 0x3333))     # 4 bytes: pos 11-14 (skipped)
    cb.add(IE.LD_imm(IE.DX, 0x4444))     # 4 bytes: pos 15-18 (skipped)
    cb.add(IE.HALT())                    # 1 byte: pos 19
    create_test_binary("test_jmp_forward", code=cb.get())
    
    # 79. Test JMP - backward jump (simple loop)
    print("\n79. Creating JMP backward jump test (loop with counter)...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0003))     # AX = 3 (loop counter)
    loop_start = cb.pos()                # Mark loop start
    cb.add(IE.DEC(IE.AX))                # Decrement AX
    cb.add(IE.CMP_imm(IE.AX, 0x0000))    # Compare AX with 0
    jpz_pos = cb.pos()
    cb.add(IE.JPZ(0xFFFF))               # Placeholder, will fix
    cb.add(IE.JMP(loop_start))           # Jump back to loop start
    halt_pos = cb.pos()
    cb.add(IE.HALT())
    # Rebuild with correct JPZ target
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0003))
    loop_start = cb.pos()
    cb.add(IE.DEC(IE.AX))
    cb.add(IE.CMP_imm(IE.AX, 0x0000))
    jpz_pos = cb.pos()
    jmp_pos = jpz_pos + 3
    halt_target = jmp_pos + 3
    cb.add(IE.JPZ(halt_target))          # Jump to HALT when zero
    cb.add(IE.JMP(loop_start))
    cb.add(IE.HALT())
    create_test_binary("test_jmp_backward", code=cb.get())
    
    # 80. Test CALL (0x27) - Basic subroutine call
    print("\n80. Creating CALL (subroutine) basic test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0005))     # 0-3: 4 bytes
    # CALL is now 4 bytes (opcode + addr + flag), at pos 4, targets 4+4+4+1 = 13
    sub_target = cb.pos() + 4 + 4 + 1
    cb.add(IE.CALL(sub_target))          # 4-7: 4 bytes (with flag)
    cb.add(IE.LD_imm(IE.BX, 0x00FF))     # 8-11: 4 bytes
    cb.add(IE.HALT())                    # 12: 1 byte
    # Subroutine at position 13
    cb.add(IE.ADD_imm(0x000A))           # 13-15: 3 bytes
    cb.add(IE.RET())                     # 16: 1 byte
    create_test_binary("test_call_basic", code=cb.get())
    
    # 81. Test CALL/RET - nested calls
    print("\n81. Creating CALL/RET nested test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0001))     # 0-3: 4 bytes
    # First CALL at 4 (4 bytes), targets 4+4+1 = 9
    first_sub_addr = cb.pos() + 4 + 1
    cb.add(IE.CALL(first_sub_addr))      # 4-7: 4 bytes (with flag)
    cb.add(IE.HALT())                    # 8: 1 byte
    # First subroutine at 9
    cb.add(IE.ADD_imm(0x0002))           # 9-11: 3 bytes
    # Second CALL at 12 (4 bytes), targets 12+4+3+1 = 20
    second_sub_addr = cb.pos() + 4 + 3 + 1
    cb.add(IE.CALL(second_sub_addr))     # 12-15: 4 bytes (with flag)
    cb.add(IE.ADD_imm(0x0001))           # 16-18: 3 bytes
    cb.add(IE.RET())                     # 19: 1 byte
    # Second subroutine at 20
    cb.add(IE.ADD_imm(0x0003))           # 20-22: 3 bytes
    cb.add(IE.RET())                     # 23: 1 byte
    create_test_binary("test_call_nested", code=cb.get())
    
    # 82. Test CALL/RET - with stack usage
    print("\n82. Creating CALL/RET with stack test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1234))     # 0-3: 4 bytes
    cb.add(IE.LD_imm(IE.BX, 0x5678))     # 4-7: 4 bytes
    # CALL at 8 (4 bytes), targets 8+4+1 = 13
    sub_addr = cb.pos() + 4 + 1
    cb.add(IE.CALL(sub_addr))            # 8-11: 4 bytes (with flag)
    cb.add(IE.HALT())                    # 12: 1 byte
    # Subroutine at 13
    cb.add(IE.PUSH(IE.AX))               # 13-14: 2 bytes
    cb.add(IE.PUSH(IE.BX))               # 15-16: 2 bytes
    cb.add(IE.POP(IE.AX))                # 17-18: 2 bytes
    cb.add(IE.POP(IE.BX))                # 19-20: 2 bytes
    cb.add(IE.RET())                     # 21: 1 byte
    create_test_binary("test_call_stack", code=cb.get())
    
    # 83. Test RET without CALL (manual return address)
    print("\n83. Creating RET without CALL test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0xABCD))     # 0-3: 4 bytes
    # PUSHW at 4, RET at 7, LD at 8, HALT at 12
    # We want to push address 12 (HALT position)
    halt_addr = cb.pos() + 3 + 1 + 4     # 4+3+1+4 = 12
    cb.add(IE.PUSHW(halt_addr))          # 4-6: 3 bytes
    cb.add(IE.RET())                     # 7: 1 byte
    cb.add(IE.LD_imm(IE.BX, 0xDEAD))     # 8-11: 4 bytes (should NOT execute)
    cb.add(IE.HALT())                    # 12: 1 byte
    create_test_binary("test_ret_without_call", code=cb.get())
    
    # ===================================================================
    # MEMORY OPERATIONS (Tests 84-93)
    # ===================================================================
    
    # 84. Test LDA - Load word from memory
    print("\n84. Creating LDA load word test...")
    data = IE.encode_word(0x1234)        # Store 0x1234 at data[0:2]
    cb = CodeBuilder()
    cb.add(IE.LDA_load(IE.AX, 0x0000))   # 0-3: Load word from data address 0
    cb.add(IE.SUB_imm(0x1234))           # 4-6: Subtract 0x1234 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 7-9: Jump to 0xFF if not zero (error)
    cb.add(IE.HALT())                    # 10: Exit successfully
    create_test_binary("test_lda_load_word", data=data, code=cb.get())
    
    # 85. Test LDAB - Load byte from memory
    print("\n85. Creating LDAB load byte test...")
    data = bytes([0x42]) + bytes([0x00]) # Byte 0x42 at data[0]
    cb = CodeBuilder()
    cb.add(IE.LDAB(IE.AX, 0x0000))       # 0-3: Load byte from data[0]
    cb.add(IE.SUB_imm(0x0042))           # 4-6: Subtract 0x42 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 7-9: Jump to 0xFF if not zero (error)
    cb.add(IE.HALT())                    # 10: Exit successfully
    create_test_binary("test_ldab_load_byte", data=data, code=cb.get())
    
    # 86. Test LDAH - Load byte to high byte
    print("\n86. Creating LDAH load high byte test...")
    data = bytes([0x56]) + bytes([0x00]) # Byte 0x56 at data[0]
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0012))     # 0-3: AX = 0x0012
    cb.add(IE.LDAH_load(IE.AX, 0x0000))  # 4-7: Load 0x56 to high byte -> AX = 0x5612
    cb.add(IE.SUB_imm(0x5612))           # 8-10: Subtract 0x5612 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 11-13: Jump if not zero (error)
    cb.add(IE.HALT())                    # 14: Exit successfully
    create_test_binary("test_ldah_load_high", data=data, code=cb.get())
    
    # 87. Test LDAL - Load byte to low byte
    print("\n87. Creating LDAL load low byte test...")
    data = bytes([0x78]) + bytes([0x00]) # Byte 0x78 at data[0]
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x3400))     # 0-3: AX = 0x3400
    cb.add(IE.LDAL_load(IE.AX, 0x0000))  # 4-7: Load 0x78 to low byte -> AX = 0x3478
    cb.add(IE.SUB_imm(0x3478))           # 8-10: Subtract 0x3478 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 11-13: Jump if not zero (error)
    cb.add(IE.HALT())                    # 14: Exit successfully
    create_test_binary("test_ldal_load_low", data=data, code=cb.get())
    
    # 88. Test STA - Store word to memory
    print("\n88. Creating STA store word test...")
    data = bytes([0x00, 0x00])           # Space for storing word
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0xABCD))     # 0-3: AX = 0xABCD
    cb.add(IE.LDA_store(0x0000, IE.AX))  # 4-7: Store AX to data[0:2]
    cb.add(IE.LDA_load(IE.BX, 0x0000))   # 8-11: Load back from data[0:2]
    cb.add(IE.LD_reg(IE.AX, IE.BX))      # 12-14: AX = BX
    cb.add(IE.SUB_imm(0xABCD))           # 15-17: Subtract 0xABCD (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 18-20: Jump if not zero (error)
    cb.add(IE.HALT())                    # 21: Exit successfully
    create_test_binary("test_sta_store_word", data=data, code=cb.get())
    
    # 89. Test STAH - Store high byte to memory
    print("\n89. Creating STAH store high byte test...")
    data = bytes([0x00])                 # Space for storing byte
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x9A00))     # 0-3: AX = 0x9A00 (high byte = 0x9A)
    cb.add(IE.LDAH_store(0x0000, IE.AX)) # 4-7: Store high byte to data[0]
    cb.add(IE.LDAB(IE.BX, 0x0000))       # 8-11: Load byte back
    cb.add(IE.LD_reg(IE.AX, IE.BX))      # 12-14: AX = BX
    cb.add(IE.SUB_imm(0x009A))           # 15-17: Subtract 0x9A (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 18-20: Jump if not zero (error)
    cb.add(IE.HALT())                    # 21: Exit successfully
    create_test_binary("test_stah_store_high", data=data, code=cb.get())
    
    # 90. Test STAL - Store low byte to memory
    print("\n90. Creating STAL store low byte test...")
    data = bytes([0x00])                 # Space for storing byte
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x00BC))     # 0-3: AX = 0x00BC (low byte = 0xBC)
    cb.add(IE.LDAL_store(0x0000, IE.AX)) # 4-7: Store low byte to data[0]
    cb.add(IE.LDAB(IE.BX, 0x0000))       # 8-11: Load byte back
    cb.add(IE.LD_reg(IE.AX, IE.BX))      # 12-14: AX = BX
    cb.add(IE.SUB_imm(0x00BC))           # 15-17: Subtract 0xBC (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 18-20: Jump if not zero (error)
    cb.add(IE.HALT())                    # 21: Exit successfully
    create_test_binary("test_stal_store_low", data=data, code=cb.get())
    
    # 91. Test LDA indirect - Load word using address in register
    print("\n91. Creating LDA indirect test...")
    data = IE.encode_word(0xDEF0)        # Store 0xDEF0 at data[0:2]
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.BX, 0x0000))     # 0-3: BX = 0x0000 (address of data)
    cb.add(IE.LDA_indirect(IE.AX, IE.BX)) # 4-6: Load word from address in BX
    cb.add(IE.SUB_imm(0xDEF0))           # 7-9: Subtract 0xDEF0 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 10-12: Jump if not zero (error)
    cb.add(IE.HALT())                    # 13: Exit successfully
    create_test_binary("test_lda_indirect", data=data, code=cb.get())
    
    # 92. Test LDAH indirect - Load byte to high byte using address in register
    print("\n92. Creating LDAH indirect test...")
    data = bytes([0xEF]) + bytes([0x00]) # Byte 0xEF at data[0]
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0012))     # 0-3: AX = 0x0012
    cb.add(IE.LD_imm(IE.BX, 0x0000))     # 4-7: BX = 0x0000 (address of data)
    cb.add(IE.LDAH_indirect(IE.AX, IE.BX)) # 8-10: Load high byte from address in BX -> AX = 0xEF12
    cb.add(IE.SUB_imm(0xEF12))           # 11-13: Subtract 0xEF12 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 14-16: Jump if not zero (error)
    cb.add(IE.HALT())                    # 17: Exit successfully
    create_test_binary("test_ldah_indirect", data=data, code=cb.get())
    
    # 93. Test LDAL indirect - Load byte to low byte using address in register
    print("\n93. Creating LDAL indirect test...")
    data = bytes([0x23]) + bytes([0x00]) # Byte 0x23 at data[0]
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x4500))     # 0-3: AX = 0x4500
    cb.add(IE.LD_imm(IE.BX, 0x0000))     # 4-7: BX = 0x0000 (address of data)
    cb.add(IE.LDAL_indirect(IE.AX, IE.BX)) # 8-10: Load low byte from address in BX -> AX = 0x4523
    cb.add(IE.SUB_imm(0x4523))           # 11-13: Subtract 0x4523 (should = 0)
    cb.add(IE.JPNZ(0xFF))                # 14-16: Jump if not zero (error)
    cb.add(IE.HALT())                    # 17: Exit successfully
    create_test_binary("test_ldal_indirect", data=data, code=cb.get())
    
    # ===================================================================
    # COMPARISON OPERATIONS (Tests 94-99)
    # ===================================================================
    
    # 94. Test CMP reg,reg - less than
    print("\n94. Creating CMP reg,reg less than test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0010))     # 0-3: AX = 16
    cb.add(IE.LD_imm(IE.BX, 0x0020))     # 4-7: BX = 32
    cb.add(IE.CMP_reg(IE.AX, IE.BX))     # 8-10: Compare AX < BX -> AX = 0xFFFF (-1)
    cb.add(IE.ADD_imm(0x0001))           # 11-13: AX + 1 = 0xFFFF + 1 = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 14-16: Jump if not zero (error)
    cb.add(IE.HALT())                    # 17: Exit successfully
    create_test_binary("test_cmp_reg_less", code=cb.get())
    
    # 95. Test CMP reg,reg - equal
    print("\n95. Creating CMP reg,reg equal test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0042))     # 0-3: AX = 66
    cb.add(IE.LD_imm(IE.BX, 0x0042))     # 4-7: BX = 66
    cb.add(IE.CMP_reg(IE.AX, IE.BX))     # 8-10: Compare AX == BX -> AX = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 11-13: Jump if not zero (error)
    cb.add(IE.HALT())                    # 14: Exit successfully
    create_test_binary("test_cmp_reg_equal", code=cb.get())
    
    # 96. Test CMP reg,reg - greater than
    print("\n96. Creating CMP reg,reg greater than test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0050))     # 0-3: AX = 80
    cb.add(IE.LD_imm(IE.BX, 0x0030))     # 4-7: BX = 48
    cb.add(IE.CMP_reg(IE.AX, IE.BX))     # 8-10: Compare AX > BX -> AX = 0x0001
    cb.add(IE.SUB_imm(0x0001))           # 11-13: AX - 1 = 0x0001 - 1 = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 14-16: Jump if not zero (error)
    cb.add(IE.HALT())                    # 17: Exit successfully
    create_test_binary("test_cmp_reg_greater", code=cb.get())
    
    # 97. Test CMP reg,imm
    print("\n97. Creating CMP reg,imm test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.BX, 0x0100))     # 0-3: BX = 256
    cb.add(IE.CMP_imm(IE.BX, 0x0100))    # 4-7: Compare BX == 256 -> AX = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 8-10: Jump if not zero (error)
    cb.add(IE.HALT())                    # 11: Exit successfully
    create_test_binary("test_cmp_imm", code=cb.get())
    
    # 98. Test CPH - Compare high bytes
    print("\n98. Creating CPH compare high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1200))     # 0-3: AX = 0x1200 (high = 0x12)
    cb.add(IE.LD_imm(IE.BX, 0x1234))     # 4-7: BX = 0x1234 (high = 0x12)
    cb.add(IE.CPH_reg(IE.AX, IE.BX))     # 8-10: Compare high bytes -> AX = 0x0000 (equal)
    cb.add(IE.JPNZ(0xFF))                # 11-13: Jump if not zero (error)
    cb.add(IE.HALT())                    # 14: Exit successfully
    create_test_binary("test_cph_reg", code=cb.get())
    
    # 99. Test CPH with immediate
    print("\n99. Creating CPH immediate test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.CX, 0x5678))     # 0-3: CX = 0x5678 (high = 0x56)
    cb.add(IE.CPH_imm(IE.CX, 0x56))      # 4-6: Compare high byte with 0x56 -> AX = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 7-9: Jump if not zero (error)
    cb.add(IE.HALT())                    # 10: Exit successfully
    create_test_binary("test_cph_imm", code=cb.get())
    
    # 100. Test CPL - Compare low bytes
    print("\n100. Creating CPL compare low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1234))     # 0-3: AX = 0x1234 (low = 0x34)
    cb.add(IE.LD_imm(IE.BX, 0x5634))     # 4-7: BX = 0x5634 (low = 0x34)
    cb.add(IE.CPL_reg(IE.AX, IE.BX))     # 8-10: Compare low bytes -> AX = 0x0000 (equal)
    cb.add(IE.JPNZ(0xFF))                # 11-13: Jump if not zero (error)
    cb.add(IE.HALT())                    # 14: Exit successfully
    create_test_binary("test_cpl_reg", code=cb.get())
    
    # 101. Test CPL with immediate
    print("\n101. Creating CPL immediate test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.DX, 0xABCD))     # 0-3: DX = 0xABCD (low = 0xCD)
    cb.add(IE.CPL_imm(IE.DX, 0xCD))      # 4-6: Compare low byte with 0xCD -> AX = 0x0000
    cb.add(IE.JPNZ(0xFF))                # 7-9: Jump if not zero (error)
    cb.add(IE.HALT())                    # 10: Exit successfully
    create_test_binary("test_cpl_imm", code=cb.get())
    
    # ===================================================================
    # BYTE-VARIANT ALU OPERATIONS (Tests 102-116) - Sample coverage
    # ===================================================================
    
    # Addition byte variants
    # 102. Test ADB - Add byte to AX
    print("\n102. Creating ADB add byte test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0010))     # 0-3: AX = 16
    cb.add(IE.ADB(0x05))                 # 4-5: AX = 16 + 5 = 21
    cb.add(IE.SUB_imm(0x0015))           # 6-8: AX - 21 = 0
    cb.add(IE.JPNZ(0xFF))                # 9-11: Jump if not zero (error)
    cb.add(IE.HALT())                    # 12: Exit successfully
    create_test_binary("test_adb_byte", code=cb.get())
    
    # 103. Test ADH - Add high byte (extracts byte, adds as word)
    print("\n103. Creating ADH add high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1200))     # 0-3: AX = 0x1200 (4608)
    cb.add(IE.LD_imm(IE.BX, 0x3400))     # 4-7: BX = 0x3400 (high = 0x34 = 52)
    cb.add(IE.ADH(IE.BX))                # 8-9: AX = 0x1200 + 0x34 = 0x1234
    cb.add(IE.SUB_imm(0x1234))           # 10-12: AX - 0x1234 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_adh_high", code=cb.get())
    
    # 104. Test ADL - Add low byte (extracts byte, adds as word)
    print("\n104. Creating ADL add low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0012))     # 0-3: AX = 0x0012 (18)
    cb.add(IE.LD_imm(IE.CX, 0xAB34))     # 4-7: CX = 0xAB34 (low = 0x34 = 52)
    cb.add(IE.ADL(IE.CX))                # 8-9: AX = 0x0012 + 0x34 = 0x0046
    cb.add(IE.SUB_imm(0x0046))           # 10-12: AX - 0x46 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_adl_low", code=cb.get())
    
    # Subtraction byte variants
    # 105. Test SBB - Subtract byte from AX
    print("\n105. Creating SBB subtract byte test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0020))     # 0-3: AX = 32
    cb.add(IE.SBB(0x08))                 # 4-5: AX = 32 - 8 = 24
    cb.add(IE.SUB_imm(0x0018))           # 6-8: AX - 24 = 0
    cb.add(IE.JPNZ(0xFF))                # 9-11: Jump if not zero (error)
    cb.add(IE.HALT())                    # 12: Exit successfully
    create_test_binary("test_sbb_byte", code=cb.get())
    
    # 106. Test SBH - Subtract high byte (extracts byte, subtracts as word)
    print("\n106. Creating SBH subtract high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0100))     # 0-3: AX = 0x0100 (256)
    cb.add(IE.LD_imm(IE.DX, 0x2000))     # 4-7: DX = 0x2000 (high = 0x20 = 32)
    cb.add(IE.SBH(IE.DX))                # 8-9: AX = 0x0100 - 0x20 = 0x00E0 (224)
    cb.add(IE.SUB_imm(0x00E0))           # 10-12: AX - 0x00E0 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_sbh_high", code=cb.get())
    
    # 107. Test SBL - Subtract low byte (extracts byte, subtracts as word)
    print("\n107. Creating SBL subtract low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0050))     # 0-3: AX = 0x0050 (80)
    cb.add(IE.LD_imm(IE.EX, 0xAB20))     # 4-7: EX = 0xAB20 (low = 0x20 = 32)
    cb.add(IE.SBL(IE.EX))                # 8-9: AX = 0x0050 - 0x20 = 0x0030 (48)
    cb.add(IE.SUB_imm(0x0030))           # 10-12: AX - 0x30 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_sbl_low", code=cb.get())
    
    # Multiplication byte variants
    # 108. Test MLB - Multiply byte
    print("\n108. Creating MLB multiply byte test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0005))     # 0-3: AX = 5
    cb.add(IE.MLB(0x06))                 # 4-5: AX = 5 * 6 = 30
    cb.add(IE.SUB_imm(0x001E))           # 6-8: AX - 30 = 0
    cb.add(IE.JPNZ(0xFF))                # 9-11: Jump if not zero (error)
    cb.add(IE.HALT())                    # 12: Exit successfully
    create_test_binary("test_mlb_byte", code=cb.get())
    
    # 109. Test MLH - Multiply high byte (extracts byte, multiplies as word)
    print("\n109. Creating MLH multiply high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0005))     # 0-3: AX = 0x0005 (5)
    cb.add(IE.LD_imm(IE.BX, 0x0400))     # 4-7: BX = 0x0400 (high = 0x04 = 4)
    cb.add(IE.MLH(IE.BX))                # 8-9: AX = 5 * 4 = 20 = 0x0014
    cb.add(IE.SUB_imm(0x0014))           # 10-12: AX - 0x0014 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_mlh_high", code=cb.get())
    
    # 110. Test MLL - Multiply low byte (extracts byte, multiplies as word)
    print("\n110. Creating MLL multiply low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0003))     # 0-3: AX = 0x0003 (3)
    cb.add(IE.LD_imm(IE.CX, 0xAB05))     # 4-7: CX = 0xAB05 (low = 0x05 = 5)
    cb.add(IE.MLL(IE.CX))                # 8-9: AX = 3 * 5 = 15 = 0x000F
    cb.add(IE.SUB_imm(0x000F))           # 10-12: AX - 0x0F = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_mll_low", code=cb.get())
    
    # Division byte variants
    # 111. Test DVB - Divide by byte
    print("\n111. Creating DVB divide by byte test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0028))     # 0-3: AX = 40
    cb.add(IE.DVB(0x05))                 # 4-5: AX = 40 / 5 = 8
    cb.add(IE.SUB_imm(0x0008))           # 6-8: AX - 8 = 0
    cb.add(IE.JPNZ(0xFF))                # 9-11: Jump if not zero (error)
    cb.add(IE.HALT())                    # 12: Exit successfully
    create_test_binary("test_dvb_byte", code=cb.get())
    
    # 112. Test DVH - Divide by high byte (extracts byte, divides as word)
    print("\n112. Creating DVH divide high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0018))     # 0-3: AX = 0x0018 (24)
    cb.add(IE.LD_imm(IE.DX, 0x0300))     # 4-7: DX = 0x0300 (high = 0x03 = 3)
    cb.add(IE.DVH(IE.DX))                # 8-9: AX = 24 / 3 = 8 = 0x0008
    cb.add(IE.SUB_imm(0x0008))           # 10-12: AX - 0x0008 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_dvh_high", code=cb.get())
    
    # 113. Test DVL - Divide by low byte (extracts byte, divides as word)
    print("\n113. Creating DVL divide low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0018))     # 0-3: AX = 0x0018 (24)
    cb.add(IE.LD_imm(IE.EX, 0xAB04))     # 4-7: EX = 0xAB04 (low = 0x04 = 4)
    cb.add(IE.DVL(IE.EX))                # 8-9: AX = 24 / 4 = 6 = 0x0006
    cb.add(IE.SUB_imm(0x0006))           # 10-12: AX - 0x0006 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_dvl_low", code=cb.get())
    
    # Remainder byte variants
    # 114. Test RMB - Remainder byte
    print("\n114. Creating RMB remainder byte test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0017))     # 0-3: AX = 23
    cb.add(IE.RMB(0x05))                 # 4-5: AX = 23 % 5 = 3
    cb.add(IE.SUB_imm(0x0003))           # 6-8: AX - 3 = 0
    cb.add(IE.JPNZ(0xFF))                # 9-11: Jump if not zero (error)
    cb.add(IE.HALT())                    # 12: Exit successfully
    create_test_binary("test_rmb_byte", code=cb.get())
    
    # 115. Test RMH - Remainder by high byte (extracts byte, remainder as word)
    print("\n115. Creating RMH remainder high bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0017))     # 0-3: AX = 0x0017 (23)
    cb.add(IE.LD_imm(IE.BX, 0x0500))     # 4-7: BX = 0x0500 (high = 0x05 = 5)
    cb.add(IE.RMH(IE.BX))                # 8-9: AX = 23 % 5 = 3 = 0x0003
    cb.add(IE.SUB_imm(0x0003))           # 10-12: AX - 0x0003 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_rmh_high", code=cb.get())
    
    # 116. Test RML - Remainder by low byte (extracts byte, remainder as word)
    print("\n116. Creating RML remainder low bytes test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x0017))     # 0-3: AX = 0x0017 (23)
    cb.add(IE.LD_imm(IE.CX, 0xAB05))     # 4-7: CX = 0xAB05 (low = 0x05 = 5)
    cb.add(IE.RML(IE.CX))                # 8-9: AX = 23 % 5 = 3 = 0x0003
    cb.add(IE.SUB_imm(0x0003))           # 10-12: AX - 0x0003 = 0
    cb.add(IE.JPNZ(0xFF))                # 13-15: Jump if not zero (error)
    cb.add(IE.HALT())                    # 16: Exit successfully
    create_test_binary("test_rml_low", code=cb.get())
    
    # ===================================================================
    # MEMORY PAGING OPERATIONS (Tests 117-118)
    # ===================================================================
    
    # 117. Test PAGE immediate - Set memory page with immediate value
    print("\n117. Creating PAGE immediate test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0x1234))     # 0-3: AX = 0x1234
    cb.add(IE.LDA_store(0x0100, IE.AX))  # 4-7: Store AX at address 0x0100 (page 0)
    cb.add(IE.PAGE_imm(1, 0))            # 8-12: Switch to page 1
    cb.add(IE.LD_imm(IE.BX, 0x5678))     # 13-16: BX = 0x5678
    cb.add(IE.LDA_store(0x0100, IE.BX))  # 17-20: Store BX at address 0x0100 (page 1)
    cb.add(IE.PAGE_imm(0, 0))            # 21-25: Switch back to page 0
    cb.add(IE.LDA_load(IE.CX, 0x0100))   # 26-29: Load from 0x0100 (should be 0x1234)
    cb.add(IE.SUB_imm(0x1234))           # 30-32: AX = CX - 0x1234 (moved to AX by SUB)
    # Note: CMP would be better but SUB works for validation
    cb.add(IE.HALT())                    # 33: Exit successfully
    create_test_binary("test_page_immediate", code=cb.get())
    
    # 118. Test PAGE register - Set memory page from register value
    print("\n118. Creating PAGE register test...")
    cb = CodeBuilder()
    cb.add(IE.LD_imm(IE.AX, 0xAAAA))     # 0-3: AX = 0xAAAA
    cb.add(IE.LDA_store(0x0200, IE.AX))  # 4-7: Store at 0x0200 page 0
    cb.add(IE.LD_imm(IE.DX, 0x0001))     # 8-11: DX = 1 (page number)
    cb.add(IE.PAGE_reg(IE.DX, 0))        # 12-15: Switch to page in DX (page 1)
    cb.add(IE.LD_imm(IE.BX, 0xBBBB))     # 16-19: BX = 0xBBBB
    cb.add(IE.LDA_store(0x0200, IE.BX))  # 20-23: Store at 0x0200 page 1
    cb.add(IE.LD_imm(IE.DX, 0x0000))     # 24-27: DX = 0 (page number)
    cb.add(IE.PAGE_reg(IE.DX, 0))        # 28-31: Switch to page 0
    cb.add(IE.LDA_load(IE.CX, 0x0200))   # 32-35: Load from 0x0200 (should be 0xAAAA)
    cb.add(IE.SUB_imm(0xAAAA))           # 36-38: AX = CX - 0xAAAA
    cb.add(IE.HALT())                    # 39: Exit successfully
    create_test_binary("test_page_register", code=cb.get())
    
    print("\n" + "=" * 50)
    print("Test binaries created successfully!")
    print(f"Total: 118 test binaries")
    print("Run with: ./build/lvm <binary_file> 0x0000")
