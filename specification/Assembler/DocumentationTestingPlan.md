# Documentation Testing Plan

## Overview

This document outlines the comprehensive testing strategy for all code examples in the assembler documentation. The goal is to ensure every code example assembles correctly, generates valid binaries, and produces expected results.

## Scope

### Documentation Files to Test

1. **Overview.md** - ~2 examples
2. **Syntax.md** - ~25 examples
3. **Registers.md** - ~15 examples
4. **InstructionSet.md** - ~40 examples
5. **Expressions.md** - ~30 examples
6. **ErrorReference.md** - ~35 examples (including error cases)
7. **BinaryFormat.md** - ~7 examples
8. **Examples.md** - ~35 examples

**Total**: ~189 code examples identified

### Example Categories

1. **Valid Programs** - Should assemble successfully and generate binaries
2. **Error Examples** - Should fail with specific error messages (from ErrorReference.md)
3. **Partial/Conceptual** - Incomplete snippets not meant to be standalone (marked with comments)

## Testing Strategy

### Phase 1: Manual Review (CURRENT)

**Goal**: Categorize all examples and identify which need testing

**Tasks**:
- [ ] Extract all code blocks from documentation files
- [ ] Classify each example:
  - `TESTABLE` - Complete, standalone program
  - `ERROR_CASE` - Intentionally invalid (from ErrorReference.md)
  - `CONCEPTUAL` - Partial snippet for illustration only
  - `NEEDS_FIX` - Should be testable but has issues
- [ ] Create manifest file listing all examples with metadata

**Deliverable**: `tests/documentation/manifest.json`

```json
{
  "examples": [
    {
      "id": "syntax_hello_world",
      "source_file": "Syntax.md",
      "line_number": 11,
      "category": "TESTABLE",
      "description": "Basic program structure example",
      "expected_outcome": "SUCCESS"
    },
    {
      "id": "error_invalid_identifier",
      "source_file": "ErrorReference.md",
      "line_number": 39,
      "category": "ERROR_CASE",
      "expected_error": "Invalid identifier",
      "expected_outcome": "FAILURE"
    }
  ]
}
```

### Phase 2: Test Infrastructure

**Goal**: Build automated testing framework

#### Directory Structure

```
tests/
├── documentation/
│   ├── manifest.json           # Example inventory
│   ├── testable/               # Valid examples
│   │   ├── syntax/
│   │   │   ├── hello_world.asm
│   │   │   ├── hello_world.expected
│   │   │   └── ...
│   │   ├── registers/
│   │   ├── instructions/
│   │   ├── expressions/
│   │   └── examples/
│   ├── error_cases/            # Invalid examples
│   │   ├── lexer_errors/
│   │   ├── parser_errors/
│   │   └── semantic_errors/
│   ├── run_doc_tests.sh        # Main test script
│   └── validate.py             # Result validation
```

#### Test Script (`run_doc_tests.sh`)

