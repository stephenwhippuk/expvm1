#pragma once

#include "token.h"
#include <vector>
#include <string>
#include <memory>

namespace lvm {
namespace assembler {

    /**
     * Lexer for Pendragon assembly language
     * 
     * Converts source text into a stream of tokens for the parser.
     * Handles:
     * - Keywords (DATA, CODE, DB, DW)
     * - Identifiers (labels, instructions)
     * - Numbers (decimal and hex)
     * - Strings (double and single quoted)
     * - Operators and punctuation
     * - Comments (;)
     * - Whitespace (ignored except for EOL)
     */
    class Lexer {
    public:
        explicit Lexer(const std::string& source);
        
        /**
         * Tokenize the entire source
         * @return Vector of all tokens
         */
        std::vector<Token> tokenize();
        
        /**
         * Get the next token (for streaming parsing)
         * @return Next token, or END_OF_FILE if done
         */
        Token next_token();
        
        /**
         * Peek at the next token without consuming it
         * @return Next token
         */
        Token peek_token();
        
        /**
         * Check if we've reached end of input
         */
        bool is_at_end() const { return current_ >= source_.length(); }
        
        /**
         * Get current line number
         */
        size_t current_line() const { return line_; }
        
    private:
        std::string source_;
        size_t current_;        // Current position in source
        size_t start_;          // Start of current token
        size_t line_;           // Current line number
        size_t column_;         // Current column number
        Token peek_cache_;      // Cached peek token
        bool has_peek_;         // Whether peek cache is valid
        
        // Character examination
        char current_char() const;
        char peek_char(size_t offset = 1) const;
        bool match(char expected);
        void advance();
        void skip_whitespace();
        void skip_comment();
        
        // Token generation
        Token make_token(TokenType type);
        Token make_token(TokenType type, const std::string& value);
        Token make_number_token(uint64_t value);
        Token error_token(const std::string& message);
        
        // Specific token parsers
        Token identifier_or_keyword();
        Token number();
        Token string_literal(char quote);
        
        // Helper functions
        bool is_digit(char c) const;
        bool is_hex_digit(char c) const;
        bool is_alpha(char c) const;
        bool is_alphanumeric(char c) const;
        TokenType identify_keyword(const std::string& text) const;
        bool is_register(const std::string& text) const;
    };

} // namespace assembler
} // namespace lvm
