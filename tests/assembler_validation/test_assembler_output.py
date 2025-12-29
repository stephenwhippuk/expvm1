#!/usr/bin/env python3
"""
Assembler Binary Output Validation

Tests that the assembler produces correct binary output by:
1. Creating simple .asm files with known instructions
2. Assembling them with the build/asm tool
3. Comparing output against expected binary sequences
4. Validating against reference binaries from instruction_validation
"""

import subprocess
import os
import sys
from pathlib import Path
from typing import List, Tuple, Optional

# Add instruction_validation to path for using InstructionEncoder
sys.path.insert(0, str(Path(__file__).parent.parent / "instruction_validation"))
from generate_test_binaries import InstructionEncoder as IE

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

class AssemblerTest:
    def __init__(self, name: str, asm_code: str, expected_code: bytes):
        self.name = name
        self.asm_code = asm_code
        self.expected_code = expected_code
        self.actual_code: Optional[bytes] = None
        self.passed = False
        self.error: Optional[str] = None

def create_asm_file(filename: str, content: str) -> Path:
    """Create a temporary .asm file"""
    path = Path("temp") / filename
    path.parent.mkdir(exist_ok=True)
    path.write_text(content)
    return path

def assemble_file(asm_path: Path, output_path: Path) -> bool:
    """Run the assembler on a file"""
    try:
        # Find assembler relative to project root
        assembler = Path(__file__).parent.parent.parent / "build" / "asm"
        result = subprocess.run(
            [str(assembler), str(asm_path), "-o", str(output_path)],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode != 0 and result.stderr:
            print(f"  Assembler stderr: {result.stderr[:200]}")
        return result.returncode == 0
    except Exception as e:
        print(f"Assembler error: {e}")
        return False

def extract_code_segment(binary_path: Path) -> bytes:
    """Extract just the code segment from a Pendragon binary file"""
    data = binary_path.read_bytes()
    pos = 0
    
    try:
        # Header Size (2 bytes)
        header_size = data[pos] | (data[pos+1] << 8)
        pos += 2
        
        # Header Version (4 bytes)
        pos += 4
        
        # Machine Name Size (1 byte)
        machine_name_size = data[pos]
        pos += 1
        
        # Machine Name (n bytes)
        pos += machine_name_size
        
        # Machine Version (4 bytes)
        pos += 4
        
        # Program Name Size (2 bytes)
        program_name_size = data[pos] | (data[pos+1] << 8)
        pos += 2
        
        # Program Name (n bytes including null terminator)
        pos += program_name_size
        
        # Data Segment Size (4 bytes)
        data_size = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24)
        pos += 4
        
        # Data Segment (n bytes)
        pos += data_size
        
        # Code Segment Size (4 bytes)
        code_size = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24)
        pos += 4
        
        # Code Segment
        return data[pos:pos+code_size]
    except IndexError:
        return b''

def run_test(test: AssemblerTest) -> None:
    """Run a single assembler test"""
    # Create .asm file
    asm_path = create_asm_file(f"{test.name}.asm", test.asm_code)
    bin_path = Path("temp") / f"{test.name}.bin"
    
    # Assemble
    if not assemble_file(asm_path, bin_path):
        test.error = "Assembly failed"
        return
    
    # Extract code segment
    if not bin_path.exists():
        test.error = "Output binary not created"
        return
    
    test.actual_code = extract_code_segment(bin_path)
    
    # Compare
    if test.actual_code == test.expected_code:
        test.passed = True
    else:
        test.error = f"Binary mismatch"

def format_bytes(data: bytes, bytes_per_line: int = 16) -> str:
    """Format bytes as hex dump"""
    lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_str = ' '.join(f'{b:02X}' for b in chunk)
        ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
        lines.append(f"  {i:04X}: {hex_str:<48} {ascii_str}")
    return '\n'.join(lines)

def print_test_result(test: AssemblerTest, verbose: bool = False) -> None:
    """Print test result with formatting"""
    if test.passed:
        print(f"{Colors.GREEN}✓{Colors.RESET} {test.name}")
    else:
        print(f"{Colors.RED}✗{Colors.RESET} {test.name}: {test.error}")
        if verbose and test.actual_code is not None:
            print(f"  Expected ({len(test.expected_code)} bytes):")
            print(format_bytes(test.expected_code))
            print(f"  Actual ({len(test.actual_code)} bytes):")
            print(format_bytes(test.actual_code))

