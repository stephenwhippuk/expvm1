# Binary-Level Testing - Session Report
**Date**: 2025-12-22
**Session**: Initial VM Instruction Validation

## Objective
Test VM instruction execution at the binary level, bypassing the assembler to isolate VM bugs from assembler bugs.

## Approach

### 1. Created Binary Test Generator (`generate_test_binaries.py`)
- Python script to create raw binary files per specification
- Implements proper Pendragon binary format (Version 1.0.0)
- Encodes instructions according to PendragonOperations.md
- Created test binaries for:
  - HALT
  - LD (immediate to register)
  - LD (all registers)
  - ADD (basic and overflow)
  - CMP (equal, less than, greater than)

### 2. Binary Format Implementation
- Header section with metadata
- Little-endian encoding for multi-byte values
- Proper data and code segment structure
- Register encoding per spec: AX=0, BX=1, CX=2, DX=3, EX=4

## Critical Bug Discovered

### Issue #1: Register Encoding Off-by-One Error

**Severity**: CRITICAL
**Impact**: ALL register-based instructions affected

#### The Problem
When running our correctly-encoded test binary:
```
./build/lvm tests/instruction_validation/binaries/test_ld_ax.bin 0x0000
[CPU ERROR] Invalid register code: 0 (0x0)
```

#### Root Cause Analysis

**Specification** (PendragonOperations.md):
```
Register Encoding: AX=0, BX=1, CX=2, DX=3, EX=4
```

**VM Implementation** (src/cpu/include/cpu.h):
```cpp
enum register_codes {
    REG_AX = 0x01,  // ❌ Should be 0x00
    REG_BX = 0x02,  // ❌ Should be 0x01
    REG_CX = 0x03,  // ❌ Should be 0x02
    REG_DX = 0x04,  // ❌ Should be 0x03
    REG_EX = 0x05,  // ❌ Should be 0x04
};
```

**Assembler** (src/assembler/ir/code_graph.cpp):
```cpp
// Comment said: "CPU uses 1-based register codes (AX=1, BX=2, etc.)"
if (upper == "AX") return 0x01;  // ❌ Should be 0x00
if (upper == "BX") return 0x02;  // ❌ Should be 0x01
// etc.
```

#### Why This Went Undetected
The **assembler had the same bug** as the VM! They were both wrong in the same way:
- Assembler encoded AX as 0x01
- VM expected AX as 0x01
- Everything appeared to work

This is a textbook example of **compensating bugs** masking each other.

## The Fix

### Changed Files

**1. src/cpu/include/cpu.h**
```cpp
enum register_codes {
    REG_AX = 0x00,  // ✅ Fixed: Was 0x01
    REG_BX = 0x01,  // ✅ Fixed: Was 0x02
    REG_CX = 0x02,  // ✅ Fixed: Was 0x03
    REG_DX = 0x03,  // ✅ Fixed: Was 0x04
    REG_EX = 0x04,  // ✅ Fixed: Was 0x05
    REG_IR = 0x05,  // Adjusted from 0x06
    REG_SP = 0x06,  // Adjusted from 0x07
    REG_SI = 0x07,  // Adjusted
};
```

**2. src/assembler/ir/code_graph.cpp**
```cpp
// Fixed: Spec defines AX=0, BX=1, CX=2, DX=3, EX=4
if (upper == "AX") return 0x00;  // ✅ Fixed: Was 0x01
if (upper == "BX") return 0x01;  // ✅ Fixed: Was 0x02
if (upper == "CX") return 0x02;  // ✅ Fixed: Was 0x03
if (upper == "DX") return 0x03;  // ✅ Fixed: Was 0x04
if (upper == "EX") return 0x04;  // ✅ Fixed: Was 0x05
```

## Verification

### Binary Test Results
All hand-crafted binary tests now pass:
```bash
✓ test_halt.bin
✓ test_ld_ax.bin
✓ test_ld_all_regs.bin
✓ test_add_basic.bin
✓ test_add_overflow.bin
✓ test_cmp_equal.bin
```

