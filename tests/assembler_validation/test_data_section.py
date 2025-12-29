#!/usr/bin/env python3
"""
Data Section Validation Tests

Tests assembler's ability to generate correct data segments:
- DB (byte), DW (word), DA (address) definitions
- Data labels and address resolution
- PAGE directives in DATA sections
- Memory operations: LDA, LDAB, LDAH, LDAL (load/store variants)
- Indirect addressing
"""

import sys
import os
import subprocess
import struct
from pathlib import Path

# Add instruction validation directory to path for InstructionEncoder
sys.path.insert(0, str(Path(__file__).parent.parent / 'instruction_validation'))
from generate_test_binaries import InstructionEncoder as IE

# Path to assembler
ASSEMBLER = str(Path(__file__).parent.parent.parent / 'build' / 'asm')

class BinaryParser:
    """Parse Pendragon binary format"""
    
    def __init__(self, binary_data):
        self.data = binary_data
        self.offset = 0
    
    def read_u8(self):
        val = self.data[self.offset]
        self.offset += 1
        return val
    
    def read_u16(self):
        val = struct.unpack('<H', self.data[self.offset:self.offset+2])[0]
        self.offset += 2
        return val
    
    def read_u32(self):
        val = struct.unpack('<I', self.data[self.offset:self.offset+4])[0]
        self.offset += 4
        return val
    
    def read_string(self, length):
        val = self.data[self.offset:self.offset+length].decode('utf-8', errors='ignore')
        self.offset += length
        return val
    
    def skip(self, count):
        self.offset += count

def parse_binary(binary_path):
    """Parse a Pendragon binary and extract segments"""
    with open(binary_path, 'rb') as f:
        data = f.read()
    
    parser = BinaryParser(data)
    
    # Parse header
    header_size = parser.read_u16()
    
    # Skip to end of header
    parser.offset = header_size
    
    # Read data segment
    data_size = parser.read_u32()
    data_segment = list(parser.data[parser.offset:parser.offset + data_size])
    parser.skip(data_size)
    
    # Read code segment
    code_size = parser.read_u32()
    code_segment = list(parser.data[parser.offset:parser.offset + code_size])
    
    return {
        'header_size': header_size,
        'data_size': data_size,
        'data_segment': data_segment,
        'code_size': code_size,
        'code_segment': code_segment
    }

def assemble(source_code, output_name="test_output"):
    """Assemble source code and return binary path"""
    asm_path = f"/tmp/{output_name}.asm"
    bin_path = f"/tmp/{output_name}.bin"
    
    # Write source to temp file
    with open(asm_path, 'w') as f:
        f.write(source_code)
    
    # Assemble (use -o flag for output)
    result = subprocess.run(
        [ASSEMBLER, asm_path, "-o", bin_path],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"Assembly failed: {result.stderr}")
        return None
    
    return bin_path

def run_test(test_num, description, source_code, validator):
    """Run a single test"""
    print(f"Test {test_num:2d}: {description}...", end=" ")
    
    try:
        bin_path = assemble(source_code, f"data_test_{test_num}")
        if bin_path is None:
            print("✗ FAILED (assembly error)")
            return False
        
        binary = parse_binary(bin_path)
        
        if validator(binary):
            print("✓ PASSED")
            return True
        else:
            print("✗ FAILED")
            return False
    except Exception as e:
        print(f"✗ FAILED (exception: {e})")
        import traceback
        traceback.print_exc()
        return False

# ============================================================================
# TEST CASES
# ============================================================================

def test_db_single_byte():
    """Test DB with single byte value"""
    source = """
DATA
    mydata: DB [0x42]

CODE
    HALT
"""
    def validate(binary):
        # Data segment should contain: size (2 bytes) + data (1 byte)
        # Size: 0x01 0x00 (little-endian 1)
        # Data: 0x42
        data = binary['data_segment']
        if len(data) != 3:
            print(f"\n  Expected 3 bytes, got {len(data)}")
            return False
        if data[0] != 0x01 or data[1] != 0x00:
            print(f"\n  Expected size [0x01, 0x00], got [{data[0]:02x}, {data[1]:02x}]")
            return False
        if data[2] != 0x42:
            print(f"\n  Expected data 0x42, got {data[2]:02x}")
            return False
        return True
    
    return run_test(1, "DB single byte", source, validate)

