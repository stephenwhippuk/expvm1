#include "semantic_analyzer.h"
#include <algorithm>
#include <cctype>

namespace lvm {
namespace assembler {

    SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbol_table)
        : symbol_table_(symbol_table)
        , in_data_section_(false)
        , in_code_section_(false) {
    }

    bool SemanticAnalyzer::analyze(ProgramNode& program) {
        errors_.clear();
        program.accept(*this);
        
        // Check for undefined symbols after full traversal
        auto undefined = symbol_table_.get_undefined_symbols();
        for (const auto& name : undefined) {
            const Symbol* symbol = symbol_table_.get(name);
            if (symbol && !symbol->references.empty()) {
                const auto& ref = symbol->references[0];
                error("Undefined symbol '" + name + "'", ref.line, ref.column);
            }
        }
        
        return !has_errors();
    }

    void SemanticAnalyzer::visit(ProgramNode& node) {
        for (auto& section : node.sections()) {
            section->accept(*this);
        }
    }

    void SemanticAnalyzer::visit(DataSectionNode& node) {
        in_data_section_ = true;
        in_code_section_ = false;
        
        for (auto& def : node.definitions()) {
            def->accept(*this);
        }
        
        in_data_section_ = false;
    }

    void SemanticAnalyzer::visit(CodeSectionNode& node) {
        in_data_section_ = false;
        in_code_section_ = true;
        
        for (auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        
        in_code_section_ = false;
    }

    void SemanticAnalyzer::visit(DataDefinitionNode& node) {
        // Define symbol in symbol table
        SymbolType type = (node.type() == DataDefinitionNode::Type::BYTE) 
                         ? SymbolType::DATA_BYTE 
                         : SymbolType::DATA_WORD;
        
        if (!symbol_table_.define(node.label(), type, node.line(), node.column())) {
            error("Duplicate definition of '" + node.label() + "'", 
                  node.line(), node.column());
        } else {
            // Set size
            uint32_t size = calculate_data_size(node);
            symbol_table_.set_size(node.label(), size);
        }
    }

    void SemanticAnalyzer::visit(LabelNode& node) {
        // Define label in symbol table
        if (!symbol_table_.define(node.name(), SymbolType::LABEL, 
                                  node.line(), node.column())) {
            error("Duplicate label '" + node.name() + "'", 
                  node.line(), node.column());
        }
    }

    void SemanticAnalyzer::visit(InstructionNode& node) {
        // Validate bracket syntax based on instruction semantics
        std::string mnemonic = node.mnemonic();
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);
        
        // Validate operands
        for (size_t i = 0; i < node.operands().size(); ++i) {
            auto& operand = node.operands()[i];
            
            // Check for bracket syntax violations
            if (mnemonic == "LD" && operand->type() == OperandNode::Type::MEMORY_ACCESS) {
                // LD with [expr] is only valid for sugar syntax (label[index])
                // which should be rewritten to LDA
                if (!operand->is_sugar_syntax()) {
                    error("LD instruction cannot use square brackets []. "
                          "Use LDA for memory access, or use parentheses () for address expressions.",
                          operand->line(), operand->column());
                }
            }
            
            operand->accept(*this);
        }
    }

    void SemanticAnalyzer::visit(OperandNode& node) {
        if (node.expression()) {
            node.expression()->accept(*this);
        }
        
        // Handle inline data operands
        if (node.type() == OperandNode::Type::INLINE_DATA && node.inline_data()) {
            node.inline_data()->accept(*this);
        }
    }

    void SemanticAnalyzer::visit(ExpressionNode& node) {
        switch (node.type()) {
            case ExpressionNode::Type::IDENTIFIER:
                // Reference to a symbol (label or data)
                validate_identifier_reference(node.identifier(), node.line(), 
                                             node.column(), "expression");
                break;
                
            case ExpressionNode::Type::REGISTER:
                // Validate register name
                validate_register(node.register_name(), node.line(), node.column());
                break;
                
            case ExpressionNode::Type::NUMBER:
                // Numbers are always valid
                break;
                
            case ExpressionNode::Type::BINARY_OP:
                // Recursively validate both sides
                if (node.left()) {
                    node.left()->accept(*this);
                }
                if (node.right()) {
                    node.right()->accept(*this);
                }
                
                // Validate operator
                if (node.op() != '+' && node.op() != '-') {
                    error("Invalid operator in expression", node.line(), node.column());
                }
                break;
        }
    }

    void SemanticAnalyzer::visit(InlineDataNode& node) {
        // Inline data creates an anonymous symbol
        // We'll handle this in the code graph construction pass
        // For now, just validate it's in the correct section
        if (!in_code_section_) {
            error("Inline data can only appear in CODE section", 
                  node.line(), node.column());
        }
    }

    void SemanticAnalyzer::error(const std::string& message, size_t line, size_t column) {
        errors_.emplace_back(message, line, column);
    }

    void SemanticAnalyzer::validate_register(const std::string& reg, 
                                            size_t line, size_t column) {
        // Valid registers: 16-bit: AX, BX, CX, DX, EX
        //                  8-bit: AH, AL, BH, BL, CH, CL, DH, DL, EH, EL (case insensitive)
        std::string upper = reg;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        // Check 16-bit registers
        if (upper == "AX" || upper == "BX" || upper == "CX" || 
            upper == "DX" || upper == "EX") {
            return;
        }
        
        // Check 8-bit sub-registers
        if (upper == "AH" || upper == "AL" || upper == "BH" || upper == "BL" ||
            upper == "CH" || upper == "CL" || upper == "DH" || upper == "DL" ||
            upper == "EH" || upper == "EL") {
            return;
        }
        
        error("Invalid register '" + reg + "'", line, column);
    }

    void SemanticAnalyzer::validate_identifier_reference(const std::string& name,
                                                         size_t line, size_t column,
                                                         const std::string& context) {
        // Add reference to symbol table
        symbol_table_.add_reference(name, line, column, context);
    }

    uint32_t SemanticAnalyzer::calculate_data_size(const DataDefinitionNode& node) {
        if (node.is_string()) {
            return static_cast<uint32_t>(node.string_data().length());
        } else {
            uint32_t element_size = (node.type() == DataDefinitionNode::Type::BYTE) ? 1 : 2;
            return static_cast<uint32_t>(node.numeric_data().size() * element_size);
        }
    }

    uint32_t SemanticAnalyzer::calculate_inline_data_size(const InlineDataNode& node) {
        if (node.is_string()) {
            return static_cast<uint32_t>(node.string_data().length());
        } else {
            uint32_t element_size = (node.type() == InlineDataNode::Type::BYTE) ? 1 : 2;
            return static_cast<uint32_t>(node.numeric_data().size() * element_size);
        }
    }

} // namespace assembler
} // namespace lvm
