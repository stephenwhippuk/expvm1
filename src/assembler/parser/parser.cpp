#include "parser.h"
#include <algorithm>

namespace lvm {
namespace assembler {

    Parser::Parser(Lexer& lexer) : lexer_(lexer) {
        // Prime the pump
        advance();
    }

    std::unique_ptr<ProgramNode> Parser::parse() {
        auto program = std::make_unique<ProgramNode>();
        
        skip_empty_lines();
        
        while (!check(TokenType::END_OF_FILE)) {
            try {
                if (is_at_section_start()) {
                    auto section = parse_section();
                    if (section) {
                        program->add_section(std::move(section));
                    }
                } else if (check(TokenType::END_OF_LINE)) {
                    advance();  // Skip empty lines
                } else {
                    error_at_current("Expected section (DATA or CODE)");
                    synchronize();
                }
            } catch (const ParseError& e) {
                // Error already recorded, continue parsing
                synchronize();
            }
        }
        
        return program;
    }

    void Parser::advance() {
        previous_ = current_;
        current_ = lexer_.next_token();
        
        // Skip comments
        while (current_.type == TokenType::COMMENT) {
            current_ = lexer_.next_token();
        }
    }

    bool Parser::check(TokenType type) const {
        return current_.type == type;
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    Token Parser::consume(TokenType type, const std::string& message) {
        if (check(type)) {
            Token token = current_;
            advance();
            return token;
        }
        
        error_at_current(message);
        throw ParseError(message, current_.line, current_.column);
    }

    void Parser::skip_to_next_line() {
        while (!check(TokenType::END_OF_LINE) && !check(TokenType::END_OF_FILE)) {
            advance();
        }
        if (check(TokenType::END_OF_LINE)) {
            advance();
        }
    }

    void Parser::skip_empty_lines() {
        while (check(TokenType::END_OF_LINE)) {
            advance();
        }
    }

    void Parser::error(const std::string& message) {
        error_at_current(message);
    }

    void Parser::error_at_current(const std::string& message) {
        if (panic_mode_) return;
        panic_mode_ = true;
        
        std::string error_msg = "Error at line " + std::to_string(current_.line) + 
                                ":" + std::to_string(current_.column) + ": " + message;
        errors_.push_back(error_msg);
    }

    void Parser::error_at_previous(const std::string& message) {
        if (panic_mode_) return;
        panic_mode_ = true;
        
        std::string error_msg = "Error at line " + std::to_string(previous_.line) + 
                                ":" + std::to_string(previous_.column) + ": " + message;
        errors_.push_back(error_msg);
    }

    void Parser::synchronize() {
        panic_mode_ = false;
        
        // Skip to next line or section
        while (!check(TokenType::END_OF_FILE)) {
            if (previous_.type == TokenType::END_OF_LINE) {
                return;
            }
            
            if (is_at_section_start()) {
                return;
            }
            
            advance();
        }
    }

    bool Parser::is_at_section_start() const {
        return check(TokenType::KEYWORD_DATA) || check(TokenType::KEYWORD_CODE);
    }

    bool Parser::is_at_statement_start() const {
        return check(TokenType::IDENTIFIER) || check(TokenType::KEYWORD_DB) || 
               check(TokenType::KEYWORD_DW);
    }

    std::unique_ptr<SectionNode> Parser::parse_section() {
        if (check(TokenType::KEYWORD_DATA)) {
            return parse_data_section();
        } else if (check(TokenType::KEYWORD_CODE)) {
            return parse_code_section();
        }
        
        error_at_current("Expected DATA or CODE section");
        return nullptr;
    }

    std::unique_ptr<DataSectionNode> Parser::parse_data_section() {
        auto section = std::make_unique<DataSectionNode>();
        section->set_location(current_.line, current_.column);
        
        consume(TokenType::KEYWORD_DATA, "Expected DATA keyword");
        consume(TokenType::END_OF_LINE, "Expected newline after DATA");
        
        skip_empty_lines();
        
        // Parse data definitions and page directives until we hit another section or EOF
        while (!check(TokenType::END_OF_FILE) && !is_at_section_start()) {
            if (check(TokenType::END_OF_LINE)) {
                advance();
                continue;
            }
            
            try {
                // Check if this is a PAGE directive
                if (check(TokenType::KEYWORD_PAGE)) {
                    auto page_dir = parse_page_directive();
                    if (page_dir) {
                        section->add_item(std::move(page_dir));
                    }
                } else {
                    // Otherwise it's a data definition
                    auto def = parse_data_definition();
                    if (def) {
                        section->add_item(std::move(def));
                    }
                }
            } catch (const ParseError& e) {
                skip_to_next_line();
            }
        }
        
        return section;
    }

    std::unique_ptr<CodeSectionNode> Parser::parse_code_section() {
        auto section = std::make_unique<CodeSectionNode>();
        section->set_location(current_.line, current_.column);
        
        consume(TokenType::KEYWORD_CODE, "Expected CODE keyword");
        consume(TokenType::END_OF_LINE, "Expected newline after CODE");
        
        skip_empty_lines();
        
        // Parse code statements until we hit another section or EOF
        while (!check(TokenType::END_OF_FILE) && !is_at_section_start()) {
            if (check(TokenType::END_OF_LINE)) {
                advance();
                continue;
            }
            
            try {
                auto statement = parse_code_statement();
                if (statement) {
                    section->add_statement(std::move(statement));
                }
            } catch (const ParseError& e) {
                skip_to_next_line();
            }
        }
        
        return section;
    }

    std::unique_ptr<PageDirectiveNode> Parser::parse_page_directive() {
        // PAGE pageName
        Token page_token = consume(TokenType::KEYWORD_PAGE, "Expected PAGE keyword");
        Token name_token = consume(TokenType::IDENTIFIER, "Expected page name after PAGE");
        consume(TokenType::END_OF_LINE, "Expected newline after PAGE directive");
        
        auto page_dir = std::make_unique<PageDirectiveNode>(name_token.lexeme);
        page_dir->set_location(page_token.line, page_token.column);
        
        return page_dir;
    }

    std::unique_ptr<DataDefinitionNode> Parser::parse_data_definition() {
        // IDENTIFIER : DB/DW/DA ...
        Token label_token = consume(TokenType::IDENTIFIER, "Expected label");
        consume(TokenType::COLON, "Expected ':' after label");
        
        DataDefinitionNode::Type def_type;
        if (match(TokenType::KEYWORD_DB)) {
            def_type = DataDefinitionNode::Type::BYTE;
        } else if (match(TokenType::KEYWORD_DW)) {
            def_type = DataDefinitionNode::Type::WORD;
        } else if (match(TokenType::KEYWORD_DA)) {
            def_type = DataDefinitionNode::Type::ADDRESS;
        } else {
            error_at_current("Expected DB, DW, or DA");
            throw ParseError("Expected DB, DW, or DA", current_.line, current_.column);
        }
        
        auto def = std::make_unique<DataDefinitionNode>(label_token.lexeme, def_type);
        def->set_location(label_token.line, label_token.column);
        
        // DA only supports label arrays
        if (def_type == DataDefinitionNode::Type::ADDRESS) {
            consume(TokenType::LEFT_BRACKET, "DA requires array notation [label1, label2, ...]");
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    Token label = consume(TokenType::IDENTIFIER, "Expected label identifier");
                    def->add_label_reference(label.lexeme);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
        }
        // Parse the data: either STRING or [number, ...]
        else if (check(TokenType::STRING)) {
            def->set_string_data(current_.value);
            advance();
        } else if (match(TokenType::LEFT_BRACKET)) {
            // Parse number list
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    Token num = consume(TokenType::NUMBER, "Expected number");
                    def->add_numeric_value(num.number_value);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
        } else {
            error_at_current("Expected string or array");
            throw ParseError("Expected string or array", current_.line, current_.column);
        }
        
        consume(TokenType::END_OF_LINE, "Expected newline after data definition");
        
        return def;
    }

    std::unique_ptr<ASTNode> Parser::parse_code_statement() {
        // Could be: label, instruction, or inline data
        
        // Inline data: DB "string" or DW [...]
        if (check(TokenType::KEYWORD_DB) || check(TokenType::KEYWORD_DW)) {
            return parse_inline_data();
        }
        
        // Must be identifier (label or instruction)
        if (!check(TokenType::IDENTIFIER)) {
            error_at_current("Expected instruction or label");
            throw ParseError("Expected instruction or label", current_.line, current_.column);
        }
        
        Token name_token = current_;
        advance();
        
        // Check if it's a label (followed by colon)
        if (check(TokenType::COLON)) {
            advance();
            consume(TokenType::END_OF_LINE, "Expected newline after label");
            return parse_label(name_token.lexeme);
        }
        
        // Otherwise it's an instruction
        return parse_instruction(name_token.lexeme);
    }

    std::unique_ptr<LabelNode> Parser::parse_label(const std::string& name) {
        auto label = std::make_unique<LabelNode>(name);
        label->set_location(previous_.line, previous_.column);
        return label;
    }

    std::unique_ptr<InstructionNode> Parser::parse_instruction(const std::string& mnemonic) {
        auto instruction = std::make_unique<InstructionNode>(mnemonic);
        instruction->set_location(previous_.line, previous_.column);
        
        // Parse operands (if any)
        if (!check(TokenType::END_OF_LINE) && !check(TokenType::END_OF_FILE)) {
            do {
                auto operand = parse_operand();
                instruction->add_operand(std::move(operand));
            } while (match(TokenType::COMMA));
        }
        
        consume(TokenType::END_OF_LINE, "Expected newline after instruction");
        
        return instruction;
    }

    std::unique_ptr<InlineDataNode> Parser::parse_inline_data() {
        InlineDataNode::Type type;
        
        if (match(TokenType::KEYWORD_DB)) {
            type = InlineDataNode::Type::BYTE;
        } else if (match(TokenType::KEYWORD_DW)) {
            type = InlineDataNode::Type::WORD;
        } else {
            error_at_current("Expected DB or DW");
            throw ParseError("Expected DB or DW", current_.line, current_.column);
        }
        
        std::unique_ptr<InlineDataNode> inline_data;
        
        if (check(TokenType::STRING)) {
            inline_data = std::make_unique<InlineDataNode>(type, current_.value);
            advance();
        } else if (match(TokenType::LEFT_BRACKET)) {
            std::vector<uint64_t> values;
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    Token num = consume(TokenType::NUMBER, "Expected number");
                    values.push_back(num.number_value);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
            inline_data = std::make_unique<InlineDataNode>(type, values);
        } else {
            error_at_current("Expected string or array");
            throw ParseError("Expected string or array", current_.line, current_.column);
        }
        
        inline_data->set_location(previous_.line, previous_.column);
        consume(TokenType::END_OF_LINE, "Expected newline after inline data");
        
        return inline_data;
    }

    std::unique_ptr<InlineDataNode> Parser::parse_inline_data_operand() {
        InlineDataNode::Type type;
        
        if (match(TokenType::KEYWORD_DB)) {
            type = InlineDataNode::Type::BYTE;
        } else if (match(TokenType::KEYWORD_DW)) {
            type = InlineDataNode::Type::WORD;
        } else {
            error_at_current("Expected DB or DW");
            throw ParseError("Expected DB or DW", current_.line, current_.column);
        }
        
        std::unique_ptr<InlineDataNode> inline_data;
        
        if (check(TokenType::STRING)) {
            inline_data = std::make_unique<InlineDataNode>(type, current_.value);
            advance();
        } else if (match(TokenType::LEFT_BRACKET)) {
            std::vector<uint64_t> values;
            if (!check(TokenType::RIGHT_BRACKET)) {
                do {
                    Token num = consume(TokenType::NUMBER, "Expected number");
                    values.push_back(num.number_value);
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
            inline_data = std::make_unique<InlineDataNode>(type, values);
        } else {
            error_at_current("Expected string or array");
            throw ParseError("Expected string or array", current_.line, current_.column);
        }
        
        inline_data->set_location(previous_.line, previous_.column);
        // Note: No newline consumption for operand version
        
        return inline_data;
    }

    std::unique_ptr<OperandNode> Parser::parse_operand() {
        // Inline data: DB/DW as operand (creates anonymous data block)
        if (check(TokenType::KEYWORD_DB) || check(TokenType::KEYWORD_DW)) {
            auto inline_data = parse_inline_data_operand();
            
            // Check for optional IN page_name
            if (match(TokenType::KEYWORD_IN)) {
                if (!check(TokenType::IDENTIFIER)) {
                    error("Expected page name after IN keyword");
                    return nullptr;
                }
                std::string page_name = current_.value;
                advance();
                inline_data->set_page_name(page_name);
            }
            
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::INLINE_DATA);
            operand->set_inline_data(std::move(inline_data));
            operand->set_location(previous_.line, previous_.column);
            return operand;
        }
        
        // Register
        if (check(TokenType::REGISTER)) {
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::REGISTER);
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::REGISTER);
            expr->set_register(current_.lexeme);
            expr->set_location(current_.line, current_.column);
            operand->set_expression(std::move(expr));
            operand->set_location(current_.line, current_.column);
            advance();
            return operand;
        }
        
        // Parentheses: (expression) - address computation
        if (check(TokenType::LEFT_PAREN)) {
            size_t line = current_.line, col = current_.column;
            advance();
            
            auto expr = parse_expression();
            consume(TokenType::RIGHT_PAREN, "Expected ')'");
            
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::ADDRESS_EXPR);
            operand->set_expression(std::move(expr));
            operand->set_location(line, col);
            return operand;
        }
        