def test_db_multiple_bytes():
    """Test DB with multiple byte values"""
    source = """
DATA
    mydata: DB [0x12, 0x34, 0x56]

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        # Size: 0x03 0x00 (3 bytes)
        # Data: 0x12 0x34 0x56
        if len(data) != 5:
            print(f"\n  Expected 5 bytes, got {len(data)}")
            return False
        if data[0] != 0x03 or data[1] != 0x00:
            print(f"\n  Expected size [0x03, 0x00], got [{data[0]:02x}, {data[1]:02x}]")
            return False
        if data[2:5] != [0x12, 0x34, 0x56]:
            print(f"\n  Expected data [0x12, 0x34, 0x56], got {data[2:5]}")
            return False
        return True
    
    return run_test(2, "DB multiple bytes", source, validate)

def test_dw_single_word():
    """Test DW with single word value"""
    source = """
DATA
    mydata: DW [0x1234]

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        # Size: 0x02 0x00 (2 bytes)
        # Data: 0x34 0x12 (little-endian)
        if len(data) != 4:
            print(f"\n  Expected 4 bytes, got {len(data)}")
            return False
        if data[0] != 0x02 or data[1] != 0x00:
            print(f"\n  Expected size [0x02, 0x00], got [{data[0]:02x}, {data[1]:02x}]")
            return False
        if data[2] != 0x34 or data[3] != 0x12:
            print(f"\n  Expected data [0x34, 0x12], got [{data[2]:02x}, {data[3]:02x}]")
            return False
        return True
    
    return run_test(3, "DW single word", source, validate)

def test_dw_multiple_words():
    """Test DW with multiple word values"""
    source = """
DATA
    mydata: DW [0x1234, 0x5678]

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        # Size: 0x04 0x00 (4 bytes)
        # Data: 0x34 0x12 0x78 0x56
        if len(data) != 6:
            print(f"\n  Expected 6 bytes, got {len(data)}")
            return False
        if data[0] != 0x04 or data[1] != 0x00:
            print(f"\n  Expected size [0x04, 0x00], got [{data[0]:02x}, {data[1]:02x}]")
            return False
        if data[2:6] != [0x34, 0x12, 0x78, 0x56]:
            print(f"\n  Expected [0x34, 0x12, 0x78, 0x56], got {[f'{b:02x}' for b in data[2:6]]}")
            return False
        return True
    
    return run_test(4, "DW multiple words", source, validate)

def test_db_string():
    """Test DB with string literal"""
    source = """
DATA
    msg: DB "Hi"

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        # Size: 0x02 0x00 (2 bytes)
        # Data: 0x48 0x69 ('H', 'i')
        if len(data) != 4:
            print(f"\n  Expected 4 bytes, got {len(data)}")
            return False
        if data[0] != 0x02 or data[1] != 0x00:
            print(f"\n  Expected size [0x02, 0x00], got [{data[0]:02x}, {data[1]:02x}]")
            return False
        if data[2] != ord('H') or data[3] != ord('i'):
            print(f"\n  Expected ['H', 'i'], got [{chr(data[2])}, {chr(data[3])}]")
            return False
        return True
    
    return run_test(5, "DB string literal", source, validate)

def test_multiple_data_items():
    """Test multiple data definitions"""
    source = """
DATA
    byte1: DB [0x10]
    word1: DW [0x2000]
    byte2: DB [0x30]

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        # byte1: size(2) + data(1) = 3 bytes: [0x01, 0x00, 0x10]
        # word1: size(2) + data(2) = 4 bytes: [0x02, 0x00, 0x00, 0x20]
        # byte2: size(2) + data(1) = 3 bytes: [0x01, 0x00, 0x30]
        # Total: 10 bytes
        expected = [0x01, 0x00, 0x10,           # byte1
                   0x02, 0x00, 0x00, 0x20,      # word1
                   0x01, 0x00, 0x30]            # byte2
        
        if len(data) != len(expected):
            print(f"\n  Expected {len(expected)} bytes, got {len(data)}")
            return False
        if data != expected:
            print(f"\n  Expected {expected}")
            print(f"\n  Got      {data}")
            return False
        return True
    
    return run_test(6, "Multiple data items", source, validate)

def test_lda_with_label():
    """Test LDA instruction referencing data label"""
    source = """
DATA
    value: DW [0x1234]

CODE
    LDA AX, value
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        # LDA AX, <address> should generate:
        # Expected: LDA with address operand
        expected = IE.LDA_load(0, 0x0000)  # AX (reg 0), address 0x0000 (data starts at 0)
        
        if len(code) < len(expected):
            print(f"\n  Code too short: {len(code)} < {len(expected)}")
            return False
        
        # Check opcode (first byte)
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        # Check register encoding
        if code[1] != expected[1]:
            print(f"\n  Expected register {expected[1]:02x}, got {code[1]:02x}")
            return False
        
        # Address should be 0x0000 (start of data segment)
        # Note: Data items have 2-byte size prefix, so actual data is at offset 2
        # But LDA references the label position, which is the start of the block (offset 0)
        return True
    
    return run_test(7, "LDA with data label", source, validate)

