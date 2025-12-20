#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>

namespace lvm {
namespace assembler {

    Lexer::Lexer(const std::string& source)
        : source_(source), current_(0), start_(0), line_(1), column_(1), has_peek_(false) {
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        
        while (true) {
            Token token = next_token();
            
            // Skip comments but keep EOL
            if (token.type != TokenType::COMMENT) {
                tokens.push_back(token);
            }
            
            if (token.type == TokenType::END_OF_FILE) {
                break;
            }
        }
        
        return tokens;
    }

    Token Lexer::next_token() {
        if (has_peek_) {
            has_peek_ = false;
            return peek_cache_;
        }
        
        skip_whitespace();
        
        if (is_at_end()) {
            return make_token(TokenType::END_OF_FILE);
        }
        
        start_ = current_;
        char c = current_char();
        advance();
        
        // Single-character tokens
        switch (c) {
            case '\n':
                line_++;
                column_ = 1;
                return make_token(TokenType::END_OF_LINE);
            case ':': return make_token(TokenType::COLON);
            case ',': return make_token(TokenType::COMMA);
            case '[': return make_token(TokenType::LEFT_BRACKET);
            case ']': return make_token(TokenType::RIGHT_BRACKET);
            case '(': return make_token(TokenType::LEFT_PAREN);
            case ')': return make_token(TokenType::RIGHT_PAREN);
            case '+': return make_token(TokenType::PLUS);
            case '-': 
                // Could be negative number or minus operator
                if (is_digit(current_char())) {
                    current_--;  // Back up to include the minus
                    return number();
                }
                return make_token(TokenType::MINUS);
            case ';':
                skip_comment();
                return make_token(TokenType::COMMENT);
            case '"':
            case '\'':
                return string_literal(c);
        }
        
        // Numbers
        if (is_digit(c)) {
            current_--;  // Back up to re-read the digit
            return number();
        }
        
        // Identifiers and keywords
        if (is_alpha(c) || c == '_') {
            current_--;  // Back up to re-read
            return identifier_or_keyword();
        }
        
        return error_token("Unexpected character");
    }

    Token Lexer::peek_token() {
        if (!has_peek_) {
            size_t saved_current = current_;
            size_t saved_start = start_;
            size_t saved_line = line_;
            size_t saved_column = column_;
            
            peek_cache_ = next_token();
            has_peek_ = true;
            
            current_ = saved_current;
            start_ = saved_start;
            line_ = saved_line;
            column_ = saved_column;
        }
        
        return peek_cache_;
    }

    char Lexer::current_char() const {
        if (is_at_end()) return '\0';
        return source_[current_];
    }

    char Lexer::peek_char(size_t offset) const {
        size_t pos = current_ + offset;
        if (pos >= source_.length()) return '\0';
        return source_[pos];
    }

    bool Lexer::match(char expected) {
        if (is_at_end()) return false;
        if (current_char() != expected) return false;
        advance();
        return true;
    }

    void Lexer::advance() {
        if (!is_at_end()) {
            current_++;
            column_++;
        }
    }

    void Lexer::skip_whitespace() {
        while (!is_at_end()) {
            char c = current_char();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else {
                break;
            }
        }
    }

    void Lexer::skip_comment() {
        // Comment goes to end of line
        while (!is_at_end() && current_char() != '\n') {
            advance();
        }
    }

    Token Lexer::make_token(TokenType type) {
        std::string lexeme = source_.substr(start_, current_ - start_);
        return Token(type, lexeme, line_, column_ - lexeme.length());
    }

    Token Lexer::make_token(TokenType type, const std::string& value) {
        Token token = make_token(type);
        token.value = value;
        return token;
    }

    Token Lexer::make_number_token(uint64_t value) {
        Token token = make_token(TokenType::NUMBER);
        token.number_value = value;
        return token;
    }

    Token Lexer::error_token(const std::string& message) {
        Token token = make_token(TokenType::UNKNOWN);
        token.value = message;
        return token;
    }

