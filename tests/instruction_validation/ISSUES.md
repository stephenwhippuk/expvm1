# Instruction Validation - Issues Found

## Purpose
Document bugs, unexpected behaviors, and VM issues discovered through systematic instruction testing.

## Issue Format

Each issue should include:
- **Issue ID**: Sequential number
- **Severity**: Critical / High / Medium / Low
- **Instruction(s)**: Affected opcodes
- **Status**: Open / In Progress / Fixed / Wontfix
- **Description**: What went wrong
- **Test File**: Which test exposed the issue
- **Expected**: What should happen
- **Actual**: What actually happens
- **Impact**: Consequences of the bug

---

## Open Issues

*(No open issues currently)*

---

## Fixed Issues

### Issue #5: CALL Instruction Missing Mandatory Flag Byte

**Severity**: Critical  
**Status**: ✅ **FIXED** (2025-12-23)  
**Instructions Affected**: CALL (0x27), RET (0x28)

**Discovered**: 2025-12-23  
**Test Files**: `binaries/test_call_basic.bin`, `binaries/test_call_nested.bin`, `binaries/test_call_stack.bin`

#### Description
The CALL instruction encoder in `generate_test_binaries.py` was only generating 3 bytes (opcode + 2-byte address) but the VM expects 4 bytes (opcode + 2-byte address + 1-byte flag). The missing flag byte caused the CPU to read the next instruction byte as the flag parameter, resulting in incorrect return value handling and stack corruption.

#### Bug Manifestation
```python
# BUGGY encoder (missing flag byte):
def CALL(addr: int, with_return: bool = False) -> bytes:
    return bytes([0x27]) + encode_word(addr)  # Only 3 bytes!

# When VM executed CALL at 0x0006, it read:
# Byte 0x0006: 0x27 (CALL opcode) ✓
# Byte 0x0007: 0x0D (low byte of address) ✓
# Byte 0x0008: 0x00 (high byte of address) ✓
# Byte 0x0009: 0x02 (LD opcode from NEXT instruction!) ✗ ← Read as flag!
```

#### Debug Output That Revealed Bug
```
[DEBUG] CALL: pushed flag=1 FP=-1 SP=1   ← Should be flag=0!
[DEBUG] RET: has_return_value=1 FP=0 SP=1
[DEBUG] Stack underflow in pop_word! SP=1 FP=0 need>=3
Runtime error: Return stack underflow on return from subroutine
```

The flag was 1 because the CPU was reading 0x02 (the LD opcode from the next instruction) as the flag byte.

#### Fix Applied
Updated CALL encoder to always include the mandatory 3rd parameter byte:
```python
def CALL(addr: int, with_return: bool = False) -> bytes:
    flag = 1 if with_return else 0
    return bytes([0x27]) + encode_word(addr) + bytes([flag])  # Now 4 bytes
```

Recalculated all address offsets in tests 80-82 (CALL changed from 3 to 4 bytes).

#### Test Results
**Before fix**: 0/3 CALL/RET tests passing
- All CALL tests failed with "Return stack underflow"
- CPU misinterpreted subsequent instruction bytes as flags

**After fix**: ✅ **3/3 CALL/RET tests passing**
- test_call_basic - PASS (basic subroutine call and return)
- test_call_nested - PASS (2-level deep nested calls)
- test_call_stack - PASS (PUSH/POP within subroutine)
- test_ret_without_call - PASS (intentional error test)

#### Files Modified
- `tests/instruction_validation/generate_test_binaries.py` - Fixed CALL encoder, recalculated offsets

#### Verification
```
=== test_call_basic ===
(SUCCESS - exit 0)

=== test_call_nested ===
(SUCCESS - exit 0)

=== test_call_stack ===
(SUCCESS - exit 0)

=== test_ret_without_call ===
Runtime error: Return stack underflow on return from subroutine
(EXPECTED FAILURE)
```

#### Related Discoveries
- Created CodeBuilder helper class for automatic position tracking
- Eliminated manual address calculation errors
- Fixed all JMP tests (77-79) as part of control flow testing phase

#### Lessons Learned
1. **Instruction encoding must match VM expectations exactly** - No optional bytes
2. **Binary-level testing catches encoding bugs** - Assembler would have hidden this
3. **Debug instrumentation is essential** - Showed exact flag values being pushed/popped
4. **Helper classes reduce errors** - CodeBuilder eliminated manual offset calculations
5. **All instruction parameters are mandatory** - Even "optional" flags must always be present

---

### Issue #4: ALU Immediate Operations Byte Order Bug

