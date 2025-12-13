# Pendragon VM v1.1 Roadmap

## Overview
Version 1.1 focuses on enhancing the VM with signed arithmetic support and addressing architectural issues discovered during v1.0 testing.

---

## 1. Signed Value Support

### 1.1 Problem Statement
**Current State (v1.0):**
- All arithmetic operations treat values as unsigned 16-bit integers
- CMP instruction performs unsigned comparison only
- No way to distinguish between -1 (0xFFFF) and 65535
- Overflow behavior doesn't account for signed interpretation

**Testing Evidence:**
- Test: `CMP 0x8000, 0x0100` (could be -32768 vs 256 signed, or 32768 vs 256 unsigned)
- Result: Returns 1 (greater than), confirming unsigned comparison
- Location: `tests/vm_behavior/comparison/FINDINGS.md`

### 1.2 Chosen Solution: CPU Mode Flag System

**Selected Implementation:** Generalized mode flag system via CPU instructions

**New Instructions:**
- `SMOD` - Set Mode (0x???)
  - Sets a specific mode flag to a state
  - Syntax: `SMOD <MODE>, <STATE>`
  - Operands:
    - MODE (1 byte): Which mode flag to set
      - 0x00: ALU_SIGNED - Controls signed/unsigned arithmetic
      - 0x01-0xFE: Reserved for future mode flags
    - STATE (1 byte): What state to set
      - 0x00: OFF/Unsigned/Disabled
      - 0x01: ON/Signed/Enabled
  - Example: `SMOD 0x00, 0x01` (Enable signed arithmetic)
  - Example: `SMOD 0x00, 0x00` (Disable signed arithmetic - unsigned mode)

- `GMOD` - Get Mode (0x???)
  - Retrieves current state of a mode flag into AX register
  - Syntax: `GMOD <MODE>`
  - Operands:
    - MODE (1 byte): Which mode flag to query
  - Returns: State value in AX (0x0000 or 0x0001)
  - Example: `GMOD 0x00` (Get ALU signed mode state into AX)

**Opcodes Required:** 2 new opcodes (SMOD, GMOD)

**Mode Flag Definitions (v1.1):**
- `MODE_ALU_SIGNED` = 0x00
  - State 0: Unsigned arithmetic (default, v1.0 compatible)
  - State 1: Signed arithmetic

**Future Mode Flags (Reserved):**
- `MODE_ALU_DWORD` = 0x01 (32-bit arithmetic, v1.2+)
- `MODE_ALU_FLOAT` = 0x02 (Floating point, v1.3+)
- `MODE_MEM_PROTECTED` = 0x03 (Enhanced memory protection)
- `MODE_DEBUG` = 0x04 (Debug/trace mode)
- 0x05-0xFE: Reserved for future expansion

**Affected Operations:**
When ALU_SIGNED mode is enabled (SMOD 0x00, 0x01), these instructions change behavior:
- `ADD`, `SUB`, `MUL`, `DIV`, `REM` - Treat operands as signed, set overflow correctly
- `CMP`, `CPH`, `CPL` - Perform signed comparison
- Overflow flag (O) - Set based on signed overflow rules

**Mode State:**
- Default state: All mode flags OFF (v1.0 compatibility)
- Mode flags persist across instructions until explicitly changed
- Mode flags are NOT saved/restored by function calls (programmer responsibility)
- Mode flag state is part of CPU context

**Advantages:**
- **Highly scalable** - Single instruction pair handles unlimited mode flags
- Minimal opcode usage (only 2 new instructions)
- Backward compatible - v1.0 programs run with default mode flags
- Flexible - can set multiple independent mode flags
- Clear programmer intent with explicit mode flag numbers
- **Foundation for diverse features** - Not just ALU modes, but any CPU behavioral flags
- Saves instruction space vs. dedicated instructions per feature

**Disadvantages:**
- Mode flag state is implicit after being set
- Programmers must track mode flags across control flow
- Function calls must document mode flag requirements/assumptions
- Requires memorizing mode flag numbers (0x00, 0x01, etc.)
- Risk of mode confusion if not carefully managed

