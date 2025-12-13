# Assembler Bracket Syntax Correction Plan

## ✅ STATUS: COMPLETE

**Completion Date**: December 2024  
**Test Status**: All 166 tests passing  
**Documentation**: Updated (see DOCUMENTATION_UPDATE_SUMMARY.md)

---

## Problem Statement (RESOLVED)

The assembler has drifted significantly from the specification in `specifications/UseOfBrackets.md`. The current implementation treats brackets `[]` and parentheses `()` interchangeably for memory access, but the specification defines distinct semantic meanings.

**Resolution**: All issues corrected. Implementation now matches specification exactly.

---

## Current vs. Specified Behavior

### Current Implementation (INCORRECT)
```assembly
LD AX, [LABEL]      ; Parser: MEMORY_DIRECT (dereference)
LD AX, (LABEL)      ; Parser: MEMORY_DIRECT (dereference) - SAME AS ABOVE
LD AX, LABEL        ; Parser: IDENTIFIER (address loading)
LDA AX, LABEL       ; Parser: IDENTIFIER (memory access)
LDA AX, [LABEL]     ; Parser: MEMORY_DIRECT
```

**Problem:** Brackets and parentheses are treated identically - both as memory dereferencing.

### Specified Behavior (CORRECT per UseOfBrackets.md)

```assembly
; LD instruction semantics:
LD AX, 1            ; Immediate value → register
LD AX, BX           ; Register → register  
LD AX, LABEL        ; Address of LABEL → register (load address)
LD AX, (LABEL + 1)  ; (Address of LABEL + 1) → register (address arithmetic)
LD AX, (LABEL + BX) ; (Address of LABEL + BX) → register (address arithmetic)

; LDA instruction semantics:
LDA AX, LABEL       ; Value AT LABEL → register (memory dereference)
LDA AX, (LABEL + 1) ; Value AT (LABEL + 1) → register (memory dereference with offset)
LDA AX, BX          ; Value AT address in BX → register (indirect addressing)
LDA AX, (LABEL + BX) ; Value AT (LABEL + BX) → register (computed address)
LDA LABEL, AX       ; Store AX → memory AT LABEL
LDA (LABEL + 1), AX ; Store AX → memory AT (LABEL + 1)

; Syntactic sugar with square brackets:
LD AX, label[1]     ; Sugar for: LDA AX, (label + 1)
LD AX, label[BX]    ; Sugar for: LDA AX, (label + BX)
LD AX, label[BX + 1] ; Sugar for: LDA AX, (label + BX + 1)
```

**Key Distinction:**
- **Parentheses `()`**: Address arithmetic - compute an address
- **Square brackets `[]`**: Syntactic sugar for memory dereferencing - equivalent to using LDA
- **No brackets/parens**: Depends on instruction context
  - `LD`: Loads the address itself
  - `LDA`: Accesses memory at the address

---

## Semantic Rules (from specification)

### LD Instruction
1. `LD reg, immediate` → Load immediate value
2. `LD reg, register` → Copy register to register
3. `LD reg, LABEL` → Load address of LABEL into register
4. `LD reg, (expr)` → Evaluate address expression, load address into register
5. **DOES NOT support `LD reg, [expr]`** (use LDA for memory access)

### LDA Instruction  
1. `LDA reg, LABEL` → Load value FROM memory at LABEL
2. `LDA reg, (expr)` → Load value FROM memory at computed address
3. `LDA reg, register` → Load value FROM memory at address in register (indirect)
4. `LDA LABEL, reg` → Store register value TO memory at LABEL
5. `LDA (expr), reg` → Store register value TO memory at computed address

### Syntactic Sugar
- `LD reg, label[...]` → Automatically converts to `LDA reg, (label + ...)`
- Brackets `[]` only valid in this sugar syntax
- Brackets `[]` in any other context = syntax error (or conversion to LDA)

---

## Issues in Current Implementation

### Issue 1: Parser Treats `[]` and `()` Identically
**File:** `src/assembler/parser/parser.cpp:335-360`

```cpp
// Memory access: [expression] or (expression)
if (check(TokenType::LEFT_BRACKET) || check(TokenType::LEFT_PAREN)) {
    bool is_bracket = check(TokenType::LEFT_BRACKET);
    size_t line = current_.line, col = current_.column;
    advance();
    
    auto expr = parse_expression();
    
    if (is_bracket) {
        consume(TokenType::RIGHT_BRACKET, "Expected ']'");
    } else {
        consume(TokenType::RIGHT_PAREN, "Expected ')'");
    }
    
    auto operand = std::make_unique<OperandNode>(OperandNode::Type::MEMORY_DIRECT);
    // ^^^ PROBLEM: Both [] and () create MEMORY_DIRECT type
```

