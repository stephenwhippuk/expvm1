# Documentation Testing

This directory contains automated tests for all code examples in the assembler documentation.

## Quick Start

### Run All Tests

```bash
# From project root
tests/documentation/run_doc_tests.sh
```

### Run Quick Tests Only

```bash
tests/documentation/run_doc_tests.sh -c quick
```

### Verbose Output

```bash
tests/documentation/run_doc_tests.sh -v
```

## Directory Structure

```
tests/documentation/
├── README.md                 # This file
├── run_doc_tests.sh         # Main test script
├── testable/                # Valid examples that should assemble
│   ├── quick/               # Quick smoke tests (5 examples)
│   ├── syntax/              # From Syntax.md (TODO)
│   ├── registers/           # From Registers.md (TODO)
│   ├── instructions/        # From InstructionSet.md (TODO)
│   ├── expressions/         # From Expressions.md (TODO)
│   └── examples/            # From Examples.md (TODO)
├── error_cases/             # Invalid examples (TODO)
│   ├── lexer_errors/
│   ├── parser_errors/
│   └── semantic_errors/
└── results/                 # Test output (generated)
    ├── *.bin                # Generated binaries
    └── *.err                # Error logs
```

## Current Status

### Implemented ✅
- Test framework with bash script
- Directory structure
- 5 quick smoke tests covering:
  - Hello World
  - Basic arithmetic
  - Loops with counters
  - Register operations

### TODO 📋
- Extract remaining ~184 examples from documentation
- Add error case tests
- Create validation script for binary format
- Add CI integration
- Create HTML test reports

## Test Files

### Quick Tests (`testable/quick/`)

| File | Source | Description | Status |
|------|--------|-------------|--------|
| `01_hello_world.asm` | Examples.md:32 | Basic program structure | ✅ Ready |
| `02_basic_arithmetic.asm` | Examples.md:92 | Arithmetic operations | ✅ Ready |
| `03_countdown_loop.asm` | Examples.md:210 | Loop with counter | ✅ Ready |
| `04_sum_loop.asm` | Examples.md:234 | Sum 1 to 10 | ✅ Ready |
| `05_register_basic.asm` | Registers.md:337 | Register usage | ✅ Ready |

## Usage

### Run Tests

```bash
# All tests
./run_doc_tests.sh

# Specific category
./run_doc_tests.sh --category quick

# Verbose mode
./run_doc_tests.sh --verbose

# Stop on first failure
./run_doc_tests.sh --stop-on-fail

# Combine options
./run_doc_tests.sh -v -c quick -s
```

### Add New Test

1. Create `.asm` file in appropriate category directory
2. Add header comment with metadata:
```assembly
; ============================================
; TEST: test_name
; SOURCE: DocumentName.md (line XX)
; DESCRIPTION: What this tests
; EXPECTED: Expected outcome
; ============================================
```
3. Run tests to verify

### Interpreting Results

**Pass (✓)**: Example assembled successfully and generated binary  
**Fail (✗)**: Assembly failed - check error log in `results/`

Example output:
```
======================================
Documentation Example Tests
======================================
Assembler: /path/to/pendragon_asm
Test dir:  tests/documentation/testable

Found 5 test file(s)
======================================
quick/01_hello_world.asm                           ✓ PASS
quick/02_basic_arithmetic.asm                      ✓ PASS
quick/03_countdown_loop.asm                        ✓ PASS
quick/04_sum_loop.asm                              ✓ PASS
quick/05_register_basic.asm                        ✓ PASS

======================================
Test Results:
======================================
  Passed:  5
  Failed:  0
  Total:   5
======================================

All tests passed!
```

## Troubleshooting

### Assembler Not Found

**Error**: `Error: Assembler not found at build/pendragon_asm`

**Solution**: Build the project first:
```bash
mkdir -p build
cd build
cmake ..
make
```

### Test Fails

1. Check error log: `cat results/<testname>.err`
2. Try assembling manually:
```bash
./build/pendragon_asm tests/documentation/testable/quick/01_hello_world.asm -o /tmp/test.bin
```
3. Check if example in documentation is correct
4. Update test if documentation changed

### No Tests Found

**Error**: `No test files found in tests/documentation/testable/xyz`

**Solution**: Check category name, or create `.asm` files in that directory

## Next Steps

See `specification/Assembler/DocumentationTestingPlan.md` for:
- Complete testing strategy
- Extraction process for remaining examples
- Binary validation framework
- CI integration plan
- Maintenance procedures

## Contributing

When adding new documentation examples:
1. Extract example to appropriate test category
2. Add header with metadata
3. Run tests to verify
4. Commit both documentation and test together

When fixing documentation bugs:
1. Update documentation
2. Update corresponding test file
3. Run tests to verify
4. Update expected results if needed
