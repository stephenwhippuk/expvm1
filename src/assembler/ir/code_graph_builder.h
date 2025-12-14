#pragma once

#include "code_graph.h"
#include "../parser/ast.h"
#include "../semantic/symbol_table.h"
#include <string>
#include <vector>
#include <memory>

namespace lvm {
namespace assembler {

    /**
     * Code graph builder error
     */
    struct CodeGraphError {
        std::string message;
        size_t line;
        size_t column;
        
        CodeGraphError(const std::string& msg, size_t ln, size_t col)
            : message(msg), line(ln), column(col) {}
            
        std::string to_string() const {
            return "Code graph error at " + std::to_string(line) + ":" + 
                   std::to_string(column) + ": " + message;
        }
    };

    /**
     * Code graph builder (Pass 3)
     * 
     * Converts AST to intermediate representation:
     * - Creates data blocks for DATA section
     * - Creates instruction nodes for CODE section
     * - Handles inline data blocks
     * - Prepares operands for address resolution
     */
    class CodeGraphBuilder : public ASTVisitor {
    public:
        explicit CodeGraphBuilder(SymbolTable& symbol_table);
        
        /**
         * Build code graph from AST
         * @return Code graph or nullptr if errors
         */
        std::unique_ptr<CodeGraph> build(ProgramNode& program);
        
        /**
         * Get errors
         */
        const std::vector<CodeGraphError>& errors() const { return errors_; }
        bool has_errors() const { return !errors_.empty(); }
        
        // Visitor methods
        void visit(ProgramNode& node) override;
        void visit(DataSectionNode& node) override;
        void visit(CodeSectionNode& node) override;
        void visit(DataDefinitionNode& node) override;
        void visit(LabelNode& node) override;
        void visit(InstructionNode& node) override;
        void visit(OperandNode& node) override;
        void visit(ExpressionNode& node) override;
        void visit(InlineDataNode& node) override;
        
    private:
        SymbolTable& symbol_table_;
        std::unique_ptr<CodeGraph> graph_;
        std::vector<CodeGraphError> errors_;
        
        // Current context
        bool in_data_section_;
        bool in_code_section_;
        InstructionOperand current_operand_;
        std::string current_instruction_mnemonic_;  // Track current instruction
        
        // Anonymous data counter
        uint32_t anonymous_counter_;
        
        // Helper methods
        void error(const std::string& message, size_t line, size_t column);
        std::vector<uint8_t> data_definition_to_bytes(const DataDefinitionNode& node);
        std::vector<uint8_t> inline_data_to_bytes(const InlineDataNode& node);
        std::vector<uint8_t> string_to_bytes(const std::string& str);
        uint8_t get_opcode_for_instruction(const std::string& mnemonic);
        uint8_t get_opcode_for_instruction_with_operands(const std::string& mnemonic, 
                                                         const std::vector<InstructionOperand>& operands);
        bool instruction_expects_word_immediate(const std::string& mnemonic) const;
        bool instruction_expects_byte_immediate(const std::string& mnemonic) const;
    };

} // namespace assembler
} // namespace lvm