**Severity**: Critical  
**Status**: ✅ **FIXED** (2025-12-23)  
**Instructions Affected**: ALL immediate ALU operations (13 instructions):
- ADD_IMM_W (0x2C), SUB_IMM_W (0x2E), MUL_IMM_W (0x33), DIV_IMM_W (0x38)
- REM_IMM_W (0x3D), AND_IMM_W (0x42), OR_IMM_W (0x47), XOR_IMM_W (0x4C)
- NOT_IMM (0x51), SHL_IMM (0x56), SHR_IMM (0x5B), ROL_IMM (0x60), ROR_IMM (0x65)

**Discovered**: 2025-12-23  
**Test Files**: `binaries/test_jpnz_not_taken.bin`, `binaries/test_sub_immediate.bin`

#### Description
All immediate ALU operations in `cpu_alu_ops.cpp` were calling `combine_bytes_to_word()` with parameters in the wrong order. The function expects `(low, high)` for little-endian encoding, but all ALU ops were calling it as `(high, low)`, causing immediate values to be byte-swapped.

#### Example Bug
```
Binary encoding: SUB 0x0005 → [0x2E, 0x05, 0x00]
CPU reads: params[0]=0x05, params[1]=0x00

Buggy code: combine_bytes_to_word(params[1], params[0])
         = combine_bytes_to_word(0x00, 0x05)
         = 0x0500  ❌ WRONG!

Correct:    combine_bytes_to_word(params[0], params[1])
         = combine_bytes_to_word(0x05, 0x00)
         = 0x0005  ✓ CORRECT!
```

#### Impact
- SUB 5 from 5 resulted in 0xFB05 instead of 0x0000, preventing ZERO flag from being set
- Conditional jumps that depended on flag state failed (e.g., JPNZ when zero should be set)
- ALL immediate ALU operations produced incorrect results
- Unit tests passed because they called functions directly, not through binary dispatch
- Only manifested when running actual binary code

#### Test That Exposed Issue
Test `test_jpnz_not_taken.bin` timed out infinitely:
```
LD AX, 0x0005    # AX = 5
SUB 0x0005       # Should make AX = 0, ZERO = true
JPNZ 0x00FF      # Should NOT jump (ZERO is true)
HALT             # Should reach here
```

**What happened**: SUB subtracted 0x0500 instead of 0x0005, result was 0xFB05 (not zero), JPNZ jumped to invalid memory, infinite loop.