**Mitigation Strategies:**
- Document best practices for mode flag management
- Recommend setting mode flags at function entry points
- Provide assembler constants for mode flag numbers (e.g., `MODE_ALU_SIGNED EQU 0x00`)
- Add mode flag display to debugger/disassembler output
- Use GMOD for defensive programming and assertions

### 1.3 Implementation Plan

**Phase 1: Design & Specification (Week 1)**
- [ ] Assign opcodes for SMOD and GMOD instructions
- [ ] Define mode flag system architecture in CPU
- [ ] Design extensible mode flag registry (256 possible flags)
- [ ] Document MODE_ALU_SIGNED (0x00) behavior specification
- [ ] Define overflow flag behavior for signed mode
- [ ] Update InstructionSet.md with SMOD/GMOD syntax and operands
- [ ] Create mode flag specification document
- [ ] Define best practices for mode flag management
- [ ] Plan mode flag value encoding (extensible for future flags)
- [ ] Define assembler constants for mode flag numbers

**Phase 2: ALU Enhancement (Week 2)**
- [ ] Add mode state to ALU class
- [ ] Implement signed arithmetic logic in ALU
- [ ] Add signed comparison methods
- [ ] Implement signed overflow detection
- [ ] Update ALU interface for mode queries
**Phase 3: CPU Implementation (Week 3)**
- [ ] Add mode flag registry to CPU context (256-flag array or bitset)
- [ ] Update opcode definitions in `opcodes.h` (SMOD, GMOD)
- [ ] Implement SMOD handler: parse MODE and STATE operands, update flag
- [ ] Implement GMOD handler: parse MODE operand, return state in AX
- [ ] Connect CPU MODE_ALU_SIGNED flag to ALU
- [ ] Modify existing ALU operation calls to check MODE_ALU_SIGNED flag
- [ ] Ensure mode flags persist correctly across instructions
- [ ] Add get_additional_bytes() entries for SMOD (2 bytes) and GMOD (1 byte)
- [ ] Modify existing ALU operation calls to respect mode
- [ ] Ensure mode persists correctly across instructions
**Phase 4: Assembler Support (Week 4)**
- [ ] Add SMOD and GMOD mnemonics to parser
- [ ] Add support for two-operand SMOD syntax
- [ ] Add support for one-operand GMOD syntax
- [ ] Define assembler constants: MODE_ALU_SIGNED, etc.
- [ ] Update `code_graph_builder.cpp` opcode mappings
- [ ] Add syntax validation for mode instructions
**Phase 5: Testing & Documentation (Week 5-6)**
- [ ] Create comprehensive signed arithmetic test suite
- [ ] Test mode flag setting and querying behavior
- [ ] Test edge cases: INT_MIN, INT_MAX, overflow
- [ ] Test mode flag persistence across jumps/calls
- [ ] Test invalid mode flag numbers (error handling)
- [ ] Update all specification documents
- [ ] Create programmer's guide for mode flag system
- [ ] Document mode flag management best practices
- [ ] Create migration examplesdocuments
**Phase 6: Integration & Release (Week 7)**
- [ ] Integration testing with existing v1.0 programs (default mode flags OFF)
- [ ] Performance testing (mode flag checking overhead)
- [ ] Update reference documentation
- [ ] Create release notes
- [ ] Tag v1.1.0 releaseg with existing v1.0 programs (unsigned mode default)
- [ ] Performance testing (mode switching overhead)
- [ ] Update reference documentation
- [ ] Create release notes
**Mode Switching:**
```assembly
; Test 1: Mode defaults to unsigned (flag OFF)
LD AX, 0xFFFF
LD BX, 0x0001
CMP AX, BX       ; Unsigned: 65535 > 1, returns 1

; Test 2: Verify default mode with GMOD
GMOD 0x00        ; Query MODE_ALU_SIGNED, AX = 0x0000 (OFF/unsigned)
CMP AX, 0x0000
; Should be equal

; Test 3: Enable signed mode
SMOD 0x00, 0x01  ; Set MODE_ALU_SIGNED to ON
GMOD 0x00        ; Verify: AX = 0x0001 (ON/signed)
LD AX, 0xFFFF    ; -1 signed
LD BX, 0x0001    ; +1
CMP AX, BX       ; Signed: -1 < +1, returns 0xFFFF (less than)

; Test 4: Return to unsigned mode
SMOD 0x00, 0x00  ; Set MODE_ALU_SIGNED to OFF
GMOD 0x00        ; Verify: AX = 0x0000 (OFF/unsigned)
LD AX, 0xFFFF
LD BX, 0x0001
CMP AX, BX       ; Unsigned: 65535 > 1, returns 1
```D             ; AX = 0x0000 (unsigned mode)
LD AX, 0xFFFF
LD BX, 0x0001
CMP AX, BX       ; Unsigned: 65535 > 1, returns 1
```
**Signed Comparison:**
```assembly
; Test 4: Boundary values
SMOD 0x00, 0x01  ; Enable signed mode
LD AX, 0x8000    ; -32768 (INT_MIN)
LD BX, 0x7FFF    ; +32767 (INT_MAX)
CMP AX, BX       ; Should return 0xFFFF (less than)