def test_ldab_with_label():
    """Test LDAB instruction referencing data label"""
    source = """
DATA
    byte_val: DB [0x42]

CODE
    LDAB AL, byte_val
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDAB(0, 0x0000)  # AL (reg 0), address 0x0000
        
        if len(code) < len(expected):
            print(f"\n  Code too short: {len(code)} < {len(expected)}")
            return False
        
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        return True
    
    return run_test(8, "LDAB with data label", source, validate)

def test_ldah_load_with_label():
    """Test LDAH (load) with data label"""
    source = """
DATA
    byte_val: DB [0x42]

CODE
    LDAH AL, byte_val
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDAH_load(0, 0x0000)  # AL (reg 0)
        
        if len(code) < len(expected):
            print(f"\n  Code too short")
            return False
        
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        return True
    
    return run_test(9, "LDAH load with data label", source, validate)

def test_ldal_load_with_label():
    """Test LDAL (load) with data label"""
    source = """
DATA
    byte_val: DB [0x42]

CODE
    LDAL AL, byte_val
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDAL_load(0, 0x0000)  # AL (reg 0)
        
        if len(code) < len(expected):
            return False
        
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        return True
    
    return run_test(10, "LDAL load with data label", source, validate)

def test_page_injection():
    """Test PAGE instruction injection for cross-page access"""
    source = """
DATA
PAGE page1
    val1: DW [0x1111]
    
PAGE page2
    val2: DW [0x2222]

CODE
    LDA AX, val1
    LDA BX, val2
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        
        # Assembler should inject PAGE instructions before LDA operations
        # Expected sequence:
        # PAGE 1 (immediate) - opcode 0x1B + page(2 bytes) + context(2 bytes)
        # LDA AX, val1
        # PAGE 2 (immediate)
        # LDA BX, val2
        # HALT
        
        # First should be PAGE 1
        if code[0] != 0x1B:  # PAGE immediate opcode
            print(f"\n  Expected PAGE opcode 0x1B, got {code[0]:02x}")
            return False
        
        # Page number (little-endian 1)
        page1 = code[1] | (code[2] << 8)
        if page1 != 1:
            print(f"\n  Expected page 1, got {page1}")
            return False
        
        return True
    
    return run_test(11, "PAGE instruction injection", source, validate)

def test_lda_store():
    """Test LDA store operation (address, register)"""
    source = """
DATA
    storage: DW [0x0000]

CODE
    LDA storage, AX
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDA_store(0x0000, 0)  # addr=0x0000, reg=AX (0)
        
        if len(code) < len(expected):
            print(f"\n  Code too short: {len(code)} < {len(expected)}")
            return False
        
        # Check opcode
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        # Check address (little-endian)
        addr = code[1] | (code[2] << 8)
        if addr != 0x0000:
            print(f"\n  Expected address 0x0000, got 0x{addr:04x}")
            return False
        
        # Check register
        if code[3] != expected[3]:
            print(f"\n  Expected register {expected[3]:02x}, got {code[3]:02x}")
            return False
        
        return True
    
    return run_test(12, "LDA store operation", source, validate)

def test_ldah_store():
    """Test LDAH store operation (address, register)"""
    source = """
DATA
    storage: DB [0x00]

CODE
    LDAH storage, AL
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDAH_store(0x0000, 0)  # addr=0x0000, reg=AL (0)
        
        if len(code) < len(expected):
            return False
        
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        return True
    
    return run_test(13, "LDAH store operation", source, validate)

def test_ldal_store():
    """Test LDAL store operation (address, register)"""
    source = """
DATA
    storage: DB [0x00]

CODE
    LDAL storage, AL
    HALT
"""
    def validate(binary):
        code = binary['code_segment']
        expected = IE.LDAL_store(0x0000, 0)  # addr=0x0000, reg=AL (0)
        
        if len(code) < len(expected):
            return False
        
        if code[0] != expected[0]:
            print(f"\n  Expected opcode {expected[0]:02x}, got {code[0]:02x}")
            return False
        
        return True
    
    return run_test(14, "LDAL store operation", source, validate)

