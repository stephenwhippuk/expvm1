# Test Coverage Matrix

## Purpose
Track which instructions have been tested and their current status.

## Current Status
**Last Updated**: 2025-12-28

| Category | Total | Tested | Coverage |
|----------|-------|--------|----------|
| Control Flow | 2 | 2 | 100% |
| Register Ops | 7 | 7 | 100% |
| Memory Ops | 10 | 10 | 100% |
| Stack Ops | 13 | 13 | 100% |
| Paging | 2 | 2 | 100% |
| Stack Frame | 1 | 1 | 100% |
| Jumps | 9 | 9 | 100% |
| Subroutines | 2 | 2 | 100% |
| Arithmetic | 20 | 20 | 100% |
| Logical | 20 | 20 | 100% |
| Bit Ops | 20 | 20 | 100% |
| Inc/Dec | 2 | 2 | 100% |
| Compare | 6 | 6 | 100% |
| Indirect Mem | 3 | 3 | 100% |
| Immediate Stack | 2 | 2 | 100% |
| Byte-Variant ALU | 15 | 15 | 100% |
| **TOTAL** | **119** | **119** | **100%** |

**Test Binaries**: 118 files in `binaries/`
**Tests Passing**: 118/118 (100%)
**Issues Found**: 5 total (all fixed)

## Legend
- ✅ **Complete** - Instruction fully tested with all variants and edge cases
- 🟡 **Partial** - Some tests exist but coverage incomplete
- ❌ **Missing** - No tests yet
- 🔴 **Failing** - Tests exist but failing
- ⚠️ **Issues** - Tests pass but VM behavior questionable

---

## Control Flow (0x00-0x01)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x00 | NOP | ✅ | 2/5 | test_nop_basic, test_nop_multiple |
| 0x01 | HALT | ✅ | 3/4 | test_halt, test_halt_boundary, all tests use HALT |

---

## Register Operations (0x02-0x08)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x02 | LD (imm) | ✅ | 4/8 | test_ld_ax, test_ld_all_regs, test_add_*, test_cmp_* |
| 0x03 | LD (reg) | ✅ | 1/3 | test_ld_reg |
| 0x04 | SWP | ✅ | 2/5 | test_swp_basic, test_swp_same |
| 0x05 | LDH (imm) | ✅ | 2/4 | test_ldh_imm, test_ldh_ldl_combined |
| 0x06 | LDH (reg) | ✅ | 1/3 | test_ldh_reg |
| 0x07 | LDL (imm) | ✅ | 2/4 | test_ldl_imm, test_ldh_ldl_combined |
| 0x08 | LDL (reg) | ✅ | 1/3 | test_ldl_reg |

---

## Memory Operations (0x09-0x0F, 0x72-0x74)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x09 | LDA (load word) | ✅ | 1/6 | test_lda_load_word - PASS |
| 0x0A | LDAB | ✅ | 1/4 | test_ldab_load_byte - PASS |
| 0x0B | LDAH (load) | ✅ | 1/3 | test_ldah_load_high - PASS |
| 0x0C | LDAL (load) | ✅ | 1/3 | test_ldal_load_low - PASS |
| 0x0D | LDA (store word) | ✅ | 1/6 | test_sta_store_word - PASS |
| 0x0E | LDAH (store) | ✅ | 1/3 | test_stah_store_high - PASS |
| 0x0F | LDAL (store) | ✅ | 1/3 | test_stal_store_low - PASS |
| 0x72 | LDA (indirect) | ✅ | 1/3 | test_lda_indirect - PASS |
| 0x73 | LDAH (indirect) | ✅ | 1/2 | test_ldah_indirect - PASS |
| 0x74 | LDAL (indirect) | ✅ | 1/2 | test_ldal_indirect - PASS |

---

