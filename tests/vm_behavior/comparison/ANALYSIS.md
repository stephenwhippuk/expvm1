# CMP Instruction Behavior Analysis

## Purpose

These tests document the current CMP instruction behavior for design validation purposes in version 1.1. **No code changes are made** - this is purely observational testing.

## Key Question: Signed vs Unsigned Comparison?

The CMP instruction compares two 16-bit values and returns:
- `-1` (0xFFFF) if first < second
- `0` (0x0000) if first == second
- `1` (0x0001) if first > second

**Critical Design Question**: Does CMP treat values as signed or unsigned integers?

### Signed Interpretation (Two's Complement)
- Range: -32768 to 32767
- 0xFFFF = -1
- 0x8000 = -32768
- Negative numbers < positive numbers

### Unsigned Interpretation
- Range: 0 to 65535
- 0xFFFF = 65535
- 0x8000 = 32768
- All comparisons are magnitude-based

## Test Suite

### 1. `cmp_negative_vs_positive.asm`
**Key Test**: Does -1 < 1?

```
LD AX, 0xFFFF    ; -1 or 65535?
CMP AX, 1
```

**Expected if Signed**: AX = 0xFFFF (meaning -1 < 1)  
**Expected if Unsigned**: AX = 0x0001 (meaning 65535 > 1)

**Result**: *(Run VM and record actual value)*

---

### 2. `cmp_negative_range.asm`
Tests multiple negative comparison scenarios:

| Test | Comparison | Signed Result | Unsigned Result |
|------|------------|---------------|-----------------|
| test1 | -1 vs 1 | 0xFFFF (-1 < 1) | 0x0001 (65535 > 1) |
| test2 | -5 vs -2 | 0xFFFF (-5 < -2) | 0xFFFF (65531 < 65534) |
| test3 | -100 vs 0 | 0xFFFF (-100 < 0) | 0x0001 (65436 > 0) |
| test4 | -1 vs -1 | 0x0000 (equal) | 0x0000 (equal) |
| test5 | -32768 vs 1 | 0xFFFF (-32768 < 1) | 0x0001 (32768 > 1) |

**Results**: *(Run and record each test value)*

---

### 3. `cmp_signed_boundaries.asm`
Tests edge cases at INT16_MIN/MAX boundaries:

| Test | Comparison | Signed | Unsigned |
|------|------------|--------|----------|
| test1 | 32767 vs -32768 | 0x0001 (>) | 0xFFFF (<) |
| test2 | -32768 vs 32767 | 0xFFFF (<) | 0x0001 (>) |
| test3 | 32767 vs 32767 | 0x0000 (=) | 0x0000 (=) |
| test4 | -32768 vs -32768 | 0x0000 (=) | 0x0000 (=) |
| test5 | 0 vs 32767 | 0xFFFF (<) | 0xFFFF (<) |
| test6 | 0 vs -32768 | 0x0001 (>) | 0xFFFF (<) |

**Results**: *(Run and record each test value)*

---

### 4. `cmp_sorting_scenario.asm`
Real-world test: Sorting mixed positive/negative array

Array: `[5, -3, 12, -8, 0, 1, -1]`

Tests comparisons needed for bubble sort ascending order.

**If Signed**: Negative numbers sort before positive (correct for signed sort)  
**If Unsigned**: Negative numbers (treated as large positives) sort after positive (incorrect)

**Results**: *(Run and record comparison results)*

---

### 5. `cmp_variant_consistency.asm`
Validates both CMP variants (reg-reg and reg-imm) handle negatives identically.

Should have:
- `test1 == test2` (both compare -1 vs 1)
- `test3 == test4` (both compare -100 vs -50)

**Results**: *(Verify consistency between variants)*

---

## Running Tests

```bash
# Assemble all tests
cd tests/vm_behavior
./run_comparison_tests.sh

# Run individual test
build/lvm comparison_results/cmp_negative_vs_positive.bin 0x0000

# Inspect memory to see results
# (Use debugger or memory dump to read result values)
```

## Recording Results

For each test, record:
1. **Register AX** after each CMP operation
2. **Memory values** where results are stored
3. Whether behavior matches signed or unsigned interpretation

### Result Template

```
Test: cmp_negative_vs_positive.asm
Execution: build/lvm comparison_results/cmp_negative_vs_positive.bin 0x0000

Memory at [result] (offset 0x0000): _______
Interpretation: [ ] Signed  [ ] Unsigned

Notes:
```

## Design Implications for V1.1

Based on observed behavior, document:

1. **Current Behavior**: Is CMP signed or unsigned?
2. **Design Decision**: Should this change in V1.1?
3. **Compatibility**: If changing, document breaking change
4. **Use Cases**: Which use cases require signed? Unsigned?
5. **New Instructions**: Need both CMPS (signed) and CMPU (unsigned)?

## Related Considerations

### Arithmetic Operations
- Does ADD/SUB handle overflow as signed or unsigned?
- Does MUL treat operands as signed or unsigned?
- Do flags (S, O, C) reflect signed or unsigned semantics?

### Conditional Jumps
Current jumps are flag-based:
- JPZ, JPNZ (zero flag)
- JPC, JPNC (carry flag)
- JPS, JPNS (sign flag)
- JPO, JPNO (overflow flag)

**Question**: Do we need signed-specific jumps like:
- JL (jump if less, signed)
- JLE (jump if less or equal, signed)
- JG (jump if greater, signed)
- JGE (jump if greater or equal, signed)

And unsigned versions:
- JB (jump if below, unsigned)
- JBE (jump if below or equal, unsigned)
- JA (jump if above, unsigned)
- JAE (jump if above or equal, unsigned)

## Conclusion

This test suite provides empirical data on current CMP behavior to inform V1.1 design decisions. All results should be documented before making any changes.