        // Square brackets: [expression] - memory dereference
        if (check(TokenType::LEFT_BRACKET)) {
            size_t line = current_.line, col = current_.column;
            advance();
            
            auto expr = parse_expression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']'");
            
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::MEMORY_ACCESS);
            operand->set_expression(std::move(expr));
            operand->set_location(line, col);
            return operand;
        }
        
        // Number (immediate)
        if (check(TokenType::NUMBER)) {
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::IMMEDIATE);
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::NUMBER);
            expr->set_number(current_.number_value);
            expr->set_location(current_.line, current_.column);
            operand->set_expression(std::move(expr));
            operand->set_location(current_.line, current_.column);
            advance();
            return operand;
        }
        
        // Identifier - may be followed by [expr] for sugar syntax
        if (check(TokenType::IDENTIFIER)) {
            std::string identifier = current_.lexeme;
            size_t line = current_.line, col = current_.column;
            advance();
            
            // Check for sugar syntax: label[expr]
            if (check(TokenType::LEFT_BRACKET)) {
                advance();  // consume [
                auto index_expr = parse_expression();
                consume(TokenType::RIGHT_BRACKET, "Expected ']'");
                
                // Create label + index expression
                auto label_expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::IDENTIFIER);
                label_expr->set_identifier(identifier);
                label_expr->set_location(line, col);
                
                auto combined = std::make_unique<ExpressionNode>(ExpressionNode::Type::BINARY_OP);
                combined->set_operator('+');
                combined->set_left(std::move(label_expr));
                combined->set_right(std::move(index_expr));
                combined->set_location(line, col);
                
                // Mark as memory access with sugar syntax
                auto operand = std::make_unique<OperandNode>(OperandNode::Type::MEMORY_ACCESS);
                operand->set_expression(std::move(combined));
                operand->set_sugar_syntax(true);
                operand->set_location(line, col);
                return operand;
            }
            
            // Plain identifier
            auto operand = std::make_unique<OperandNode>(OperandNode::Type::IDENTIFIER);
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::IDENTIFIER);
            expr->set_identifier(identifier);
            expr->set_location(line, col);
            operand->set_expression(std::move(expr));
            operand->set_location(line, col);
            return operand;
        }
        
        error_at_current("Expected operand");
        throw ParseError("Expected operand", current_.line, current_.column);
    }

    std::unique_ptr<ExpressionNode> Parser::parse_expression() {
        auto left = parse_term();
        
        // Handle + and - operators
        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            char op = current_.lexeme[0];
            size_t line = current_.line, col = current_.column;
            advance();
            
            auto right = parse_term();
            
            auto binary = std::make_unique<ExpressionNode>(ExpressionNode::Type::BINARY_OP);
            binary->set_operator(op);
            binary->set_left(std::move(left));
            binary->set_right(std::move(right));
            binary->set_location(line, col);
            
            left = std::move(binary);
        }
        
        return left;
    }

    std::unique_ptr<ExpressionNode> Parser::parse_term() {
        // Number
        if (check(TokenType::NUMBER)) {
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::NUMBER);
            expr->set_number(current_.number_value);
            expr->set_location(current_.line, current_.column);
            advance();
            return expr;
        }
        
        // Register
        if (check(TokenType::REGISTER)) {
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::REGISTER);
            expr->set_register(current_.lexeme);
            expr->set_location(current_.line, current_.column);
            advance();
            return expr;
        }
        
        // Identifier
        if (check(TokenType::IDENTIFIER)) {
            auto expr = std::make_unique<ExpressionNode>(ExpressionNode::Type::IDENTIFIER);
            expr->set_identifier(current_.lexeme);
            expr->set_location(current_.line, current_.column);
            advance();
            return expr;
        }
        
        error_at_current("Expected number, register, or identifier in expression");
        throw ParseError("Expected term in expression", current_.line, current_.column);
    }

} // namespace assembler
} // namespace lvm