**Problem:** Both notations create the same AST node type, losing semantic distinction.

### Issue 2: No Bracket-to-Index Sugar Parsing
**Missing:** `label[expr]` syntax not recognized by lexer/parser

The specification says `LD AX, label[BX + 1]` should be sugar for `LDA AX, (label + BX + 1)`, but the parser doesn't support this syntax at all.

### Issue 3: Code Generation Doesn't Distinguish Semantics
**File:** `src/assembler/ir/code_graph_builder.cpp`

The code generator doesn't differentiate between:
- Address loading (LD with parentheses)
- Memory dereferencing (LDA)
- Bracket sugar conversion

### Issue 4: Test Files Use Incorrect Syntax
**Examples:**
- `tests/documentation/testable/quick/09_square_function.asm:14` uses `LD AX, [input]`
  - Should be: `LDA AX, input` or `LD AX, input[0]` (sugar)
- `tests/documentation/testable/quick/92_structure_access.asm:19` uses `LD AX, [person_age + 0]`
  - Should be: `LDA AX, (person_age + 0)` or `LD AX, person_age[0]` (sugar)

---

## Correction Plan

### Phase 1: AST and Parser Changes (Week 1)

#### 1.1 Update AST Node Types
**File:** `src/assembler/parser/ast.h`

Modify `OperandNode::Type` enum:
```cpp
enum class Type {
    IMMEDIATE,          // Literal number: 42, 0xFF
    REGISTER,           // Register name: AX, BX
    IDENTIFIER,         // Bare label: LABEL
    ADDRESS_EXPR,       // NEW: Parentheses (expr) - address computation
    MEMORY_ACCESS,      // NEW: [expr] or label[expr] - memory dereference
};
```

**Rationale:** Distinguish between address expressions `()` and memory access `[]`.

#### 1.2 Add Index Syntax Support to Lexer
**File:** `src/assembler/lexer/lexer.cpp`

Add recognition of `identifier[...]` pattern:
- When seeing `IDENTIFIER` followed immediately by `[`, treat as index syntax
- Parse as special token or handle in parser

#### 1.3 Modify Parser to Handle New Semantics
**File:** `src/assembler/parser/parser.cpp`

```cpp
std::unique_ptr<OperandNode> Parser::parse_operand() {
    // Check for identifier[expr] sugar syntax
    if (check(TokenType::IDENTIFIER)) {
        Token identifier = current_;
        advance();
        
        if (check(TokenType::LEFT_BRACKET)) {
            // Sugar: label[expr] → convert to LDA with (label + expr)
            advance(); // consume [
            auto index_expr = parse_expression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
            
            // Create memory access operand with combined expression
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::MEMORY_ACCESS);
            auto combined = create_addition_expr(identifier.lexeme, index_expr);
            operand->set_expression(std::move(combined));
            operand->set_sugar_syntax(true); // Mark for code gen
            return operand;
        } else {
            // Plain identifier
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::IDENTIFIER);
            // ... existing code
        }
    }
    
    // Parentheses: (expr) → address expression
    if (check(TokenType::LEFT_PAREN)) {
        advance();
        auto expr = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')'");
        
        auto operand = std::make_unique<OperandNode>(OperandNode::Type::ADDRESS_EXPR);
        operand->set_expression(std::move(expr));
        return operand;
    }
    
    // Square brackets: [expr] → memory access (may need instruction rewrite)
    if (check(TokenType::LEFT_BRACKET)) {
        advance();
        auto expr = parse_expression();
        consume(TokenType::RIGHT_BRACKET, "Expected ']'");
        
        auto operand = std::make_unique<OperandNode>(OperandNode::Type::MEMORY_ACCESS);
        operand->set_expression(std::move(expr));
        return operand;
    }
    
    // ... rest of operand parsing
}
```

### Phase 2: Semantic Analysis / Instruction Rewriting (Week 2)

#### 2.1 Add Instruction Semantic Validator
**New File:** `src/assembler/semantic/instruction_validator.cpp`

Create validation pass that checks operand types match instruction semantics:

```cpp
class InstructionValidator {
public:
    void validate(InstructionNode* instr) {
        std::string mnemonic = to_upper(instr->mnemonic());
        
        if (mnemonic == "LD") {
            validate_LD(instr);
        } else if (mnemonic == "LDA") {
            validate_LDA(instr);
        }
        // ... other instructions
    }
    
private:
    void validate_LD(InstructionNode* instr) {
        // LD supports: immediate, register, identifier, address_expr
        // LD does NOT support: memory_access
        
        for (auto& operand : instr->operands()) {
            if (operand->type() == OperandNode::Type::MEMORY_ACCESS) {
                // User wrote: LD AX, [label]
                // Should be: LDA AX, label  OR  LD AX, label[0]
                throw SemanticError(
                    "LD instruction cannot dereference memory with []. "
                    "Use LDA for memory access, or use parentheses () for address arithmetic."
                );
            }
        }
    }
    
    void validate_LDA(InstructionNode* instr) {
        // LDA expects memory access or address expressions
        // Bare identifiers in LDA context mean "memory at label"
    }
};
```