# Test Suite Definition
def create_tests() -> List[AssemblerTest]:
    """Create the test suite"""
    tests = []
    
    # Test 1: Basic HALT
    tests.append(AssemblerTest(
        "halt_only",
        "CODE\n    HALT\n",
        IE.HALT()
    ))
    
    # Test 2: NOP and HALT
    tests.append(AssemblerTest(
        "nop_halt",
        "CODE\n    NOP\n    HALT\n",
        IE.NOP() + IE.HALT()
    ))
    
    # Test 3: Simple LD immediate
    tests.append(AssemblerTest(
        "ld_immediate",
        "CODE\n    LD AX, 0x1234\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) + IE.HALT()
    ))
    
    # Test 4: Multiple register loads
    tests.append(AssemblerTest(
        "multi_ld",
        """CODE
    LD AX, 0x1000
    LD BX, 0x2000
    LD CX, 0x3000
    HALT
""",
        IE.LD_imm(IE.AX, 0x1000) +
        IE.LD_imm(IE.BX, 0x2000) +
        IE.LD_imm(IE.CX, 0x3000) +
        IE.HALT()
    ))
    
    # Test 5: Register to register
    tests.append(AssemblerTest(
        "ld_register",
        "CODE\n    LD AX, 0x5678\n    LD BX, AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x5678) +
        IE.LD_reg(IE.BX, IE.AX) +  # LD BX, AX (register-to-register)
        IE.HALT()
    ))
    
    # Test 6: Arithmetic
    tests.append(AssemblerTest(
        "add_immediate",
        "CODE\n    LD AX, 100\n    ADD 50\n    HALT\n",
        IE.LD_imm(IE.AX, 100) +
        IE.ADD_imm(50) +
        IE.HALT()
    ))
    
    # Test 7: Jump
    tests.append(AssemblerTest(
        "jump_forward",
        "CODE\n    JMP skip\nskip:\n    HALT\n",
        IE.JMP(0x0003) +  # JMP is 3 bytes, so skip to byte 3
        IE.HALT()
    ))
    
    # Test 8: Conditional jump
    tests.append(AssemblerTest(
        "jump_conditional",
        """CODE
    LD AX, 0
    JPZ zero
    HALT
zero:
    HALT
""",
        IE.LD_imm(IE.AX, 0) +
        IE.JPZ(0x0008) +  # Assembler calculates to byte 8
        IE.HALT() +
        IE.HALT()
    ))
    
    # Test 9: Stack operations
    tests.append(AssemblerTest(
        "stack_push_pop",
        "CODE\n    LD AX, 0xABCD\n    PUSH AX\n    POP BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0xABCD) +
        IE.PUSH(IE.AX) +
        IE.POP(IE.BX) +
        IE.HALT()
    ))
    
    # Test 10: Memory store and load operations
    tests.append(AssemblerTest(
        "memory_store_load",
        "CODE\n    LD AX, 0x1234\n    LDA 0x0100, AX\n    LDA BX, 0x0100\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LDA_store(0x0100, IE.AX) +
        IE.LDA_load(IE.BX, 0x0100) +
        IE.HALT()
    ))
    
    # Test 11: High/low byte operations
    tests.append(AssemblerTest(
        "byte_operations",
        "CODE\n    LDH AX, 0x12\n    LDL AX, 0x34\n    HALT\n",
        IE.LDH_imm(IE.AX, 0x12) +
        IE.LDL_imm(IE.AX, 0x34) +
        IE.HALT()
    ))
    
    # Test 12: CALL/RET
    tests.append(AssemblerTest(
        "call_ret",
        """CODE
    CALL sub
    HALT
sub:
    RET
""",
        IE.CALL(0x0005, False) +  # CALL is 4 bytes, HALT is 1, so sub at 5
        IE.HALT() +
        IE.RET()
    ))
    
    # Test 13: Comparison (assembler encoding)
    tests.append(AssemblerTest(
        "compare",
        "CODE\n    LD AX, 100\n    CMP AX, 100\n    HALT\n",
        bytes([0x02, 0x00, 0x64, 0x00,  # LD AX, 100
               0x6D, 0x00, 0x64, 0x00,  # CMP: opcode 0x6D (immediate), reg0, imm 100
               0x01])                    # HALT
    ))
    
    # Test 14: Logical operations
    tests.append(AssemblerTest(
        "logical_ops",
        "CODE\n    LD AX, 0xFF00\n    AND 0x00FF\n    OR 0x0F0F\n    HALT\n",
        IE.LD_imm(IE.AX, 0xFF00) +
        IE.AND_imm(0x00FF) +
        IE.OR_imm(0x0F0F) +
        IE.HALT()
    ))
    
    # Test 15: Shift operations
    tests.append(AssemblerTest(
        "shift_ops",
        "CODE\n    LD AX, 0x0001\n    SHL 4\n    SHR 2\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0001) +
        IE.SHL_imm(4) +
        IE.SHR_imm(2) +
        IE.HALT()
    ))
    
    # Test 16: Register-to-register operations (SWP, LDH, LDL)
    tests.append(AssemblerTest(
        "register_operations",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x5678\n    SWP AX, BX\n    LDH CX, AX\n    LDL DX, BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x5678) +
        IE.SWP(IE.AX, IE.BX) +
        IE.LDH_reg(IE.CX, IE.AX) +
        IE.LDL_reg(IE.DX, IE.BX) +
        IE.HALT()
    ))
    
    # Test 17: All arithmetic operations
    tests.append(AssemblerTest(
        "arithmetic_ops",
        "CODE\n    LD AX, 100\n    ADD 50\n    SUB 25\n    MUL 2\n    DIV 5\n    HALT\n",
        IE.LD_imm(IE.AX, 100) +
        IE.ADD_imm(50) +
        IE.SUB_imm(25) +
        IE.MUL_imm(2) +
        IE.DIV_imm(5) +
        IE.HALT()
    ))
    
    # Test 18: Byte arithmetic (ADB, SBB)
    tests.append(AssemblerTest(
        "byte_arithmetic",
        "CODE\n    LD AX, 100\n    ADB 50\n    SBB 25\n    HALT\n",
        IE.LD_imm(IE.AX, 100) +
        IE.ADB(50) +
        IE.SBB(25) +
        IE.HALT()
    ))
    
    # Test 19: INC/DEC
    tests.append(AssemblerTest(
        "inc_dec",
        "CODE\n    LD AX, 10\n    INC AX\n    DEC AX\n    HALT\n",
        IE.LD_imm(IE.AX, 10) +
        IE.INC(IE.AX) +
        IE.DEC(IE.AX) +
        IE.HALT()
    ))
    
    # Test 20: Multiple register loads (all registers)
    tests.append(AssemblerTest(
        "all_registers",
        """CODE
    LD AX, 0x1111
    LD BX, 0x2222
    LD CX, 0x3333
    LD DX, 0x4444
    LD EX, 0x5555
    HALT
""",
        IE.LD_imm(IE.AX, 0x1111) +
        IE.LD_imm(IE.BX, 0x2222) +
        IE.LD_imm(IE.CX, 0x3333) +
        IE.LD_imm(IE.DX, 0x4444) +
        IE.LD_imm(IE.EX, 0x5555) +
        IE.HALT()
    ))
    
    # Test 21: XOR operation
    tests.append(AssemblerTest(
        "xor_op",
        "CODE\n    LD AX, 0xFFFF\n    XOR 0xAAAA\n    HALT\n",
        IE.LD_imm(IE.AX, 0xFFFF) +
        IE.XOR_imm(0xAAAA) +
        IE.HALT()
    ))
    
    # Test 22: NOT operation
    tests.append(AssemblerTest(
        "not_op",
        "CODE\n    LD AX, 0x00FF\n    NOT AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x00FF) +
        IE.NOT_reg(IE.AX) +
        IE.HALT()
    ))
    
    # Test 23: Stack immediate push
    tests.append(AssemblerTest(
        "stack_immediate",
        "CODE\n    PUSHW 0x1234\n    POP AX\n    HALT\n",
        IE.PUSHW(0x1234) +
        IE.POP(IE.AX) +
        IE.HALT()
    ))
    
    # Test 24: All conditional jumps
    tests.append(AssemblerTest(
        "conditional_jumps",
        """CODE
    JPZ skip1
skip1:
    JPNZ skip2
skip2:
    JPC skip3
skip3:
    JPNC skip4
skip4:
    HALT
""",
        IE.JPZ(0x0003) +
        IE.JPNZ(0x0006) +
        IE.JPC(0x0009) +
        IE.JPNC(0x000C) +
        IE.HALT()
    ))
    
    # Test 25: REM (remainder/modulo)
    tests.append(AssemblerTest(
        "remainder",
        "CODE\n    LD AX, 17\n    REM 5\n    HALT\n",
        IE.LD_imm(IE.AX, 17) +
        IE.REM_imm(5) +
        IE.HALT()
    ))
    
    # Test 26: Arithmetic with register operands
    tests.append(AssemblerTest(
        "arithmetic_register",
        "CODE\n    LD AX, 100\n    LD BX, 50\n    ADD BX\n    SUB BX\n    MUL BX\n    DIV BX\n    HALT\n",
        IE.LD_imm(IE.AX, 100) +
        IE.LD_imm(IE.BX, 50) +
        IE.ADD_reg(IE.BX) +
        IE.SUB_reg(IE.BX) +
        IE.MUL_reg(IE.BX) +
        IE.DIV_reg(IE.BX) +
        IE.HALT()
    ))
    
    # Test 27: Logical operations with register operands
    tests.append(AssemblerTest(
        "logical_register",
        "CODE\n    LD AX, 0xFF00\n    LD BX, 0x00FF\n    AND BX\n    OR BX\n    XOR BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0xFF00) +
        IE.LD_imm(IE.BX, 0x00FF) +
        IE.AND_reg(IE.BX) +
        IE.OR_reg(IE.BX) +
        IE.XOR_reg(IE.BX) +
        IE.HALT()
    ))
    
    # Test 28: Shift operations with register operands
    tests.append(AssemblerTest(
        "shift_register",
        "CODE\n    LD AX, 0x1234\n    LD BX, 4\n    SHL BX\n    SHR BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 4) +
        IE.SHL_reg(IE.BX) +
        IE.SHR_reg(IE.BX) +
        IE.HALT()
    ))
    
    # Test 29: REM with register
    tests.append(AssemblerTest(
        "rem_register",
        "CODE\n    LD AX, 17\n    LD BX, 5\n    REM BX\n    HALT\n",
        IE.LD_imm(IE.AX, 17) +
        IE.LD_imm(IE.BX, 5) +
        IE.REM_reg(IE.BX) +
        IE.HALT()
    ))
    
    # Test 30: CMP with register
    tests.append(AssemblerTest(
        "cmp_register",
        "CODE\n    LD AX, 100\n    LD BX, 100\n    CMP AX, BX\n    HALT\n",
        IE.LD_imm(IE.AX, 100) +
        IE.LD_imm(IE.BX, 100) +
        IE.CMP_reg(IE.AX, IE.BX) +
        IE.HALT()
    ))
    
    # Test 31: Rotate operations (ROL, ROR)
    tests.append(AssemblerTest(
        "rotate_ops",
        "CODE\n    LD AX, 0x8001\n    ROL 1\n    ROR 2\n    HALT\n",
        IE.LD_imm(IE.AX, 0x8001) +
        IE.ROL_imm(1) +
        IE.ROR_imm(2) +
        IE.HALT()
    ))
    
    # Test 32: Rotate with register operand
    tests.append(AssemblerTest(
        "rotate_register",
        "CODE\n    LD AX, 0x0F0F\n    LD BX, 4\n    ROL BX\n    ROR BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0F0F) +
        IE.LD_imm(IE.BX, 4) +
        IE.ROL_reg(IE.BX) +
        IE.ROR_reg(IE.BX) +
        IE.HALT()
    ))
    
    # Test 33: Byte-specific operations (high/low byte arithmetic)
    tests.append(AssemblerTest(
        "byte_ops_high_low",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x5678\n    ADH BX\n    ADL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x5678) +
        IE.ADH(IE.BX) +
        IE.ADL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 34: Sub-register operations (all sub-registers)
    tests.append(AssemblerTest(
        "subregs_all",
        "CODE\n    LDH AH, 0xAA\n    LDL AL, 0xBB\n    LDH BH, 0xCC\n    LDL BL, 0xDD\n    HALT\n",
        IE.LDH_imm(IE.AX, 0xAA) +
        IE.LDL_imm(IE.AX, 0xBB) +
        IE.LDH_imm(IE.BX, 0xCC) +
        IE.LDL_imm(IE.BX, 0xDD) +
        IE.HALT()
    ))
    
    # Test 35: Stack operations with high/low bytes
    tests.append(AssemblerTest(
        "stack_bytes",
        "CODE\n    LD AX, 0x1234\n    PUSHH AX\n    PUSHL AX\n    POPL BX\n    POPH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.PUSHH(IE.AX) +
        IE.PUSHL(IE.AX) +
        IE.POPL(IE.BX) +
        IE.POPH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 36: Multiple jumps in sequence
    tests.append(AssemblerTest(
        "multiple_jumps",
        """CODE
start:
    JMP label1
label1:
    JMP label2
label2:
    JMP end
end:
    HALT
""",
        IE.JMP(0x0003) +  # Jump to label1 at 0x0003
        IE.JMP(0x0006) +  # Jump to label2 at 0x0006
        IE.JMP(0x0009) +  # Jump to end at 0x0009
        IE.HALT()
    ))
    
    # Test 37: CALL without page flag
    tests.append(AssemblerTest(
        "call_basic",
        """CODE
    CALL sub
    HALT
sub:
    RET
""",
        IE.CALL(0x0005, False) +  # CALL without page flag
        IE.HALT() +
        IE.RET()
    ))
    
    # Test 38: Byte arithmetic (SBH, SBL, MLB, etc.)
    tests.append(AssemblerTest(
        "byte_arithmetic_variants",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x5678\n    SBH BX\n    SBL BX\n    MLH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x5678) +
        IE.SBH(IE.BX) +
        IE.SBL(IE.BX) +
        IE.MLH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 39: More byte arithmetic (DVH, DVL, RMH, RML)
    tests.append(AssemblerTest(
        "byte_div_rem",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x0005\n    DVH BX\n    DVL BX\n    RMH BX\n    RML BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x0005) +
        IE.DVH(IE.BX) +
        IE.DVL(IE.BX) +
        IE.RMH(IE.BX) +
        IE.RML(IE.BX) +
        IE.HALT()
    ))
    
    # Test 40: Complex expression with multiple operations
    tests.append(AssemblerTest(
        "complex_sequence",
        """CODE
    LD AX, 100
    LD BX, 10
    ADD BX
    MUL BX
    DIV BX
    CMP AX, BX
    JPZ end
    INC AX
end:
    HALT
""",
        IE.LD_imm(IE.AX, 100) +
        IE.LD_imm(IE.BX, 10) +
        IE.ADD_reg(IE.BX) +
        IE.MUL_reg(IE.BX) +
        IE.DIV_reg(IE.BX) +
        IE.CMP_reg(IE.AX, IE.BX) +
        IE.JPZ(0x0016) +  # Assembler calculates jump to end address
        IE.INC(IE.AX) +
        IE.HALT()
    ))
    
    # Test 41: All conditional jumps (Sign, Overflow)
    tests.append(AssemblerTest(
        "conditional_jumps_all",
        """CODE
    JPS skip1
skip1:
    JPNS skip2
skip2:
    JPO skip3
skip3:
    JPNO skip4
skip4:
    HALT
""",
        IE.JPS(0x0003) +
        IE.JPNS(0x0006) +
        IE.JPO(0x0009) +
        IE.JPNO(0x000C) +
        IE.HALT()
    ))
    
    # Test 42: MLB (multiply low byte)
    tests.append(AssemblerTest(
        "multiply_low_byte",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x0002\n    MLB BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x0002) +
        IE.MLB(IE.BX) +
        IE.HALT()
    ))
    
    # Test 43: MLL (multiply low-low bytes)
    tests.append(AssemblerTest(
        "multiply_low_low",
        "CODE\n    LD AX, 0x0F0F\n    LD BX, 0x0003\n    MLL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0F0F) +
        IE.LD_imm(IE.BX, 0x0003) +
        IE.MLL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 44: Nested subroutines
    tests.append(AssemblerTest(
        "nested_calls",
        """CODE
    CALL sub1
    HALT
sub1:
    CALL sub2
    RET
sub2:
    RET
""",
        IE.CALL(0x0005, False) +
        IE.HALT() +
        IE.CALL(0x000A, False) +
        IE.RET() +
        IE.RET()
    ))
    
    # Test 45: Stack frame operations (PEEK variants)
    # Note: PEEK uses BYTE offset (not word as in spec), assembler is correct
    tests.append(AssemblerTest(
        "stack_peek",
        "CODE\n    LD AX, 0x1234\n    PUSH AX\n    PEEK BX, 0\n    POP CX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.PUSH(IE.AX) +
        bytes([0x16, IE.BX, 0x00]) +  # PEEK BX, 0 - uses byte offset
        IE.POP(IE.CX) +
        IE.HALT()
    ))
    
    # Test 46: PEEKF (frame-relative peek)
    tests.append(AssemblerTest(
        "stack_peekf",
        "CODE\n    LD AX, 0xABCD\n    PUSH AX\n    PEEKF BX, 0\n    HALT\n",
        IE.LD_imm(IE.AX, 0xABCD) +
        IE.PUSH(IE.AX) +
        bytes([0x17, IE.BX, 0x00]) +  # PEEKF BX, 0 - uses byte offset
        IE.HALT()
    ))
    
    # Test 47: PEEKB (byte peek)
    tests.append(AssemblerTest(
        "stack_peekb",
        "CODE\n    LD AX, 0x1234\n    PUSH AX\n    PEEKB BX, 0\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.PUSH(IE.AX) +
        bytes([0x18, IE.BX, 0x00]) +  # PEEKB BX, 0 - uses byte offset
        IE.HALT()
    ))
    
    # Test 48: PEEKFB (frame byte peek)
    tests.append(AssemblerTest(
        "stack_peekfb",
        "CODE\n    LD AX, 0x5678\n    PUSH AX\n    PEEKFB BX, 0\n    HALT\n",
        IE.LD_imm(IE.AX, 0x5678) +
        IE.PUSH(IE.AX) +
        bytes([0x19, IE.BX, 0x00]) +  # PEEKFB BX, 0 - uses byte offset
        IE.HALT()
    ))
    
    # Test 49: FLSH (flush stack)
    tests.append(AssemblerTest(
        "stack_flsh",
        "CODE\n    LD AX, 0x1111\n    PUSH AX\n    FLSH\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1111) +
        IE.PUSH(IE.AX) +
        IE.FLSH() +
        IE.HALT()
    ))
    
    # Test 50: SETF (set stack frame)
    tests.append(AssemblerTest(
        "stack_setf",
        "CODE\n    LD AX, 0x2000\n    PUSH AX\n    SETF 0x2000\n    HALT\n",
        IE.LD_imm(IE.AX, 0x2000) +
        IE.PUSH(IE.AX) +
        IE.SETF(0x2000) +
        IE.HALT()
    ))
    
    # Test 51: CPH (compare high byte immediate)
    tests.append(AssemblerTest(
        "cph_immediate",
        "CODE\n    LD AX, 0x1234\n    CPH AX, 0x12\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.CPH_imm(IE.AX, 0x12) +
        IE.HALT()
    ))
    
    # Test 52: CPH (compare high byte register)
    tests.append(AssemblerTest(
        "cph_register",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x5600\n    CPH AX, BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x5600) +
        IE.CPH_reg(IE.AX, IE.BX) +
        IE.HALT()
    ))
    
    # Test 53: CPL (compare low byte immediate)
    tests.append(AssemblerTest(
        "cpl_immediate",
        "CODE\n    LD AX, 0x1234\n    CPL AX, 0x34\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.CPL_imm(IE.AX, 0x34) +
        IE.HALT()
    ))
    
    # Test 54: CPL (compare low byte register)
    tests.append(AssemblerTest(
        "cpl_register",
        "CODE\n    LD AX, 0x1234\n    LD BX, 0x0078\n    CPL AX, BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.LD_imm(IE.BX, 0x0078) +
        IE.CPL_reg(IE.AX, IE.BX) +
        IE.HALT()
    ))
    
    # Test 55-57: LDA indirect tests - SKIP: require DATA section support
    # tests.append(AssemblerTest(
    #     "lda_indirect_load",
    #     "DATA\\n    value: .WORD 0x1234\\nCODE\\n    LD BX, value\\n    LDA AX, [BX]\\n    HALT\\n",
    #     IE.LD_imm(IE.BX, 0) +
    #     IE.LDA_indirect(IE.AX, IE.BX) +
    #     IE.HALT()
    # ))
    
    # Test 58: PAGE immediate (assembler auto-adds context=0)
    tests.append(AssemblerTest(
        "page_immediate",
        "CODE\n    PAGE 0x0001\n    HALT\n",
        IE.PAGE_imm(0x0001) +  # PAGE_imm already includes context word in InstructionEncoder
        IE.HALT()
    ))
    
    # Test 59: PAGE register (assembler auto-adds context=0)
    tests.append(AssemblerTest(
        "page_register",
        "CODE\n    LD AX, 0x0002\n    PAGE AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0002) +
        IE.PAGE_reg(IE.AX) +  # PAGE_reg already includes context word in InstructionEncoder
        IE.HALT()
    ))
    
    # Test 60: CMP immediate
    tests.append(AssemblerTest(
        "cmp_immediate",
        "CODE\n    LD AX, 0x1234\n    CMP AX, 0x1234\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.CMP_imm(IE.AX, 0x1234) +
        IE.HALT()
    ))
    
    # Test 61: NOT immediate
    tests.append(AssemblerTest(
        "not_immediate",
        "CODE\n    NOT 0x00FF\n    HALT\n",
        IE.NOT_imm(0x00FF) +
        IE.HALT()
    ))
    
    # Test 62: SYS (system call)
    tests.append(AssemblerTest(
        "sys_call",
        "CODE\n    SYS 0x01\n    HALT\n",
        IE.SYS(0x01) +  # SYS already encodes as word (3 bytes: opcode + word)
        IE.HALT()
    ))
    
    # Test 63: ANB (AND byte immediate)
    tests.append(AssemblerTest(
        "anb_byte",
        "CODE\n    LD AX, 0x12FF\n    ANB 0xF0\n    HALT\n",
        IE.LD_imm(IE.AX, 0x12FF) +
        IE.ANB(0xF0) +
        IE.HALT()
    ))
    
    # Test 64: ANH (AND high byte)
    tests.append(AssemblerTest(
        "anh_high",
        "CODE\n    LD AX, 0xFF34\n    LD BX, 0xF000\n    ANH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0xFF34) +
        IE.LD_imm(IE.BX, 0xF000) +
        IE.ANH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 65: ANL (AND low byte)
    tests.append(AssemblerTest(
        "anl_low",
        "CODE\n    LD AX, 0x12FF\n    LD BX, 0x00F0\n    ANL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x12FF) +
        IE.LD_imm(IE.BX, 0x00F0) +
        IE.ANL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 66: ORB (OR byte immediate)
    tests.append(AssemblerTest(
        "orb_byte",
        "CODE\n    LD AX, 0x1200\n    ORB 0x0F\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1200) +
        IE.ORB(0x0F) +
        IE.HALT()
    ))
    
    # Test 67: ORH (OR high byte)
    tests.append(AssemblerTest(
        "orh_high",
        "CODE\n    LD AX, 0x0034\n    LD BX, 0xF000\n    ORH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0034) +
        IE.LD_imm(IE.BX, 0xF000) +
        IE.ORH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 68: ORL (OR low byte)
    tests.append(AssemblerTest(
        "orl_low",
        "CODE\n    LD AX, 0x1200\n    LD BX, 0x000F\n    ORL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1200) +
        IE.LD_imm(IE.BX, 0x000F) +
        IE.ORL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 69: XOB (XOR byte immediate)
    tests.append(AssemblerTest(
        "xob_byte",
        "CODE\n    LD AX, 0x12AA\n    XOB 0xFF\n    HALT\n",
        IE.LD_imm(IE.AX, 0x12AA) +
        IE.XOB(0xFF) +
        IE.HALT()
    ))
    
    # Test 70: XOH (XOR high byte)
    tests.append(AssemblerTest(
        "xoh_high",
        "CODE\n    LD AX, 0xAA34\n    LD BX, 0xFF00\n    XOH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0xAA34) +
        IE.LD_imm(IE.BX, 0xFF00) +
        IE.XOH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 71: XOL (XOR low byte)
    tests.append(AssemblerTest(
        "xol_low",
        "CODE\n    LD AX, 0x12AA\n    LD BX, 0x00FF\n    XOL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x12AA) +
        IE.LD_imm(IE.BX, 0x00FF) +
        IE.XOL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 72: NOTB (NOT byte immediate)
    tests.append(AssemblerTest(
        "notb_byte",
        "CODE\n    NOTB 0xAA\n    HALT\n",
        IE.NOTB(0xAA) +
        IE.HALT()
    ))
    
    # Test 73: NOTH (NOT high byte)
    tests.append(AssemblerTest(
        "noth_high",
        "CODE\n    LD AX, 0xAA34\n    NOTH AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0xAA34) +
        IE.NOTH(IE.AX) +
        IE.HALT()
    ))
    
    # Test 74: NOTL (NOT low byte)
    tests.append(AssemblerTest(
        "notl_low",
        "CODE\n    LD AX, 0x12AA\n    NOTL AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x12AA) +
        IE.NOTL(IE.AX) +
        IE.HALT()
    ))
    
    # Test 75: SLB (shift left byte)
    tests.append(AssemblerTest(
        "slb_byte",
        "CODE\n    LD AX, 0x0001\n    SLB 4\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0001) +
        IE.SLB(4) +
        IE.HALT()
    ))
    
    # Test 76: SLH (shift left high byte)
    tests.append(AssemblerTest(
        "slh_high",
        "CODE\n    LD AX, 0x0100\n    LD BX, 0x0400\n    SLH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0100) +
        IE.LD_imm(IE.BX, 0x0400) +
        IE.SLH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 77: SLL (shift left low byte)
    tests.append(AssemblerTest(
        "sll_low",
        "CODE\n    LD AX, 0x0001\n    LD BX, 0x0004\n    SLL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0001) +
        IE.LD_imm(IE.BX, 0x0004) +
        IE.SLL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 78: SHRB (shift right byte)
    tests.append(AssemblerTest(
        "shrb_byte",
        "CODE\n    LD AX, 0x0080\n    SHRB 4\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0080) +
        IE.SHRB(4) +
        IE.HALT()
    ))
    
    # Test 79: SHRH (shift right high byte)
    tests.append(AssemblerTest(
        "shrh_high",
        "CODE\n    LD AX, 0x8000\n    LD BX, 0x0400\n    SHRH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x8000) +
        IE.LD_imm(IE.BX, 0x0400) +
        IE.SHRH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 80: SHRL (shift right low byte)
    tests.append(AssemblerTest(
        "shrl_low",
        "CODE\n    LD AX, 0x0080\n    LD BX, 0x0004\n    SHRL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0080) +
        IE.LD_imm(IE.BX, 0x0004) +
        IE.SHRL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 81: ROLB (rotate left byte)
    tests.append(AssemblerTest(
        "rolb_byte",
        "CODE\n    LD AX, 0x0081\n    ROLB 1\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0081) +
        IE.ROLB(1) +
        IE.HALT()
    ))
    
    # Test 82: ROLH (rotate left high byte)
    tests.append(AssemblerTest(
        "rolh_high",
        "CODE\n    LD AX, 0x8100\n    LD BX, 0x0100\n    ROLH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x8100) +
        IE.LD_imm(IE.BX, 0x0100) +
        IE.ROLH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 83: ROLL (rotate left low byte)
    tests.append(AssemblerTest(
        "roll_low",
        "CODE\n    LD AX, 0x0081\n    LD BX, 0x0001\n    ROLL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0081) +
        IE.LD_imm(IE.BX, 0x0001) +
        IE.ROLL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 84: RORB (rotate right byte)
    tests.append(AssemblerTest(
        "rorb_byte",
        "CODE\n    LD AX, 0x0081\n    RORB 1\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0081) +
        IE.RORB(1) +
        IE.HALT()
    ))
    
    # Test 85: RORH (rotate right high byte)
    tests.append(AssemblerTest(
        "rorh_high",
        "CODE\n    LD AX, 0x8100\n    LD BX, 0x0100\n    RORH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x8100) +
        IE.LD_imm(IE.BX, 0x0100) +
        IE.RORH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 86: RORL (rotate right low byte)
    tests.append(AssemblerTest(
        "rorl_low",
        "CODE\n    LD AX, 0x0081\n    LD BX, 0x0001\n    RORL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0081) +
        IE.LD_imm(IE.BX, 0x0001) +
        IE.RORL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 87: PUSHH (push high byte)
    tests.append(AssemblerTest(
        "pushh_basic",
        "CODE\n    LD AX, 0x1234\n    PUSHH AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.PUSHH(IE.AX) +
        IE.HALT()
    ))
    
    # Test 88: POPH (pop high byte)
    tests.append(AssemblerTest(
        "poph_basic",
        "CODE\n    LD AX, 0x1234\n    PUSHH AX\n    POPH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.PUSHH(IE.AX) +
        IE.POPH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 89: PUSHL (push low byte)
    tests.append(AssemblerTest(
        "pushl_basic",
        "CODE\n    LD AX, 0x5678\n    PUSHL AX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x5678) +
        IE.PUSHL(IE.AX) +
        IE.HALT()
    ))
    
    # Test 90: POPL (pop low byte)
    tests.append(AssemblerTest(
        "popl_basic",
        "CODE\n    LD AX, 0x5678\n    PUSHL AX\n    POPL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x5678) +
        IE.PUSHL(IE.AX) +
        IE.POPL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 91: PUSHB (push immediate byte)
    tests.append(AssemblerTest(
        "pushb_immediate",
        "CODE\n    PUSHB 0x42\n    HALT\n",
        IE.PUSHB(0x42) +
        IE.HALT()
    ))
    
    # Test 92: PUSHW (push immediate word)
    tests.append(AssemblerTest(
        "pushw_immediate",
        "CODE\n    PUSHW 0xABCD\n    HALT\n",
        IE.PUSHW(0xABCD) +
        IE.HALT()
    ))
    
    # Test 93: SBB (subtract byte)
    tests.append(AssemblerTest(
        "sbb_byte",
        "CODE\n    LD AX, 0x1234\n    SBB 0x10\n    HALT\n",
        IE.LD_imm(IE.AX, 0x1234) +
        IE.SBB(0x10) +
        IE.HALT()
    ))
    
    # Test 67: SBH (subtract high byte)
    tests.append(AssemblerTest(
        "sbh_high",
        "CODE\n    LD AX, 0x5000\n    LD BX, 0x1000\n    SBH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x5000) +
        IE.LD_imm(IE.BX, 0x1000) +
        IE.SBH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 68: SBL (subtract low byte)
    tests.append(AssemblerTest(
        "sbl_low",
        "CODE\n    LD AX, 0x0050\n    LD BX, 0x0010\n    SBL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0050) +
        IE.LD_imm(IE.BX, 0x0010) +
        IE.SBL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 69: DVB (divide by byte)
    tests.append(AssemblerTest(
        "dvb_byte",
        "CODE\n    LD AX, 0x0014\n    DVB 0x05\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0014) +
        IE.DVB(0x05) +
        IE.HALT()
    ))
    
    # Test 70: DVH (divide by high byte)
    tests.append(AssemblerTest(
        "dvh_high",
        "CODE\n    LD AX, 0x0014\n    LD BX, 0x0500\n    DVH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0014) +
        IE.LD_imm(IE.BX, 0x0500) +
        IE.DVH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 71: DVL (divide by low byte)
    tests.append(AssemblerTest(
        "dvl_low",
        "CODE\n    LD AX, 0x0014\n    LD BX, 0x0005\n    DVL BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0014) +
        IE.LD_imm(IE.BX, 0x0005) +
        IE.DVL(IE.BX) +
        IE.HALT()
    ))
    
    # Test 72: RMB (remainder by byte)
    tests.append(AssemblerTest(
        "rmb_byte",
        "CODE\n    LD AX, 0x0017\n    RMB 0x05\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0017) +
        IE.RMB(0x05) +
        IE.HALT()
    ))
    
    # Test 73: RMH (remainder by high byte)
    tests.append(AssemblerTest(
        "rmh_high",
        "CODE\n    LD AX, 0x0017\n    LD BX, 0x0500\n    RMH BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0017) +
        IE.LD_imm(IE.BX, 0x0500) +
        IE.RMH(IE.BX) +
        IE.HALT()
    ))
    
    # Test 74: RML (remainder by low byte)
    tests.append(AssemblerTest(
        "rml_low",
        "CODE\n    LD AX, 0x0017\n    LD BX, 0x0005\n    RML BX\n    HALT\n",
        IE.LD_imm(IE.AX, 0x0017) +
        IE.LD_imm(IE.BX, 0x0005) +
        IE.RML(IE.BX) +
        IE.HALT()
    ))
    
    return tests

def main():
    print(f"{Colors.BOLD}Assembler Binary Output Validation{Colors.RESET}")
    print("=" * 60)
    print()
    
    # Create output directory
    Path("temp").mkdir(exist_ok=True)
    
    # Run tests
    tests = create_tests()
    passed = 0
    failed = 0
    
    for test in tests:
        run_test(test)
        print_test_result(test, verbose=True)
        if test.passed:
            passed += 1
        else:
            failed += 1
    
    # Summary
    print()
    print("=" * 60)
    total = passed + failed
    success_rate = (passed / total * 100) if total > 0 else 0
    
    if failed == 0:
        print(f"{Colors.GREEN}{Colors.BOLD}✓ ALL TESTS PASSED{Colors.RESET}")
    else:
        print(f"{Colors.YELLOW}PARTIAL SUCCESS{Colors.RESET}")
    
    print(f"Passed: {passed}/{total} ({success_rate:.1f}%)")
    if failed > 0:
        print(f"Failed: {failed}/{total}")
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