    Token Lexer::identifier_or_keyword() {
        while (is_alphanumeric(current_char()) || current_char() == '_') {
            advance();
        }
        
        std::string text = source_.substr(start_, current_ - start_);
        
        // Check for keyword
        TokenType type = identify_keyword(text);
        if (type != TokenType::IDENTIFIER) {
            return make_token(type);
        }
        
        // Check for register
        if (is_register(text)) {
            return make_token(TokenType::REGISTER);
        }
        
        return make_token(TokenType::IDENTIFIER);
    }

    Token Lexer::number() {
        bool is_negative = false;
        if (current_char() == '-') {
            is_negative = true;
            advance();
        }
        
        uint64_t value = 0;
        
        // Check for hex prefix
        if (current_char() == '0' && (peek_char() == 'x' || peek_char() == 'X')) {
            advance();  // Skip '0'
            advance();  // Skip 'x'
            
            // Parse hex
            while (is_hex_digit(current_char())) {
                char c = current_char();
                value = value * 16 + (is_digit(c) ? (c - '0') : (std::tolower(c) - 'a' + 10));
                advance();
            }
        } else {
            // Parse decimal
            while (is_digit(current_char())) {
                value = value * 10 + (current_char() - '0');
                advance();
            }
        }
        
        if (is_negative) {
            value = static_cast<uint64_t>(-static_cast<int64_t>(value));
        }
        
        return make_number_token(value);
    }

    Token Lexer::string_literal(char quote) {
        std::string value;
        
        while (!is_at_end() && current_char() != quote) {
            if (current_char() == '\\') {
                advance();
                if (!is_at_end()) {
                    char escaped = current_char();
                    switch (escaped) {
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case 'r': value += '\r'; break;
                        case '\\': value += '\\'; break;
                        case '"': value += '"'; break;
                        case '\'': value += '\''; break;
                        case '0': value += '\0'; break;
                        default: value += escaped; break;
                    }
                    advance();
                }
            } else {
                if (current_char() == '\n') {
                    line_++;
                    column_ = 0;
                }
                value += current_char();
                advance();
            }
        }
        
        if (is_at_end()) {
            return error_token("Unterminated string");
        }
        
        advance();  // Closing quote
        
        return make_token(TokenType::STRING, value);
    }

    bool Lexer::is_digit(char c) const {
        return c >= '0' && c <= '9';
    }

    bool Lexer::is_hex_digit(char c) const {
        return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    bool Lexer::is_alpha(char c) const {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool Lexer::is_alphanumeric(char c) const {
        return is_alpha(c) || is_digit(c);
    }

    TokenType Lexer::identify_keyword(const std::string& text) const {
        // Case-insensitive keyword matching
        std::string upper = text;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        if (upper == "DATA") return TokenType::KEYWORD_DATA;
        if (upper == "CODE") return TokenType::KEYWORD_CODE;
        if (upper == "PAGE") return TokenType::KEYWORD_PAGE;
        if (upper == "IN") return TokenType::KEYWORD_IN;
        if (upper == "DB") return TokenType::KEYWORD_DB;
        if (upper == "DW") return TokenType::KEYWORD_DW;
        if (upper == "DA") return TokenType::KEYWORD_DA;
        
        return TokenType::IDENTIFIER;
    }

    bool Lexer::is_register(const std::string& text) const {
        // Case-insensitive register matching
        std::string upper = text;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        // 16-bit registers
        if (upper == "AX" || upper == "BX" || upper == "CX" || 
            upper == "DX" || upper == "EX") {
            return true;
        }
        
        // 8-bit sub-registers (high and low bytes)
        if (upper == "AH" || upper == "AL" || upper == "BH" || upper == "BL" ||
            upper == "CH" || upper == "CL" || upper == "DH" || upper == "DL" ||
            upper == "EH" || upper == "EL") {
            return true;
        }
        
        return false;
    }

} // namespace assembler
} // namespace lvm