### Assembler Verification
**Before Fix**:
```
Offset: 0x28  Data: 02 01 34 12 01
                     ^  ^^ (AX encoded as 0x01 - WRONG)
```

**After Fix**:
```
Offset: 0x28  Data: 02 00 34 12 01
                     ^  ^^ (AX encoded as 0x00 - CORRECT)
```

### Binary Comparison
Hand-crafted binary matches assembler output:
```
Hand-crafted: 02 00 34 12 01
Assembler:    02 00 34 12 01
              ✓ MATCH
```

## Impact Assessment

### Affected Code
- ✅ VM register handling - FIXED
- ✅ Assembler register encoding - FIXED
- ⚠️ All previously assembled binaries - NEED REASSEMBLY
- ⚠️ Any external tools using register codes - NEED UPDATE

### Breaking Changes
**All existing binary files assembled before this fix are now INVALID**
- They use old encoding (AX=0x01, etc.)
- Will fail or use wrong registers with fixed VM
- **Action Required**: Reassemble all `.asm` files

## Lessons Learned

### 1. Specification is King
The specification clearly stated AX=0, but both VM and assembler deviated from it. The spec must be the single source of truth.

### 2. Value of Binary-Level Testing
Testing at the binary level immediately exposed this critical bug that had been hidden by compensating errors in assembler and VM.

### 3. Compensating Bugs are Dangerous
When two components have matching bugs, they can mask each other. Only independent validation against specification catches these.

### 4. Off-by-One Errors in Enums
Starting enums at the wrong value is easy to do and hard to catch without proper testing.

### 5. Documentation Can Be Wrong
The assembler had a comment claiming "CPU uses 1-based register codes" - this was incorrect and misled developers.

## Next Steps

### Immediate
- [x] Fix VM register encoding
- [x] Fix assembler register encoding
- [x] Verify fixes with binary tests
- [x] Document the issue
- [ ] Run existing test suites to check for breakage
- [ ] Update any documentation mentioning register encoding

### Short Term
- [ ] Expand binary test coverage to all instructions
- [ ] Create automated binary test runner
- [ ] Verify all assembler tests still pass
- [ ] Check for other hardcoded register values in codebase

### Long Term
- [ ] Implement comprehensive binary-level test suite for all 119 instructions
- [ ] Create binary comparison tool for assembler validation
- [ ] Add CI checks to verify binary format compliance
- [ ] Document proper testing methodology

## Testing Methodology Validated

This session proves the value of the multi-level testing approach:

1. **Binary Level** (bypasses assembler)
   - Tests VM instruction execution directly
   - Validates binary format compliance
   - Catches VM implementation bugs
   
2. **Assembler Level** (compares output)
   - Validates assembler generates correct encoding
   - Can compare with hand-crafted binaries
   - Catches assembler bugs

3. **Integration Level** (end-to-end)
   - Tests full pipeline
   - Catches interaction issues
   - Validates real-world usage

**Key Insight**: Only binary-level testing found this bug because it bypassed the buggy assembler that was compensating for the buggy VM.

## Statistics

- **Time to Find Bug**: ~5 minutes after running first binary test
- **Time to Fix**: ~10 minutes (both VM and assembler)
- **Time to Verify**: ~5 minutes
- **Binary Tests Created**: 6
- **Critical Bugs Found**: 1
- **Critical Bugs Fixed**: 1

## Conclusion

Binary-level testing proved its value immediately by finding a critical, specification-violating bug that had been hidden by compensating errors. This validates our testing approach and emphasizes the importance of testing against the specification independently from other components.

The fix ensures the VM now correctly implements the register encoding as specified, and the assembler generates binaries that comply with the specification.

---

**Report Generated**: 2025-12-22
**Status**: Issue #1 Fixed and Verified
**Recommendation**: Continue with comprehensive binary-level instruction testing
