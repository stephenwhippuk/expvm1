#pragma once

#include "ast.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include <memory>
#include <string>
#include <stdexcept>

namespace lvm {
namespace assembler {

    /**
     * Exception thrown during parsing
     */
    class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& message, size_t line, size_t column)
            : std::runtime_error(format_message(message, line, column))
            , line_(line), column_(column) {}
        
        size_t line() const { return line_; }
        size_t column() const { return column_; }
        
    private:
        size_t line_;
        size_t column_;
        
        static std::string format_message(const std::string& msg, size_t line, size_t col) {
            return "Parse error at " + std::to_string(line) + ":" + 
                   std::to_string(col) + ": " + msg;
        }
    };

    /**
     * Recursive descent parser for Pendragon assembly language
     * 
     * Grammar:
     *   program        → section*
     *   section        → data_section | code_section
     *   data_section   → "DATA" EOL data_definition*
     *   code_section   → "CODE" EOL code_statement*
     *   data_definition→ IDENTIFIER ":" data_directive EOL
     *   data_directive → "DB" (STRING | "[" number_list "]")
     *                  | "DW" "[" number_list "]"
     *   code_statement → label | instruction | inline_data
     *   label          → IDENTIFIER ":" EOL
     *   instruction    → IDENTIFIER operand_list? EOL
     *   inline_data    → "DB" (STRING | "[" number_list "]") EOL
     *   operand_list   → operand ("," operand)*
     *   operand        → register | number | identifier | memory_access
     *   memory_access  → "[" expression "]" | "(" expression ")"
     *   expression     → term (("+"|"-") term)*
     *   term           → number | identifier | register
     */
    class Parser {
    public:
        explicit Parser(Lexer& lexer);
        
        /**
         * Parse the entire program
         * @return AST root node
         * @throws ParseError on syntax errors
         */
        std::unique_ptr<ProgramNode> parse();
        
        /**
         * Get all error messages accumulated during parsing
         */
        const std::vector<std::string>& errors() const { return errors_; }
        
        /**
         * Check if any errors occurred
         */
        bool has_errors() const { return !errors_.empty(); }
        
    private:
        Lexer& lexer_;
        Token current_;
        Token previous_;
        std::vector<std::string> errors_;
        bool panic_mode_ = false;
        
        // Token management
        void advance();
        bool check(TokenType type) const;
        bool match(TokenType type);
        Token consume(TokenType type, const std::string& message);
        void skip_to_next_line();
        void skip_empty_lines();
        
        // Error handling
        void error(const std::string& message);
        void error_at_current(const std::string& message);
        void error_at_previous(const std::string& message);
        void synchronize();
        
        // Grammar rules
        std::unique_ptr<SectionNode> parse_section();
        std::unique_ptr<DataSectionNode> parse_data_section();
        std::unique_ptr<CodeSectionNode> parse_code_section();
        std::unique_ptr<PageDirectiveNode> parse_page_directive();
        std::unique_ptr<DataDefinitionNode> parse_data_definition();
        std::unique_ptr<ASTNode> parse_code_statement();
        std::unique_ptr<LabelNode> parse_label(const std::string& name);
        std::unique_ptr<InstructionNode> parse_instruction(const std::string& mnemonic);
        std::unique_ptr<InlineDataNode> parse_inline_data();
        std::unique_ptr<InlineDataNode> parse_inline_data_operand();
        std::unique_ptr<OperandNode> parse_operand();
        std::unique_ptr<ExpressionNode> parse_expression();
        std::unique_ptr<ExpressionNode> parse_term();
        
        // Helper functions
        bool is_at_section_start() const;
        bool is_at_statement_start() const;
    };

} // namespace assembler
} // namespace lvm