#### Root Cause Analysis
The endianness bug in `combine_bytes_to_word()` was previously fixed (Issue #3), changing the function from big-endian to little-endian. However, some caller sites (memory operations, jumps) were updated, but **all 13 ALU immediate operations were missed**.

**Buggy pattern** in `src/cpu/cpu_alu_ops.cpp`:
```cpp
case OPCODE_SUB_IMM_W:
    word_t value = combine_bytes_to_word(params[1], params[0]);  // ❌ WRONG ORDER
    alu_->sub(value);
    break;
```

**Corrected pattern**:
```cpp
case OPCODE_SUB_IMM_W:
    word_t value = combine_bytes_to_word(params[0], params[1]);  // ✓ CORRECT ORDER
    alu_->sub(value);
    break;
```

#### Fix Applied
Mass replacement in `src/cpu/cpu_alu_ops.cpp`:
- Changed all 13 instances of `combine_bytes_to_word(params[1], params[0])`
- To: `combine_bytes_to_word(params[0], params[1])`
- Used `sed -i 's/combine_bytes_to_word(params\[1\], params\[0\])/combine_bytes_to_word(params[0], params[1])/g'`

#### Debug Process
Added temporary debug logging to trace execution:
```cpp
std::cout << "[DEBUG] IR=0x" << std::hex << current_ir << " Opcode=0x" << (int)opcode << std::endl;
std::cout << "[DEBUG] SUB immediate: value=0x" << std::hex << value << std::endl;
std::cout << "[DEBUG] After SUB: AX=0x" << std::hex << ax_val << " ZERO=" << zero_flag << std::endl;
```

This revealed:
```
[DEBUG] SUB immediate: value=0x500    ← Should be 0x5!
[DEBUG] After SUB: AX=0xfb05 ZERO=0   ← Should be 0x0 ZERO=1!
[DEBUG] Taking jump to 0xff           ← Should NOT jump!
```

#### Test Results
**Before fix**: 73/76 tests passing (96%)
- test_jpnz_not_taken.bin - TIMEOUT
- Many ALU tests happened to work due to test data symmetry

**After fix**: ✅ **75/77 tests passing (97.4%)**
- test_jpnz_not_taken.bin - NOW PASSES
- All conditional jump tests - PASS (16/16)
- All ALU immediate operations - PASS
- Only 2 failures: intentional error tests (underflow conditions)

#### Files Modified
- `src/cpu/cpu_alu_ops.cpp` - Fixed 13 parameter order bugs

#### Verification Results
✅ All 194 C++ unit tests pass (were already passing)
✅ All 16 conditional jump tests now pass (JPZ, JPNZ, JPC, JPNC, JPS, JPNS, JPO, JPNO - both taken and not-taken)
✅ All immediate ALU operations verified
✅ Added comprehensive flag testing to ALU subtraction unit test

#### Related Issues
- **Issue #3**: Original endianness fix that introduced this regression
- Root cause was incomplete migration when fixing combine_bytes_to_word signature

#### Lessons Learned
1. **Binary-level testing is essential** - Unit tests alone missed this bug
2. **Function signature changes require exhaustive caller review** - 13 call sites were missed
3. **Debug logging is invaluable** - Showed exact values being computed
4. **Systematic testing reveals edge cases** - Conditional jumps exposed ALU flag issues
5. **Test at the right level** - This only manifested in binary execution, not unit tests


### Issue #3: Endianness Mismatch in combine_bytes Functions

**Severity**: Critical  
**Status**: ✅ **FIXED** (2025-12-22)  
**Instructions Affected**: ALL 16-bit immediate operands:
- SETF (0x1D), JMP family (0x1E-0x26), CALL (0x27)
- PEEK family with offset (0x16-0x19), Memory operations (0x09-0x0F)
- LD immediate (0x02), PUSHW (0x14), PAGE (0x1A-0x1B)
- SYS_FUNC (0x7F)

**Discovered**: 2025-12-22  
**Test Files**: `binaries/test_setf_basic.bin`, `binaries/test_peek_offset.bin`

#### Description
The helper functions `combine_bytes_to_address` and `combine_bytes_to_word` used big-endian byte order `(high << 8) | low`, but the Pendragon binary format specifies little-endian (low byte first). This caused all 16-bit immediate operands to be incorrectly decoded.

#### Example Bug
```
Binary encoding: SETF 0x0004 → [0x1D, 0x04, 0x00]
CPU reads: params[0]=0x04, params[1]=0x00
Old helper: combine_bytes_to_address(0x04, 0x00) = (0x04 << 8) | 0x00 = 0x0400 = 1024
Result: "Frame pointer exceeds stack capacity" (thought FP was 1024, not 4!)
```

#### Root Cause
**Helper functions** in `src/helpers/helpers.cpp` treated first parameter as high byte (big-endian):
```cpp
addr_t combine_bytes_to_address(byte_t high, byte_t low) {
    return (high << 8) | low;  // ❌ BIG-ENDIAN
}
```

Some CPU instructions compensated by swapping parameters (CALL, LD), others didn't (SETF, JMP), causing inconsistent failures.

#### Fix Applied
1. **Changed function signatures** to `(byte_t low, byte_t high)` for clarity
2. **Updated implementation** to little-endian: `return (low) | (high << 8);`
3. **Standardized all CPU calls** to pass `(params[0], params[1])` or `(params[1], params[2])`
4. **Removed compensation swaps** from instructions that were working around the bug

#### Test Results
✅ **ALL 33 tests now PASS**
- test_setf_basic.bin - NOW PASSES
- test_peek_offset.bin - NOW PASSES  
- test_peekf.bin - NOW PASSES
- test_peekfb.bin - NOW PASSES
- All jump, memory, and arithmetic tests - PASS


### Issue #2: PEEK Offset Calculation Error

**Severity**: High  
**Status**: ✅ **FIXED** (2025-12-22) - Same root cause as Issue #3  
**Instruction**: PEEK (0x16), PEEKF (0x17), PEEKB (0x18), PEEKFB (0x19)  
**Discovered**: 2025-12-22  
**Test File**: `binaries/test_peek_offset.bin`, `binaries/test_peekf.bin`, `binaries/test_peekfb.bin`



### Issue #1: Register Encoding Off-by-One Error

**Severity**: Critical
**Status**: Fixed (2025-12-22)
**Instruction**: ALL instructions using register operands (0x02-0x08, 0x09-0x74, etc.)
**Discovered**: 2025-12-22
**Test File**: `binaries/test_ld_ax.bin`

#### Description
The VM's register encoding was off-by-one from the specification. The CPU defined `REG_AX = 0x01` when the specification clearly states AX should be encoded as `0x00`.

#### Expected Behavior (Per Specification)
```
Register Encoding: Registers are encoded as bytes: AX=0, BX=1, CX=2, DX=3, EX=4
```
From: `specification/PendragonOperations.md`

#### Actual Behavior (VM Implementation)
```cpp
enum register_codes {
    REG_AX = 0x01,  // Should be 0x00
    REG_BX = 0x02,  // Should be 0x01
    REG_CX = 0x03,  // Should be 0x02
    REG_DX = 0x04,  // Should be 0x03
    REG_EX = 0x05,  // Should be 0x04
};
```
From: `src/cpu/include/cpu.h`

#### Impact
- **ALL** instructions using register operands would fail or use wrong registers
- Hand-crafted binary files (correctly encoded) were rejected with "Invalid register code" error
- Assembler may have been compensating for this bug, masking the issue
- Any external tools or programs generating binaries would fail

#### Test That Exposed Issue
Generated binary test with correctly encoded LD instruction:
```
LD AX, 0x1234
Encoded as: [0x02, 0x00, 0x34, 0x12]
           opcode  AX   value(LE)
```

VM Error:
```
[CPU ERROR] Invalid register code: 0 (0x0)
Exception: Invalid register code: 0
```

#### Root Cause
The register codes enum started at 0x01 instead of 0x00, likely a historical artifact or misunderstanding of the specification.

#### Fix Applied
```cpp
enum register_codes {
    REG_AX = 0x00,  // Fixed: Was 0x01
    REG_BX = 0x01,  // Fixed: Was 0x02
    REG_CX = 0x02,  // Fixed: Was 0x03
    REG_DX = 0x03,  // Fixed: Was 0x04
    REG_EX = 0x04,  // Fixed: Was 0x05
    REG_IR = 0x05,  // Adjusted from 0x06
    REG_SP = 0x06,  // Adjusted from 0x07
    REG_SI = 0x07,  // Adjusted
};
```

#### Verification Needed
After this fix:
1. ✅ Binary tests now execute correctly
2. ✅ **Assembler was also affected** - had same bug, now fixed
3. ⚠️ **All existing binary files** assembled before this fix may be incorrectly encoded
4. ⚠️ **Any hardcoded register values** in code need review

#### Files Modified
- `src/cpu/include/cpu.h` - Fixed VM register encoding
- `src/assembler/ir/code_graph.cpp` - Fixed assembler register encoding

#### Verification Results
✅ Hand-crafted binary tests pass with fixed VM
✅ Assembler now generates correct encoding (AX=0x00)
✅ Assembler output matches hand-crafted binaries
✅ Old assembled binaries will NOT work (need reassembly)

#### Related Issues
None yet - this is the first issue found through binary-level testing.

#### Lessons Learned
This demonstrates the critical value of testing at the binary level:
1. Specifications must be the source of truth
2. Direct binary testing catches fundamental encoding errors
3. Assembler may mask VM bugs by compensating
4. Off-by-one errors in enums are easy to introduce and hard to spot

---

## Known Limitations / Design Decisions

*Document intentional behaviors that might seem like bugs*

### Example:
- **CMP Signed/Unsigned**: Document whether comparisons treat values as signed or unsigned
- **Flag Preservation**: Which instructions preserve vs. clear flags
- **Stack Overflow**: Behavior when stack exceeds limits

---

## Testing Methodology

Issues are discovered through:
1. **Instruction Isolation Tests** - Testing each instruction independently
2. **Edge Case Tests** - Boundary values (0, max, overflow)
3. **Integration Tests** - Instruction combinations
4. **Flag Behavior Tests** - Verifying flag setting/clearing
5. **Cross-Instruction Tests** - Comparing similar instructions for consistency

---

## Reporting New Issues

When a test fails:

1. **Verify the test is correct** - Check test expectations match specification
2. **Isolate the problem** - Create minimal test case
3. **Document thoroughly** - Record all relevant state
4. **Add to this file** - Use template below
5. **Update test status** - Mark in COVERAGE.md

### Issue Template

```markdown
## Issue #N: [Brief Description]

**Severity**: [Critical/High/Medium/Low]
**Status**: Open
**Instruction**: [Mnemonic] (0x[HEX])
**Discovered**: [Date]
**Test File**: `path/to/test.asm`

### Description
[What went wrong]

### Expected Behavior
[What should happen according to spec]

### Actual Behavior
[What actually happens]

### Register/Memory State
- Before: AX=0x[...], BX=0x[...], etc.
- After: AX=0x[...], BX=0x[...], etc.
- Memory: [if relevant]

### Impact
[Consequences - crashes, wrong results, etc.]

### Reproduction Steps
1. Load test file
2. Execute
3. Observe result

### Suggested Fix
[If known]

### Related Issues
[Links to similar issues]
```

---

## Issue Severity Definitions

### Critical
- VM crashes
- Memory corruption
- Security vulnerabilities
- Complete instruction non-functionality

### High
- Wrong results that would break most programs
- Incorrect flag setting affecting control flow
- Major spec deviations

### Medium
- Edge case failures
- Minor spec deviations
- Inconsistencies between similar instructions

### Low
- Documentation issues
- Performance problems
- Nice-to-have improvements

---

## Cross-Reference

Issues should be cross-referenced with:
- **COVERAGE.md** - Mark instructions with known issues
- **Test files** - Link test that exposed issue
- **Source code** - Link to implementation (if known)
- **Specification** - Reference relevant spec sections

---

**Last Updated**: 2025-12-22