```bash
#!/bin/bash
# Automated documentation example tester

ASSEMBLER="../build/pendragon_asm"
TEST_DIR="tests/documentation"
RESULTS_DIR="$TEST_DIR/results"

# Initialize
mkdir -p "$RESULTS_DIR"
PASSED=0
FAILED=0

# Test valid examples
echo "Testing valid examples..."
for asm_file in "$TEST_DIR/testable"/**/*.asm; do
    base=$(basename "$asm_file" .asm)
    dir=$(dirname "$asm_file")
    
    echo -n "Testing $base... "
    
    # Assemble
    if $ASSEMBLER "$asm_file" -o "$RESULTS_DIR/${base}.bin" 2> "$RESULTS_DIR/${base}.err"; then
        # Check if .expected file exists
        if [ -f "$dir/${base}.expected" ]; then
            # Validate results
            if python3 validate.py "$RESULTS_DIR/${base}.bin" "$dir/${base}.expected"; then
                echo "✓ PASS"
                ((PASSED++))
            else
                echo "✗ FAIL (incorrect output)"
                ((FAILED++))
            fi
        else
            echo "✓ PASS (assembled successfully)"
            ((PASSED++))
        fi
    else
        echo "✗ FAIL (assembly failed)"
        cat "$RESULTS_DIR/${base}.err"
        ((FAILED++))
    fi
done

# Test error cases
echo ""
echo "Testing error cases..."
for asm_file in "$TEST_DIR/error_cases"/**/*.asm; do
    base=$(basename "$asm_file" .asm)
    
    echo -n "Testing error case $base... "
    
    # Should fail to assemble
    if ! $ASSEMBLER "$asm_file" -o "$RESULTS_DIR/${base}.bin" 2> "$RESULTS_DIR/${base}.err"; then
        # Check for expected error message if .expected_error exists
        if [ -f "${asm_file%.asm}.expected_error" ]; then
            expected=$(cat "${asm_file%.asm}.expected_error")
            if grep -q "$expected" "$RESULTS_DIR/${base}.err"; then
                echo "✓ PASS (correct error)"
                ((PASSED++))
            else
                echo "✗ FAIL (wrong error message)"
                ((FAILED++))
            fi
        else
            echo "✓ PASS (failed as expected)"
            ((PASSED++))
        fi
    else
        echo "✗ FAIL (should have failed)"
        ((FAILED++))
    fi
done

# Report
echo ""
echo "======================================"
echo "Test Results:"
echo "  Passed: $PASSED"
echo "  Failed: $FAILED"
echo "  Total:  $((PASSED + FAILED))"
echo "======================================"

exit $FAILED
```

#### Validation Script (`validate.py`)

```python
#!/usr/bin/env python3
"""
Validate binary output against expected results
"""

import sys
import json
import struct

def validate_binary(binary_path, expected_path):
    """
    Validate binary against expected file
    Expected file format (JSON):
    {
        "binary_size": 123,
        "header": {
            "machine_name": "Pendragon",
            "program_name": "test"
        },
        "data_segment_size": 10,
        "code_segment_size": 5,
        "registers_at_halt": {
            "AX": "0x0042",
            "BX": "0x0000"
        }
    }
    """
    with open(binary_path, 'rb') as f:
        binary_data = f.read()
    
    with open(expected_path, 'r') as f:
        expected = json.load(f)
    
    # Check binary size
    if 'binary_size' in expected:
        if len(binary_data) != expected['binary_size']:
            print(f"Size mismatch: expected {expected['binary_size']}, got {len(binary_data)}")
            return False
    
    # Check header
    if 'header' in expected:
        # Parse header (simplified)
        offset = 2  # Skip header size
        offset += 4  # Skip version
        
        # Machine name
        name_len = binary_data[offset]
        offset += 1
        machine_name = binary_data[offset:offset+name_len].decode('ascii')
        
        if machine_name != expected['header'].get('machine_name', 'Pendragon'):
            print(f"Machine name mismatch: expected {expected['header']['machine_name']}, got {machine_name}")
            return False
    
    # Check segment sizes
    # (Would need to parse full binary format)
    
    print("Validation passed")
    return True

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: validate.py <binary> <expected>")
        sys.exit(1)
    
    if validate_binary(sys.argv[1], sys.argv[2]):
        sys.exit(0)
    else:
        sys.exit(1)
```

### Phase 3: Example Extraction

**Goal**: Extract examples from documentation into test files

#### Extraction Process

1. **Parse Markdown Files**
   - Identify all ```assembly code blocks
   - Extract surrounding context (description, expected results)
   - Generate unique ID for each example

2. **Create Test Files**
   - Write each example to `.asm` file
   - Add header comment with metadata
   - Create `.expected` file if validation data available

3. **Handle Special Cases**
   - **Error examples**: Move to `error_cases/` directory
   - **Partial snippets**: Mark as conceptual, don't test
   - **Examples with variations**: Create separate test for each

#### Example Test File Format

**File**: `tests/documentation/testable/syntax/hello_world.asm`

```assembly
; ============================================
; TEST: hello_world
; SOURCE: Syntax.md (line 11)
; DESCRIPTION: Basic program structure
; EXPECTED: Assembles successfully
; ============================================

DATA
    message: DB "Hello, World!"
    msg_len: DW [13]

CODE
start:
    LD AX, [message]
    HALT