def test_da_address_array():
    """Test DA (address array) definition"""
    source = """
DATA
    label1: DB [0x11]
    label2: DB [0x22]
    table: DA [label1, label2]

CODE
    HALT
"""
    def validate(binary):
        data = binary['data_segment']
        
        # label1: size(2) + data(1) = 3 bytes at offset 0
        # label2: size(2) + data(1) = 3 bytes at offset 3
        # table: size(2) + addr1(2) + addr2(2) = 6 bytes at offset 6
        
        if len(data) < 12:
            print(f"\n  Expected at least 12 bytes, got {len(data)}")
            return False
        
        # Check table size (should be 4 bytes for 2 addresses)
        table_offset = 6
        table_size = data[table_offset] | (data[table_offset+1] << 8)
        if table_size != 4:
            print(f"\n  Expected table size 4, got {table_size}")
            return False
        
        # Check addresses in table
        # Address of label1 should be 0x0000
        addr1 = data[table_offset+2] | (data[table_offset+3] << 8)
        if addr1 != 0x0000:
            print(f"\n  Expected addr1 0x0000, got 0x{addr1:04x}")
            return False
        
        # Address of label2 should be 0x0003 (after label1)
        addr2 = data[table_offset+4] | (data[table_offset+5] << 8)
        if addr2 != 0x0003:
            print(f"\n  Expected addr2 0x0003, got 0x{addr2:04x}")
            return False
        
        return True
    
    return run_test(15, "DA address array", source, validate)

def test_lda_indirect():
    """Test 16: LDA indirect addressing - LDA AX, BX"""
    source = """
DATA
    value: DW [0x5678]

CODE
    LD BX, value
    LDA AX, BX
    HALT
"""
    
    def validate(binary):
        # Expected code: LD BX, value + LDA AX, [BX] + HALT
        code = binary['code_segment']
        expected = (
            IE.LD_imm(IE.BX, 0x0000) +  # LD BX, 0x0000 (value label = 0)
            IE.LDA_indirect(IE.AX, IE.BX) +  # LDA AX, [BX]
            IE.HALT()
        )
        
        if bytes(code) != expected:
            print(f"\n  Code mismatch:")
            print(f"  Expected: {expected.hex()}")
            print(f"  Got:      {bytes(code).hex()}")
            return False
        
        return True
    
    return run_test(16, "LDA indirect addressing", source, validate)

def test_ldah_indirect():
    """Test 17: LDAH indirect addressing - LDAH AX, BX"""
    source = """
DATA
    value: DW [0x1234]

CODE
    LD BX, value
    LDAH AX, BX
    HALT
"""
    
    def validate(binary):
        # Expected code: LD BX, value + LDAH AX, [BX] + HALT
        code = binary['code_segment']
        expected = (
            IE.LD_imm(IE.BX, 0x0000) +  # LD BX, 0x0000 (value label = 0)
            IE.LDAH_indirect(IE.AX, IE.BX) +  # LDAH AX, [BX]
            IE.HALT()
        )
        
        if bytes(code) != expected:
            print(f"\n  Code mismatch:")
            print(f"  Expected: {expected.hex()}")
            print(f"  Got:      {bytes(code).hex()}")
            return False
        
        return True
    
    return run_test(17, "LDAH indirect addressing", source, validate)

def test_ldal_indirect():
    """Test 18: LDAL indirect addressing - LDAL AX, BX"""
    source = """
DATA
    value: DW [0x5678]

CODE
    LD BX, value
    LDAL AX, BX
    HALT
"""
    
    def validate(binary):
        # Expected code: LD BX, value + LDAL AX, [BX] + HALT
        code = binary['code_segment']
        expected = (
            IE.LD_imm(IE.BX, 0x0000) +  # LD BX, 0x0000 (value label = 0)
            IE.LDAL_indirect(IE.AX, IE.BX) +  # LDAL AX, [BX]
            IE.HALT()
        )
        
        if bytes(code) != expected:
            print(f"\n  Code mismatch:")
            print(f"  Expected: {expected.hex()}")
            print(f"  Got:      {bytes(code).hex()}")
            return False
        
        return True
    
    return run_test(18, "LDAL indirect addressing", source, validate)

# ============================================================================
# MAIN
# ============================================================================

def main():
    print("=" * 70)
    print("DATA SECTION VALIDATION TESTS")
    print("=" * 70)
    print()
    
    tests = [
        test_db_single_byte,
        test_db_multiple_bytes,
        test_dw_single_word,
        test_dw_multiple_words,
        test_db_string,
        test_multiple_data_items,
        test_lda_with_label,
        test_ldab_with_label,
        test_ldah_load_with_label,
        test_ldal_load_with_label,
        test_page_injection,
        test_lda_store,
        test_ldah_store,
        test_ldal_store,
        test_da_address_array,
        test_lda_indirect,
        test_ldah_indirect,
        test_ldal_indirect,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        if test():
            passed += 1
        else:
            failed += 1
    
    print()
    print("=" * 70)
    if failed == 0:
        print(f"✓ ALL TESTS PASSED")
    else:
        print(f"✗ SOME TESTS FAILED")
    print(f"Passed: {passed}/{len(tests)} ({passed/len(tests)*100:.1f}%)")
    print("=" * 70)
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
