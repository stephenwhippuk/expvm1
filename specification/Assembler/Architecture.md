# Pendragon Assembler Architecture

## Design Overview

The assembler is designed as a multi-pass system with clear phase separation and extensibility in mind.

## Pass Structure

### Pass 1: Lexical Analysis & Parsing
- **Input**: Source `.asm` file
- **Output**: Abstract Syntax Tree (AST)
- **Purpose**: 
  - Tokenize input
  - Validate syntax
  - Build hierarchical representation
  - Catch syntax errors early

### Pass 2: Semantic Analysis & Symbol Resolution
- **Input**: AST
- **Output**: Symbol table + validated AST
- **Purpose**:
  - Build symbol table (labels, data definitions)
  - Validate instruction operands
  - Check for undefined references (first pass)
  - Type checking

### Pass 3: Code Graph Construction
- **Input**: Validated AST + Symbol table
- **Output**: Code Graph
- **Purpose**:
  - Create intermediate representation
  - Separate DATA and CODE sections
  - Handle anonymous data blocks
  - Build control flow information

### Pass 4: Address Resolution
- **Input**: Code Graph + Symbol table
- **Output**: Updated symbol table with absolute addresses
- **Purpose**:
  - Calculate data segment layout
  - Calculate code segment layout
  - Resolve all label addresses
  - Calculate offset expressions

### Pass 5: Code Generation
- **Input**: Code Graph + resolved symbol table
- **Output**: Binary file
- **Purpose**:
  - Generate machine code from instructions
  - Emit data segment
  - Emit code segment
  - Create binary file header

## Key Design Principles

### 1. Extensibility
- **Plugin-based instruction handlers**: Each instruction type can have custom handler
- **Extensible token types**: New syntax can be added without rewriting parser
- **Visitor pattern for AST traversal**: New operations can be added easily
- **Strategy pattern for code generation**: Different output formats possible

### 2. Error Handling
- **Phase-specific error reporting**: Each pass reports appropriate errors
- **Error recovery**: Continue parsing to find multiple errors
- **Rich error messages**: Include line numbers, context, suggestions

### 3. Separation of Concerns
- **Lexer**: Only knows about tokens
- **Parser**: Only knows about grammar rules
- **Semantic Analyzer**: Only knows about meaning/validity
- **Code Generator**: Only knows about binary format

## Core Components

### Lexer (Tokenizer)
```
Token types:
- KEYWORD (DATA, CODE, DB, DW, etc.)
- IDENTIFIER (labels, register names)
- NUMBER (hex, decimal)
- STRING (quoted strings)
- OPERATOR ([, ], +, etc.)
- SEPARATOR (comma, colon)
- COMMENT
```

### Parser
```
Grammar rules for:
- Sections (DATA, CODE)
- Labels (LABEL:)
- Data definitions (DB, DW)
- Instructions (opcode + operands)
- Expressions (offsets, array access)
```

### AST Nodes
```
- ProgramNode (root)
  - DataSectionNode
    - DataDefinitionNode
  - CodeSectionNode
    - LabelNode
    - InstructionNode
    - InlineDataNode
```

### Symbol Table
```
Entries:
- Name
- Type (label, data, anonymous)
- Section (DATA, CODE)
- Address (resolved in pass 4)
- Size (for data blocks)
- References (where it's used)
```

### Code Graph
```
Nodes:
- DataBlockNode (with address, size, content)
- InstructionNode (with opcode, operands, address)
- LabelNode (with address)

Edges:
- Data dependencies
- Control flow (jumps, calls)
```

## Extension Points

### Adding New Instructions
1. Add opcode definition to instruction set
2. Implement instruction handler (operand validation, encoding)
3. Register handler with instruction factory
4. No changes to core passes needed

### Adding New Syntactic Sugar
1. Add token types if needed
2. Update parser grammar rules
3. Add AST node type
4. Implement desugaring pass (AST → AST transformation)
5. Core passes remain unchanged

### Adding New Data Types
1. Add directive keyword (e.g., DD for double word)
2. Implement data definition handler
3. Register with data definition factory
4. Code graph automatically handles it

## File Structure

```
src/assembler/
├── lexer/
│   ├── token.h/cpp          - Token definitions
│   ├── lexer.h/cpp          - Tokenizer
│   └── token_type.h         - Token type enum
├── parser/
│   ├── ast.h/cpp            - AST node definitions
│   ├── parser.h/cpp         - Parser implementation
│   └── grammar.h            - Grammar rules
├── semantic/
│   ├── symbol_table.h/cpp   - Symbol management
│   ├── semantic_analyzer.h/cpp
│   └── type_checker.h/cpp
├── ir/
│   ├── code_graph.h/cpp     - Intermediate representation
│   ├── instruction_node.h/cpp
│   └── data_node.h/cpp
├── codegen/
│   ├── address_resolver.h/cpp
│   ├── encoder.h/cpp        - Instruction encoding
│   └── binary_writer.h/cpp  - Binary file generation
├── instructions/
│   ├── instruction_set.h    - Opcode definitions
│   ├── instruction_handler.h - Handler interface
│   └── handlers/            - Per-instruction implementations
├── extensions/
│   ├── desugar.h/cpp        - Syntactic sugar transformations
│   └── optimizations/       - Optional optimization passes
├── assembler.h/cpp          - Main assembler orchestration
└── errors.h/cpp             - Error reporting

tools/
└── asm.cpp                  - Command-line tool
```

## Usage Flow

```cpp
Assembler assembler;

// Configure
assembler.set_output_format(BinaryFormat::PENDRAGON_V1);
assembler.enable_optimizations(false);

// Assemble
try {
    assembler.assemble("program.asm", "program.bin");
} catch (const AssemblerError& e) {
    // Handle errors with line numbers
    std::cerr << e.what() << std::endl;
}
```

## Next Steps

1. Implement Token and Lexer
2. Implement AST nodes and Parser
3. Implement Symbol Table
4. Implement Code Graph
5. Implement Address Resolver
6. Implement Binary Writer
7. Add instruction handlers
8. Create command-line tool
9. Add tests for each pass
