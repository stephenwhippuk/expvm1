#pragma once

#include "symbol_table.h"
#include "../parser/ast.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

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
        
        /**
         * Get page number by page name (for inline data IN clause)
         * Returns 0 if page name not found
         */
        uint16_t get_page_number(const std::string& page_name) const {
            auto it = page_names_.find(page_name);
            return (it != page_names_.end()) ? it->second : 0;
        }
        
        // Visitor methods
        void visit(ProgramNode& node) override;
        void visit(DataSectionNode& node) override;
        void visit(CodeSectionNode& node) override;
        void visit(PageDirectiveNode& node) override;
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
        
        // Page tracking (for PAGE directive support)
        uint16_t current_page_;           // Current page number (0-65535)
        std::unordered_map<uint16_t, uint32_t> page_sizes_;  // Bytes used per page
        std::unordered_map<std::string, uint16_t> page_names_; // Page name -> number mapping
        uint32_t current_page_address_;   // Current address within page (0-65535)
        
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
