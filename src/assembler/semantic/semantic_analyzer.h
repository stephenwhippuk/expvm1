#pragma once

#include "symbol_table.h"
#include "../parser/ast.h"
#include <string>
#include <vector>
#include <memory>

namespace lvm {
namespace assembler {

    /**
     * Semantic error
     */
    struct SemanticError {
        std::string message;
        size_t line;
        size_t column;
        
        SemanticError(const std::string& msg, size_t ln, size_t col)
            : message(msg), line(ln), column(col) {}
            
        std::string to_string() const {
            return "Semantic error at " + std::to_string(line) + ":" + 
                   std::to_string(column) + ": " + message;
        }
    };

    /**
     * Semantic analyzer (Pass 2)
     * 
     * Performs semantic analysis on the AST:
     * - Builds symbol table
     * - Validates operand types
     * - Checks for duplicate definitions
     * - Checks for undefined references
     * - Validates register usage
     * - Validates memory access expressions
     */
    class SemanticAnalyzer : public ASTVisitor {
    public:
        explicit SemanticAnalyzer(SymbolTable& symbol_table);
        
        /**
         * Analyze the AST
         * @return true if successful, false if errors found
         */
        bool analyze(ProgramNode& program);
        
        /**
         * Get all errors found during analysis
         */
        const std::vector<SemanticError>& errors() const { return errors_; }
        
        /**
         * Check if any errors occurred
         */
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
        std::vector<SemanticError> errors_;
        
        // Current context
        bool in_data_section_;
        bool in_code_section_;
        
        // Helper methods
        void error(const std::string& message, size_t line, size_t column);
        void validate_register(const std::string& reg, size_t line, size_t column);
        void validate_identifier_reference(const std::string& name, size_t line, 
                                          size_t column, const std::string& context);
        uint32_t calculate_data_size(const DataDefinitionNode& node);
        uint32_t calculate_inline_data_size(const InlineDataNode& node);
    };

} // namespace assembler
} // namespace lvm