```

**File**: `tests/documentation/testable/syntax/hello_world.expected` (optional)

```json
{
  "binary_size": null,
  "assembles": true,
  "registers_at_halt": {
    "AX": "0x0048"
  }
}
```

### Phase 4: Automated Testing

**Goal**: Run tests automatically and report results

#### CI Integration

```yaml
# .github/workflows/doc-tests.yml
name: Documentation Tests

on: [push, pull_request]

jobs:
  test-docs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Build assembler
        run: |
          mkdir build
          cd build
          cmake ..
          make
      
      - name: Run documentation tests
        run: |
          cd tests/documentation
          ./run_doc_tests.sh
      
      - name: Upload test results
        if: always()
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: tests/documentation/results/
```

#### Manual Testing

```bash
# Build project
cmake --build build

# Run all documentation tests
cd tests/documentation
./run_doc_tests.sh

# Run specific category
./run_doc_tests.sh --category testable/syntax

# Run with verbose output
./run_doc_tests.sh --verbose

# Generate HTML report
./run_doc_tests.sh --html-report
```

### Phase 5: Continuous Maintenance

**Goal**: Keep tests in sync with documentation

#### Workflow

1. **When Adding New Documentation**:
   - Extract code examples
   - Add to test suite
   - Run tests before committing

2. **When Updating Examples**:
   - Update test files
   - Re-run affected tests
   - Update expected results if needed

3. **When Fixing Bugs**:
   - If bug affects examples, update tests
   - Ensure all examples still work
   - Add regression test if needed

#### Automated Sync

**Script**: `sync_doc_tests.sh`

```bash
#!/bin/bash
# Extract examples from docs and update test suite

python3 extract_examples.py \
  --docs "specification/Assembler/Reference/*.md" \
  --output "tests/documentation" \
  --update-manifest
```

## Test Categories

### 1. Syntax Tests (25 examples)

- Basic structure
- Comments
- Labels
- Data definitions (DB, DW)
- Instructions
- Operands
- Numeric literals
- Complete programs

**Priority**: HIGH - Foundation for all other tests

### 2. Register Tests (15 examples)

- 16-bit register usage
- 8-bit sub-register usage
- Register encoding
- Register preservation
- Flag interactions

**Priority**: HIGH - Core functionality

### 3. Instruction Tests (40 examples)

- Each instruction category:
  - Control flow (JMP, CALL, RET, etc.)
  - Data movement (LD, LDAB, LDA, etc.)
  - Stack operations (PUSH, POP)
  - Arithmetic (ADD, SUB, MUL, DIV, etc.)
  - Logical (AND, OR, XOR, NOT)
  - Bit operations (SHL, SHR, ROL, ROR)
  - Comparison (CMP, CPL)
  - System (HALT, NOP, etc.)

**Priority**: HIGH - Complete instruction coverage

### 4. Expression Tests (30 examples)

- All addressing modes
- Operator precedence
- Label arithmetic
- Array indexing
- Structure access
- Complex expressions

**Priority**: MEDIUM - Advanced features

### 5. Error Tests (35 examples)

- Lexer errors
- Parser errors
- Semantic errors
- Codegen errors
- Each error should produce specific message

**Priority**: MEDIUM - Ensure good error messages

### 6. Binary Format Tests (7 examples)

- Header validation
- Segment structure
- Encoding verification
- Memory layout

**Priority**: LOW - More about documentation than functionality

### 7. Complete Program Tests (35 examples from Examples.md)

- Hello World
- Arithmetic operations
- Loops and counting
- Array operations
- String processing
- Functions/subroutines
- Conditional logic
- Memory operations
- Bit manipulation
- Sorting algorithm

**Priority**: HIGH - Real-world usage

## Success Criteria

### Phase 1 Complete When:
- [ ] All 189 examples catalogued
- [ ] Each example classified
- [ ] Manifest file created

### Phase 2 Complete When:
- [ ] Directory structure created
- [ ] `run_doc_tests.sh` implemented
- [ ] `validate.py` implemented
- [ ] Can run tests manually

### Phase 3 Complete When:
- [ ] All testable examples extracted to `.asm` files
- [ ] Error cases properly separated
- [ ] Expected result files created where applicable

### Phase 4 Complete When:
- [ ] All tests passing
- [ ] CI integration working
- [ ] Test report generated

### Phase 5 Complete When:
- [ ] Sync script implemented
- [ ] Documentation updated with testing instructions
- [ ] Maintenance workflow documented

## Known Issues / Challenges

### 1. Conceptual Examples

**Issue**: Some examples are partial snippets for illustration, not complete programs.

**Solution**: Mark these in manifest as `CONCEPTUAL` and skip testing. Add comments to documentation indicating they're not complete.

### 2. Examples Requiring Runtime Validation

**Issue**: Some examples (especially in Examples.md) need to verify register/memory state after execution.

**Challenge**: No VM implementation yet to run programs.

**Solutions**:
- **Short-term**: Test assembly only (verify binary generation)
- **Long-term**: When VM ready, add execution validation
- **Alternative**: Create binary format validator to check instruction encoding

### 3. Platform-Specific Examples

**Issue**: None currently, but may arise.

**Solution**: Tag with platform requirements in manifest.

### 4. Error Message Variations

**Issue**: Error messages may change with assembler improvements.

**Solution**: Use regex patterns in `.expected_error` files rather than exact strings.

### 5. Hexadecimal vs Decimal

**Issue**: Examples use different number formats.

**Solution**: Both should work; test as-is.

## Timeline Estimate

| Phase | Tasks | Estimated Time | Priority |
|-------|-------|----------------|----------|
| 1: Review | Catalogue all examples | 2-4 hours | HIGH |
| 2: Infrastructure | Build test framework | 4-6 hours | HIGH |
| 3: Extraction | Extract ~100 testable examples | 6-8 hours | HIGH |
| 4: Automation | CI setup, full test run | 2-3 hours | MEDIUM |
| 5: Maintenance | Sync scripts, documentation | 2-3 hours | LOW |
| **Total** | | **16-24 hours** | |

## Quick Start

### Minimum Viable Testing

To get started quickly with basic validation:

```bash
# 1. Create test directory
mkdir -p tests/documentation/testable/quick