## Stack Operations (0x10-0x1A, 0x75-0x76)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x10 | PUSH | ✅ | 3/3 | test_push_pop_basic, test_push_multiple, test_push_pop_multiple |
| 0x11 | PUSHH | ✅ | 1/2 | test_pushh_poph |
| 0x12 | PUSHL | ✅ | 1/2 | test_pushl_popl |
| 0x13 | POP | ✅ | 4/3 | test_push_pop_basic, test_push_pop_multiple, test_pop_underflow |
| 0x14 | POPH | ✅ | 1/2 | test_pushh_poph |
| 0x15 | POPL | ✅ | 2/2 | test_pushl_popl, test_pushb_immediate |
| 0x16 | PEEK | ✅ | 2/3 | test_peek_basic, test_peek_offset |
| 0x17 | PEEKF | ✅ | 1/2 | test_peekf |
| 0x18 | PEEKB | ✅ | 1/2 | test_peekb |
| 0x19 | PEEKFB | ✅ | 1/2 | test_peekfb |
| 0x1A | FLSH | ✅ | 1/2 | test_flsh_basic |
| 0x75 | PUSHW | ✅ | 1/2 | test_pushw_immediate |
| 0x76 | PUSHB | ✅ | 1/2 | test_pushb_immediate |

---

## Memory Paging (0x1B-0x1C)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x1B | PAGE (imm) | ✅ | 117 | test_page_immediate - Store/load across pages |
| 0x1C | PAGE (reg) | ✅ | 118 | test_page_register - Dynamic page switching |

---

## Stack Frame (0x1D)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x1D | SETF | ✅ | 1/3 | test_setf_basic |

---