; Test 5: Equal negative values
LD AX, 0xFFFE    ; -2
LD BX, 0xFFFE    ; -2
CMP AX, BX       ; Should return 0 (equal)
``` AX, BX       ; Should return 0 (equal)
```
**Signed Arithmetic:**
```assembly
; Test 6: Negative addition
SMOD 0x00, 0x01  ; Enable signed mode
LD AX, 0xFFFF    ; -1
ADD AX, 0xFFFF   ; + (-1)
; Result: 0xFFFE (-2)

; Test 7: Overflow detection
LD AX, 0x7FFF    ; +32767
ADD AX, 1        ; + 1
; Result: 0x8000 (-32768), overflow flag set

; Test 8: Signed multiplication
LD AX, 0xFFFF    ; -1
MUL AX, 0x0003   ; * 3
; Result: 0xFFFD (-3)

; Test 9: Mode persists
ADD AX, 1        ; Still in signed mode
; Result: 0xFFFE (-2)
```esult: 0xFFFE (-2)
```

**Mode Management Best Practices:**
```assembly
; Test 10: Function mode discipline
my_function:
    PUSH AX
    PUSH BX
    GMOD 0x00        ; Save current ALU_SIGNED state
    PUSH AX
    
    SMOD 0x00, 0x01  ; Enable signed mode
    ; ... do signed arithmetic ...
    
    POP AX           ; Restore previous mode
    CMP AX, 0x0000
    JPE restore_unsigned
    SMOD 0x00, 0x01  ; Was signed, restore to signed
    JMP mode_restored
restore_unsigned:
    SMOD 0x00, 0x00  ; Was unsigned, restore to unsigned
mode_restored:
    POP BX
    POP AX
    RET

; Test 11: Defensive mode checking
critical_function:
    GMOD 0x00        ; Query MODE_ALU_SIGNED
    CMP AX, 0x0001   ; Verify we're in signed mode
    JPE mode_error   ; Jump if not equal (if unsigned)
    ; ... proceed with signed operations ...
    RET
mode_error:
    ; Handle unexpected mode
    HALT

; Test 12: Using assembler constants (cleaner syntax)
### 1.5 Documentation Updates Required

- [ ] `specification/Assembler/Reference/InstructionSet.md` - Add SMOD and GMOD documentation with operand syntax
- [ ] `specification/Reference/ALU.md` - Document MODE_ALU_SIGNED flag and signed arithmetic behavior
- [ ] `specification/Reference/CPU.md` - Document CPU mode flag registry system
- [ ] `specification/Reference/Flags.md` - Clarify overflow flag behavior in each mode
- [ ] `specification/PendragonOperations.md` - Add SMOD/GMOD to master operation list
- [ ] Create `specification/Reference/ModeFlagSystem.md` - Complete mode flag specification and extensibility
- [ ] Create `specification/Reference/SignedProgramming.md` - Best practices guide
- [ ] Update `README.md` - Highlight v1.1 mode flag system and signed support
- [ ] `specification/Assembler/Reference/InstructionSet.md` - Add SMOD, UMOD, and GMOD documentation
- [ ] `specification/Reference/ALU.md` - Document mode state and signed arithmetic behavior
**Guaranteed Compatibility:**
- All v1.0 programs will assemble and run identically
- CPU defaults to all mode flags OFF on startup
- No changes to existing opcodes or behavior
- Existing programs never encounter SMOD/GMOD instructions