#### 2.2 Instruction Rewriting for Sugar Syntax
**New File:** `src/assembler/semantic/instruction_rewriter.cpp`

Handle `LD reg, label[expr]` → convert to `LDA reg, (label + expr)`:

```cpp
class InstructionRewriter {
public:
    void rewrite_sugar(InstructionNode* instr) {
        if (to_upper(instr->mnemonic()) == "LD") {
            for (size_t i = 0; i < instr->operands().size(); i++) {
                auto& operand = instr->operands()[i];
                
                if (operand->type() == OperandNode::Type::MEMORY_ACCESS && 
                    operand->is_sugar_syntax()) {
                    // Rewrite: LD → LDA
                    instr->set_mnemonic("LDA");
                    // Convert MEMORY_ACCESS → ADDRESS_EXPR
                    operand->set_type(OperandNode::Type::ADDRESS_EXPR);
                    break;
                }
            }
        }
    }
};
```

### Phase 3: Code Generation Updates (Week 2-3)

#### 3.1 Update Code Graph Builder
**File:** `src/assembler/ir/code_graph_builder.cpp`

```cpp
void CodeGraphBuilder::visit(OperandNode& node) {
    switch (node.type()) {
        case OperandNode::Type::IDENTIFIER:
            // Bare label - meaning depends on instruction
            if (current_instruction_mnemonic_ == "LD") {
                // LD AX, LABEL → load address of label
                current_operand_.type = InstructionOperand::Type::ADDRESS;
                current_operand_.symbol_name = node.expression()->identifier();
            } else if (current_instruction_mnemonic_ == "LDA") {
                // LDA AX, LABEL → load value from memory at label
                current_operand_.type = InstructionOperand::Type::ADDRESS;
                current_operand_.symbol_name = node.expression()->identifier();
            }
            break;
            
        case OperandNode::Type::ADDRESS_EXPR:
            // Parentheses: (label + offset)
            // For LD: compute address, load address value
            // For LDA: compute address, load memory at address
            current_operand_.type = InstructionOperand::Type::EXPRESSION;
            // ... parse expression components
            break;
            
        case OperandNode::Type::MEMORY_ACCESS:
            // Square brackets: [expr]
            // Should only appear after sugar rewriting or in LDA context
            current_operand_.type = InstructionOperand::Type::EXPRESSION;
            // ... parse expression components
            break;
    }
}
```

#### 3.2 Handle Complex Expressions
For expressions like `(LABEL + BX + 1)` that can't be encoded in a single instruction, implement multi-instruction sequences:

```cpp
// Pseudo-algorithm:
if (expression_has_register_and_constant(expr)) {
    // (label + BX + 1) becomes:
    // 1. LD AX, (label + 1)    ; resolve label + constant
    // 2. ADD AX, BX             ; add register offset
    // 3. LDA AX, AX             ; load from computed address (if needed)
    
    emit_instruction(LD, AX, resolve_label_plus_constant(expr));
    emit_instruction(ADD, AX, extract_register(expr));
    if (needs_dereference) {
        emit_instruction(LDA, dest_reg, AX);
    }
}
```

### Phase 4: Documentation Updates (Week 3)

#### 4.1 Update Syntax Documentation
**Files to update:**
- `specification/Assembler/Reference/Syntax.md` - Fix operand type descriptions
- `specification/Assembler/Reference/Expressions.md` - Document `()` vs `[]` distinction
- `specification/Assembler/Reference/InstructionSet.md` - Update LD and LDA examples

#### 4.2 Update Examples
Add clear examples showing:
```assembly
; Address loading (LD)
LD AX, buffer           ; Load address of buffer → AX
LD BX, (buffer + 10)    ; Load address (buffer+10) → BX

; Memory access (LDA)
LDA AX, buffer          ; Load value from buffer → AX
LDA BX, (buffer + 10)   ; Load value from (buffer+10) → BX

; Syntactic sugar
LD AX, buffer[10]       ; Sugar → LDA AX, (buffer + 10)
LD BX, buffer[CX]       ; Sugar → LDA BX, (buffer + CX)
```

### Phase 5: Test Corrections (Week 4)

#### 5.1 Fix Existing Tests
Audit and fix all 166 documentation tests:

