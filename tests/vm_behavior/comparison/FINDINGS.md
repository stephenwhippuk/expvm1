# CMP Instruction Behavior - Findings

## Test Date
2024 - v1.0 VM

## Summary
The CMP instruction treats all values as **UNSIGNED 16-bit integers**.

## Evidence

### Test 1: Boundary Test (0x8000 vs 0x0100)
**Test Code:**
```assembly
LD BX, 0x8000    ; 32768 unsigned / -32768 signed
LD CX, 0x0100    ; 256 unsigned / 256 signed
CMP BX, CX       ; Compare BX to CX
PUSH AX          ; Push result
SYS 0x1500       ; Print result
HALT
```

**Result:** Printed `1`

**Interpretation:**
- If signed: -32768 < 256 → would return 0xFFFF (-1)
- If unsigned: 32768 > 256 → returns 1 ✓

**Conclusion:** CMP performs UNSIGNED comparison

## Implications for v1.1 Design

1. Need signed comparison instructions (CMPS or similar)
2. Or document that CMP is unsigned and provide guidance
3. Or keep as-is and document clearly that all comparisons are unsigned

## Technical Notes

- CMP sets AX to:
  - 0 if operands are equal
  - 1 if first operand > second operand  
  - 0xFFFF if first operand < second operand
- This makes sense for unsigned: 0xFFFF is the largest 16-bit value
- For signed, -1 (0xFFFF) meaning "less than" is counterintuitive

## Bug Fixes Required During Testing

### Bug 1: Assembler Immediate Value Optimization
**Problem:** Assembler optimized immediates to 1 byte when value ≤255, but CPU expected fixed 2-byte operands for word instructions.

**Fix:** Modified `code_graph_builder.cpp` to use instruction semantics instead of value-based optimization:
- Added `instruction_expects_word_immediate()` helper
- Added `instruction_expects_byte_immediate()` helper  
- Modified `visit(OperandNode)` to check instruction type first

### Bug 2: Missing Register Operand Encoding
**Problem:** `CodeInstructionNode::encode()` didn't write register operands to bytecode (comment said "handled by opcode selection" but nothing handled it).

**Fix:** Modified `code_graph.cpp`:
- Added `register_name_to_code()` helper function
- Updated `size()` to count registers as 1 byte each
- Updated `encode()` to write register byte codes

### Bug 3: Register Code Mismatch  
**Problem:** Documentation said AX=0, BX=1, etc., but CPU expected AX=1, BX=2, etc.

**Fix:** Updated `register_name_to_code()` to use 1-based codes matching CPU's expectations.

## Files Modified
- `src/assembler/ir/code_graph_builder.h` - Added instruction context tracking
- `src/assembler/ir/code_graph_builder.cpp` - Fixed operand type determination
- `src/assembler/ir/code_graph.cpp` - Fixed register encoding
- `src/instruction_unit/include/systemcalls.h` - Added SYSCALL_DEBUG_PRINT_WORD 0x1500
- `src/instruction_unit/instruction_unit.cpp` - Added debug syscall handler
- `src/basic_io/include/basic_io.h` - Added debug_print_word() method
- `src/basic_io/include/basic_io_accessor.h` - Added debug_print_word() method  
- `src/basic_io/basic_io.cpp` - Implemented debug_print_word()