**Migration Path:**
- Programs can adopt mode flags selectively where needed
- No "big bang" migration required
- Can freely toggle mode flags within a program
- Legacy functions continue to work with default mode flags

**Mode Management Requirements:**
- Programmers must explicitly set mode flags when special behavior needed
- Functions should document their mode flag assumptions
- Best practice: Save mode flag with GMOD, restore at exit
- Mode flags are NOT automatically saved/restored by CALL/RET
- Use GMOD for defensive programming and mode flag assertions

**Assembly Syntax:**
- Two new instructions: `SMOD <MODE>, <STATE>` and `GMOD <MODE>`
- Existing instructions (ADD, CMP, etc.) behave differently based on mode flags
- Assembler constants recommended for mode flag numbers
- Mode flag intention must be clear from contextre at exit
- Mode state is NOT automatically saved/restored by CALL/RET
- Use GMOD for defensive programming and mode assertions

**Assembly Syntax:**
- Three new instructions: `SMOD`, `UMOD`, and `GMOD`
- Existing instructions (ADD, CMP, etc.) behave differently based on mode
- GMOD returns mode value in AX (0=unsigned, 1=signed)
- Mode intention must be clear from context
my_function:
    PUSH AX
    SMOD             ; Set expected mode at entry
    ; ... do signed arithmetic ...
    UMOD             ; Restore to unsigned before return
    POP AX
    RET
```

### 1.5 Documentation Updates Required

- [ ] `specification/Assembler/Reference/InstructionSet.md` - Add signed instruction documentation
- [ ] `specification/Reference/ALU.md` - Document signed arithmetic behavior
- [ ] `specification/Reference/Flags.md` - Clarify overflow flag behavior
- [ ] `specification/PendragonOperations.md` - Add new operations to master list
- [ ] Create `specification/Reference/SignedArithmetic.md` - Comprehensive signed math guide
- [ ] Update `README.md` - Highlight v1.1 signed support

### 1.6 Backward Compatibility Considerations

**Guaranteed Compatibility:**
- All v1.0 programs will assemble and run identically
- No changes to existing opcodes or behavior
### 1.7 Implementation Details

**CPU Mode State:**
```cpp
class Cpu {
private:
    enum class ArithmeticMode {
        UNSIGNED = 0x00,  // Default, v1.0 compatible
        SIGNED   = 0x01,
        // Future extensions:
        // DWORD    = 0x02,  // 32-bit arithmetic (v1.2+)
        // FLOAT    = 0x03   // Floating point (v1.3+)
    };
    ArithmeticMode alu_mode_ = ArithmeticMode::UNSIGNED;
    
    // GMOD implementation
    void execute_gmod() {
        AX->set_value(static_cast<word_t>(alu_mode_));
    }
};
```

**ALU Interface Changes:**
```cpp
class ALU {
public:
    enum class Mode { 
        UNSIGNED = 0x00, 
        SIGNED   = 0x01,
        // Extensible for future:
        // DWORD = 0x02,
        // FLOAT = 0x03
    };
    
    void set_mode(Mode mode);
    Mode get_mode() const;
    word_t get_mode_value() const;  // For GMOD
    