**Pattern 1:** `LD AX, [label]` → Should be `LDA AX, label`
```bash
# Find all cases:
grep -r "LD.*\[" tests/documentation/testable/
```

**Pattern 2:** `LD AX, [label + offset]` → Should be `LDA AX, (label + offset)`

**Pattern 3:** Convert appropriate cases to sugar syntax for testing

#### 5.2 Add New Tests
Create tests specifically for:
- `test_bracket_semantics.asm` - Test `()` vs `[]` distinction
- `test_sugar_syntax.asm` - Test `label[expr]` → LDA conversion
- `test_address_loading.asm` - Test LD with parentheses
- `test_complex_expressions.asm` - Test multi-instruction sequences

---

## Implementation Checklist

### Week 1: Parser & AST ✅ COMPLETE
- [x] Update `OperandNode::Type` enum with ADDRESS_EXPR and MEMORY_ACCESS
- [x] Add sugar syntax field to OperandNode
- [x] Modify lexer to recognize `identifier[` pattern
- [x] Update `parse_operand()` to handle three cases: `()`, `[]`, `label[]`
- [x] Update parser tests
- [x] Add AST visitor updates

### Week 2: Semantics & Code Gen ✅ COMPLETE
- [x] Create InstructionValidator class
- [x] Implement LD semantic validation
- [x] Implement LDA semantic validation
- [x] Create InstructionRewriter for sugar syntax
- [x] Update CodeGraphBuilder::visit(OperandNode)
- [x] Implement complex expression handling
- [x] Add semantic analysis tests

### Week 3: Documentation ✅ COMPLETE
- [x] Update Syntax.md with correct bracket semantics
- [x] Update Expressions.md with `()` vs `[]` rules
- [x] Update InstructionSet.md examples (verified not needed)
- [x] Add sugar syntax documentation
- [x] Create documentation summary

### Week 4: Tests & Validation ✅ COMPLETE
- [x] Audit all 166 tests for incorrect bracket usage
- [x] Fix test files to match specification (53 files)
- [x] Add new bracket semantic tests
- [x] Add sugar syntax tests
- [x] Verify all tests pass with correct syntax (166/166)
- [x] Update PROGRESS.md with completion status

---

## Risk Assessment

**High Risk Areas:**
1. **Breaking Changes**: Fixing tests will temporarily break 20+ existing tests
2. **Complex Expressions**: Multi-instruction generation is complex and error-prone
3. **Semantic Ambiguity**: Some edge cases may be underspecified

**Mitigation:**
- Create comprehensive test suite before changing existing tests
- Document all semantic decisions clearly
- Provide clear error messages for invalid syntax
- This is prerelease, so breaking changes are acceptable for correctness

---

## Success Criteria ✅ ALL ACHIEVED

1. ✅ Parser correctly distinguishes `()`, `[]`, and `label[]` syntax
2. ✅ LD instruction only accepts address operands (no memory dereference)
3. ✅ LDA instruction handles memory access correctly
4. ✅ Sugar syntax `label[expr]` converts to `LDA (label + expr)`
5. ✅ All 166 documentation tests pass with corrected syntax
6. ✅ New bracket semantic tests pass
7. ✅ Documentation accurately reflects implementation (Syntax.md, Expressions.md)
8. ✅ Clear error messages for invalid bracket usage

---

## Implementation Summary

**Timeline**: Completed ahead of schedule (4-week plan completed in 2 days)

**Files Modified**:
- Parser: `src/assembler/parser/parser.cpp`, `src/assembler/parser/ast.h`
- Semantic: `src/assembler/semantic/semantic_analyzer.cpp`
- New: `src/assembler/semantic/instruction_rewriter.h/.cpp`
- Code Gen: `src/assembler/ir/code_graph_builder.cpp`
- Tools: `tools/asm.cpp`, `tools/test_parser.cpp`
- Tests: 53 test files in `tests/documentation/testable/quick/`
- Documentation: `Syntax.md` (734 lines), `Expressions.md` (917 lines)

**Test Results**: 166/166 passing

**Documentation Created**:
- `DOCUMENTATION_UPDATE_SUMMARY.md` - Detailed changes to documentation
- This file updated with completion status

---

## Timeline Summary

- **Week 1**: Parser & AST changes ✅ COMPLETE
- **Week 2**: Semantic analysis & code generation ✅ COMPLETE
- **Week 3**: Documentation updates
- **Week 4**: Test fixes & validation
- **Total**: ~4 weeks for complete implementation

---

## Notes

This is a **major architectural correction** that touches parser, semantic analysis, code generation, documentation, and tests. The specification in `UseOfBrackets.md` is clear, but the implementation has diverged significantly. This plan provides a systematic approach to bring the assembler back into compliance with the specification while maintaining code quality and test coverage.
