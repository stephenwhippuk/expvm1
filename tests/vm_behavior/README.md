# VM Behavior Tests

## Purpose

These tests verify the actual runtime behavior of the VM, particularly for edge cases and design validation for future versions.

## Test Categories

### Comparison Tests (`comparison/`)
Tests for CMP instruction behavior with:
- Positive numbers
- Negative numbers (two's complement)
- Zero
- Edge cases (max positive, max negative)
- Signed vs unsigned interpretation

## Running Tests

These tests require the VM (lvm) to execute and verify results:

```bash
# Assemble test
build/asm tests/vm_behavior/comparison/test_name.asm -o /tmp/test.bin

# Run in VM
build/lvm /tmp/test.bin 0x0000

# Check results (register values, memory, etc.)
```

## Expected Results

Each test file documents expected register values after execution.

## Version Notes

These tests are for design validation and may inform changes in version 1.1+.
Current behavior is NOT modified - tests document actual behavior.