    // Existing methods now mode-aware:
    void add(word_t value);      // Respects current mode
    void sub(word_t value);      // Respects current mode
    void mul(word_t value);      // Respects current mode
    void div(word_t value);      // Respects current mode
    word_t compare(word_t val);  // Respects current mode
};
```
### 2.1 Mode Flag System Extensibility

**Design Philosophy:**
The mode flag system introduced in v1.1 provides a generalized mechanism for controlling CPU and unit behavior. Instead of adding dedicated instructions for each feature, a single pair of instructions (SMOD/GMOD) manages up to 256 independent behavioral flags.

**Future Mode Flag Types:**

#### MODE_ALU_DWORD (0x01) - v1.2 Candidate
- **Purpose:** 32-bit arithmetic operations
- **Behavior Changes when enabled:**
  - ADD, SUB, MUL, DIV operate on register pairs (e.g., AX:BX as 32-bit value)
  - CMP performs 32-bit comparisons
  - Overflow flag set for 32-bit overflow
  - Load/Store operations work with double-words
- **Example:** `SMOD 0x01, 1` enables DWORD mode
- **Advantages:** 
  - Handles larger integers without multiple instructions
  - Better for address calculations in large memory spaces
  - Natural fit for 32-bit systems

#### MODE_ALU_FLOAT (0x02) - v1.3 Candidate
- **Purpose:** IEEE 754 floating-point arithmetic
- **Behavior Changes when enabled:**
  - Registers interpreted as 16-bit floats (half-precision)
  - Or register pairs as 32-bit floats (single-precision)
  - ADD, SUB, MUL, DIV use floating-point arithmetic
  - CMP performs floating-point comparison
  - New flags for NaN, Infinity detection
- **Example:** `SMOD 0x02, 1` enables float mode
- **Advantages:**
  - Scientific computing support
  - Graphics/game development capabilities
  - Financial calculations with decimals

#### MODE_MEM_PROTECTED (0x03) - Security Enhancement
- **Purpose:** Enhanced memory access control
- **Behavior Changes when enabled:**
  - Stricter page boundary enforcement
  - Read/write permission checking
  - Stack overflow protection
  - Invalid address detection
- **Example:** `SMOD 0x03, 1` enables protected memory mode

#### MODE_DEBUG (0x04) - Development Tool
- **Purpose:** Debugging and tracing support
- **Behavior Changes when enabled:**
  - Single-step execution
  - Instruction trace logging
  - Register change tracking
**Mode Flag System Benefits:**
1. **Opcode Conservation:** Two instructions (SMOD/GMOD) control 256 potential behaviors
2. **Independence:** Multiple flags can be active simultaneously without interference
3. **Clear Intent:** Flag number explicitly declared in code
4. **Testability:** GMOD allows verification of any flag state
5. **Flexibility:** Toggle flags as needed within a program
6. **Gradual Adoption:** Can add new flags without changing instruction set
7. **Orthogonality:** Different concerns (ALU mode, memory protection, debugging) separated

**Implementation Considerations:**
- Each flag controls independent behavior in CPU or units
- Multiple flags can be active: e.g., signed mode + debug mode + protected memory
- Flag space: 256 flags (0x00-0xFF) available
- Practical limit: ~20-30 flags before system becomes too complex
- Each flag should serve a distinct, justified purpose
- Flag interactions must be carefully designed to avoid conflicts

### 2.2 Other Potential Enhancements
- **Example:** `SMOD 0x04, 1` enables debug modet mode
4. **Flexibility:** Switch modes as needed within a program
5. **Gradual Adoption:** Can add new modes without changing existing ones

**Implementation Considerations:**
- Each new mode requires ALU logic updates
- Mode value space: 16 bits allows 65,536 potential modes
- Practical limit: ~8-10 modes before complexity becomes unmanageable
- Each mode should serve a distinct, justified purpose

### 2.2 Other Potential Enhancementssible):**
- 0x0000 - Unsigned mode (default)
- 0x0001 - Signed mode
- 0x0002 - Reserved for DWORD mode (future)
- 0x0003 - Reserved for Float mode (future)
- 0x0004-0xFFFF - Reserved for future expansion

---

## 2. Future Considerations (Post-v1.1)
- Programs can gradually adopt signed instructions where needed
- No "big bang" migration required
- Can mix signed and unsigned operations as appropriate

**Assembly Syntax:**
- New mnemonics clearly distinguished (CMPS vs CMP, ADDS vs ADD)
- No ambiguity in programmer intent

---

## 2. Future Considerations (Post-v1.1)

### 2.1 Potential Enhancements
- Floating-point arithmetic (v1.2?)
- 32-bit register extensions (v2.0?)
- Hardware interrupt support
- Memory protection enhancements
- DMA controller unit

### 2.2 Performance Optimizations
- JIT compilation support
- Instruction pipeline improvements
- Cache-aware memory access patterns

---

## Version History
- **v1.0** - Initial release, unsigned arithmetic only
- **v1.1** (Planned) - Signed arithmetic support
- **v1.2** (Future) - TBD

## References
- Current v1.0 specification: `specification/`
- Bug reports: `tests/vm_behavior/comparison/FINDINGS.md`
- Test suite: `tests/documentation/`

---

**Status:** DRAFT  
**Last Updated:** 2024-12-13  
**Owner:** Architecture Team