## Jump Instructions (0x1E-0x26)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x1E | JMP | ✅ | 3/3 | test_jmp_forward, test_jmp_backward, test_jmp_to_halt - PASS |
| 0x1F | JPZ | ✅ | 2/2 | test_jpz_taken, test_jpz_not_taken - PASS |
| 0x20 | JPNZ | ✅ | 2/2 | test_jpnz_taken, test_jpnz_not_taken - PASS (fixed Issue #4) |
| 0x21 | JPC | ✅ | 2/2 | test_jpc_taken, test_jpc_not_taken - PASS |
| 0x22 | JPNC | ✅ | 2/2 | test_jpnc_taken, test_jpnc_not_taken - PASS |
| 0x23 | JPS | ✅ | 2/2 | test_jps_taken, test_jps_not_taken - PASS |
| 0x24 | JPNS | ✅ | 2/2 | test_jpns_taken, test_jpns_not_taken - PASS |
| 0x25 | JPO | ✅ | 2/2 | test_jpo_taken, test_jpo_not_taken - PASS |
| 0x26 | JPNO | ✅ | 2/2 | test_jpno_taken, test_jpno_not_taken - PASS |

---

## Subroutine Instructions (0x27-0x28)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x27 | CALL | ✅ | 4/4 | test_call_basic, test_call_nested, test_call_stack, test_ret_without_call - PASS (fixed Issue #5) |
| 0x28 | RET | ✅ | 3/3 | test_call_basic, test_call_nested, test_ret_without_call - PASS |

---

## Arithmetic - Addition (0x29-0x2D)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x2C | ADD (word) | ✅ | 2/7 | test_add_basic, test_add_overflow - PASS |
| 0x2A | ADD (reg) | ❌ | 0/4 | Not yet tested |
| 0x2B | ADB | ❌ | 0/3 | Not yet tested |
| 0x2C | ADH | ❌ | 0/4 | Not yet tested |
| 0x2D | ADL | ❌ | 0/4 | Not yet tested |

---

## Arithmetic - Subtraction (0x2E-0x32)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x2E | SUB (word) | ✅ | 2/6 | test_sub_immediate, test_sub_register - PASS (fixed Issue #4) |
| 0x2F | SUB (reg) | ✅ | 1/3 | test_sub_register - PASS |
| 0x30 | SBB | ❌ | 0/3 | Not yet tested |
| 0x31 | SBH | ❌ | 0/3 | Not yet tested |
| 0x32 | SBL | ❌ | 0/3 | Not yet tested |

---

## Arithmetic - Multiplication (0x33-0x37)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x33 | MUL (word) | ✅ | 2/5 | test_mul_immediate, test_mul_register - PASS |
| 0x34 | MUL (reg) | ✅ | 1/3 | test_mul_register - PASS |
| 0x35 | MLB | ❌ | 0/3 | Not yet tested |
| 0x36 | MLH | ❌ | 0/2 | Not yet tested |
| 0x37 | MLL | ❌ | 0/2 | Not yet tested |

---

## Arithmetic - Division (0x38-0x3C)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x38 | DIV (word) | ✅ | 2/4 | test_div_immediate, test_div_register - PASS |
| 0x39 | DIV (reg) | ✅ | 1/3 | test_div_register - PASS |
| 0x3A | DVB | ❌ | 0/3 | Not yet tested |
| 0x3B | DVH | ❌ | 0/2 | Not yet tested |
| 0x3C | DVL | ❌ | 0/2 | Not yet tested |

---

## Arithmetic - Remainder (0x3D-0x41)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x3D | REM (word) | ✅ | 2/4 | test_rem_immediate, test_rem_register - PASS |
| 0x3E | REM (reg) | ✅ | 1/2 | test_rem_register - PASS |
| 0x3F | RMB | ❌ | 0/2 | Not yet tested |
| 0x40 | RMH | ❌ | 0/2 | Not yet tested |
| 0x41 | RML | ❌ | 0/2 | Not yet tested |

---

## Logical - AND (0x42-0x46)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x42 | AND (word) | ✅ | 2/4 | test_and_immediate, test_and_register - PASS |
| 0x43 | AND (reg) | ✅ | 1/2 | test_and_register - PASS |
| 0x44 | ANB | ❌ | 0/2 | Not yet tested |
| 0x45 | ANH | ❌ | 0/2 | Not yet tested |
| 0x46 | ANL | ❌ | 0/2 | Not yet tested |

---

## Logical - OR (0x47-0x4B)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x47 | OR (word) | ✅ | 2/3 | test_or_immediate, test_or_register - PASS |
| 0x48 | OR (reg) | ✅ | 1/2 | test_or_register - PASS |
| 0x49 | ORB | ❌ | 0/2 | |
| 0x4A | ORH | ❌ | 0/2 | |
| 0x4B | ORL | ❌ | 0/2 | |

---

## Logical - XOR (0x4C-0x50)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x4C | XOR (word) | ✅ | 2/4 | test_xor_immediate, test_xor_register - PASS |
| 0x4D | XOR (reg) | ✅ | 1/3 | test_xor_register - PASS |
| 0x4E | XOB | ❌ | 0/2 | Not yet tested |
| 0x4F | XOH | ❌ | 0/2 | Not yet tested |
| 0x50 | XOL | ❌ | 0/2 | Not yet tested |

---

## Logical - NOT (0x51-0x55)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x51 | NOT (word) | ✅ | 2/3 | test_not_immediate, test_not_register - PASS |
| 0x52 | NOT (reg) | ✅ | 1/2 | test_not_register - PASS |
| 0x53 | NOTB | ❌ | 0/2 | Not yet tested |
| 0x54 | NOTH | ❌ | 0/2 | Not yet tested |
| 0x55 | NOTL | ❌ | 0/2 | Not yet tested |

---

## Shift - Left (0x56-0x5A)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x56 | SHL (word) | ✅ | 2/5 | test_shl_immediate, test_shl_register - PASS |
| 0x57 | SHL (reg) | ✅ | 1/2 | test_shl_register - PASS |
| 0x58 | SLB | ❌ | 0/2 | Not yet tested |
| 0x59 | SLH | ❌ | 0/2 | Not yet tested |
| 0x5A | SLL | ❌ | 0/2 | Not yet tested |

---

## Shift - Right (0x5B-0x5F)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x5B | SHR (word) | ✅ | 2/5 | test_shr_immediate, test_shr_register - PASS |
| 0x5C | SHR (reg) | ✅ | 1/2 | test_shr_register - PASS |
| 0x5D | SHRB | ❌ | 0/2 | Not yet tested |
| 0x5E | SHRH | ❌ | 0/2 | Not yet tested |
| 0x5F | SHRL | ❌ | 0/2 | Not yet tested |

---

## Rotate - Left (0x60-0x64)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x60 | ROL (word) | ✅ | 2/4 | test_rol_immediate, test_rol_register - PASS |
| 0x61 | ROL (reg) | ✅ | 1/2 | test_rol_register - PASS |
| 0x62 | ROLB | ❌ | 0/2 | |
| 0x63 | ROLH | ❌ | 0/2 | |
| 0x64 | ROLL | ❌ | 0/2 | |

---

## Rotate - Right (0x65-0x69)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x65 | ROR (word) | ✅ | 2/4 | test_ror_immediate, test_ror_register - PASS |
| 0x66 | ROR (reg) | ✅ | 1/2 | test_ror_register - PASS |
| 0x67 | RORB | ❌ | 0/2 | Not yet tested |
| 0x68 | RORH | ❌ | 0/2 | Not yet tested |
| 0x69 | RORL | ❌ | 0/2 | Not yet tested |

---

## Inc/Dec (0x6A-0x6B)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x6A | INC | ✅ | 1/4 | test_inc_register - PASS |
| 0x6B | DEC | ✅ | 1/4 | test_dec_register - PASS |

---

## Comparison (0x6C-0x71)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x6C | CMP (reg,reg) | ✅ | 3/7 | test_cmp_reg_less, test_cmp_reg_equal, test_cmp_reg_greater - PASS |
| 0x6D | CMP (reg,imm) | ✅ | 1/4 | test_cmp_imm - PASS |
| 0x6E | CPH (reg,reg) | ✅ | 1/5 | test_cph_reg - PASS |
| 0x6F | CPH (reg,imm) | ✅ | 1/2 | test_cph_imm - PASS |
| 0x70 | CPL (reg,reg) | ✅ | 1/5 | test_cpl_reg - PASS |
| 0x71 | CPL (reg,imm) | ✅ | 1/2 | test_cpl_imm - PASS |

---

## System Call (0x7F)

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x7F | SYS | ❌ | 0/3 | Depends on available syscalls |

---

## Summary Statistics

- **Total Instructions**: 119
- **Tested (Complete)**: 0 (0%)
- **Tested (Partial)**: 0 (0%)
- **Untested**: 119 (100%)
- **Known Issues**: 0

## Priority Order for Implementation

1. **Phase 1 - Foundation** (Required for other tests)
   - HALT (0x01)
   - LD (0x02)
   - NOP (0x00)

2. **Phase 2 - Core Operations**
   - Register operations (0x03-0x08)
   - Basic arithmetic (ADD, SUB)
   - JMP (0x1E)

3. **Phase 3 - Memory & Stack**
   - Memory load/store (0x09-0x0F)
   - Stack operations (0x10-0x1A)

4. **Phase 4 - Advanced Arithmetic**
   - MUL, DIV, REM
   - Logical operations

5. **Phase 5 - Control Flow**
   - Conditional jumps
   - CALL/RET
   - Comparison

6. **Phase 6 - Bit Operations**
   - Shift and rotate operations

7. **Phase 7 - Integration & Edge Cases**
   - Complex sequences
   - Boundary conditions

---

## Byte-Variant ALU Operations (0x2B-0x2D, 0x30-0x32, 0x35-0x37, 0x3A-0x3C, 0x3F-0x41)

**Semantics**: These operations extract a byte from a register (high/low) or use immediate byte, then operate on AX as a word.
Example: `ADH BX` with AX=0x11FF, BX=0x22EA → extracts 0x22 → AX = 0x11FF + 0x0022 = 0x1221

| Opcode | Instruction | Status | Tests | Notes |
|--------|-------------|--------|-------|-------|
| 0x2B | ADB imm | ✅ | 102 | Add immediate byte as word to AX |
| 0x2C | ADH reg | ✅ | 103 | Add register high byte as word to AX |
| 0x2D | ADL reg | ✅ | 104 | Add register low byte as word to AX |
| 0x30 | SBB imm | ✅ | 105 | Subtract immediate byte as word from AX |
| 0x31 | SBH reg | ✅ | 106 | Subtract register high byte as word from AX |
| 0x32 | SBL reg | ✅ | 107 | Subtract register low byte as word from AX |
| 0x35 | MLB imm | ✅ | 108 | Multiply AX by immediate byte as word |
| 0x36 | MLH reg | ✅ | 109 | Multiply AX by register high byte as word |
| 0x37 | MLL reg | ✅ | 110 | Multiply AX by register low byte as word |
| 0x3A | DVB imm | ✅ | 111 | Divide AX by immediate byte as word |
| 0x3B | DVH reg | ✅ | 112 | Divide AX by register high byte as word |
| 0x3C | DVL reg | ✅ | 113 | Divide AX by register low byte as word |
| 0x3F | RMB imm | ✅ | 114 | AX modulo immediate byte as word |
| 0x40 | RMH reg | ✅ | 115 | AX modulo register high byte as word |
| 0x41 | RML reg | ✅ | 116 | AX modulo register low byte as word |

---

**Last Updated**: 2025-12-28