# 2. Copy a few critical examples manually
# Example: Hello World from Syntax.md

cat > tests/documentation/testable/quick/hello_world.asm << 'EOF'
DATA
    message: DB "Hello, World!"

CODE
    LD AX, [message]
    HALT
EOF

# 3. Test manually
./build/pendragon_asm tests/documentation/testable/quick/hello_world.asm -o /tmp/test.bin
echo $?  # Should be 0 (success)

# 4. Verify binary exists and has content
ls -lh /tmp/test.bin
hexdump -C /tmp/test.bin | head
```

### Priority Examples to Test First

1. **Syntax.md** - Complete program example (line 365)
2. **Examples.md** - Hello World (line 32)
3. **Examples.md** - Basic Arithmetic (line 92)
4. **Examples.md** - Simple Loop (line 210)
5. **Registers.md** - Simple register usage (line 43)

These 5 examples cover the most critical functionality and should assemble successfully.

## Maintenance Schedule

- **Daily**: Run tests during development
- **Per Commit**: Automated CI testing
- **Weekly**: Review failed tests, update expected results
- **Per Release**: Full test suite validation
- **When Docs Updated**: Extract new examples, add to suite

## Future Enhancements

### 1. VM Integration
When VM is implemented:
- Run binaries to completion
- Validate final register/memory state
- Check for runtime errors
- Performance benchmarking

### 2. Visual Test Reports
Generate HTML reports with:
- Pass/fail summary
- Failed test details with diffs
- Source code highlighting
- Binary hex dumps

### 3. Fuzzing
- Generate variations of examples
- Test edge cases
- Stress test assembler

### 4. Performance Tests
- Track assembly time for complex examples
- Detect performance regressions
- Binary size optimization

## See Also

- **[Overview](Reference/Overview.md)**: Documentation entry point
- **[Examples](Reference/Examples.md)**: Main source of test programs
- **CMakeLists.txt**: Build configuration
- **README.md**: Project overview

## Contact

For questions about documentation testing:
- Review this plan
- Check existing test infrastructure
- Run initial manual tests to understand flow
