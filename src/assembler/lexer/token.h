#pragma once

#include <string>
#include <cstdint>

namespace lvm {
namespace assembler {

    /**
     * Token types for the assembler lexer
     */
    enum class TokenType {
        // End of input
        END_OF_FILE,
        END_OF_LINE,
        
        // Keywords
        KEYWORD_DATA,           // DATA
        KEYWORD_CODE,           // CODE
        KEYWORD_DB,             // DB (define byte)
        KEYWORD_DW,             // DW (define word)
        KEYWORD_DA,             // DA (define address array)
        KEYWORD_PAGE,           // PAGE (page directive)
        KEYWORD_IN,             // IN (inline data page specification)
        
        // Identifiers and literals
        IDENTIFIER,             // Labels, variable names
        REGISTER,               // AX, BX, CX, DX, EX
        NUMBER,                 // 42, 0xFF, etc.
        STRING,                 // "Hello" or 'Hello'
        
        // Operators
        COLON,                  // :
        COMMA,                  // ,
        LEFT_BRACKET,           // [
        RIGHT_BRACKET,          // ]
        LEFT_PAREN,             // (
        RIGHT_PAREN,            // )
        PLUS,                   // +
        MINUS,                  // -
        
        // Comments
        COMMENT,                // ; comment text
        
        // Error/Unknown
        UNKNOWN
    };

    /**
     * Represents a single token from the source
     */
    struct Token {
        TokenType type;
        std::string lexeme;         // The actual text
        std::string value;          // Processed value (e.g., string without quotes)
        uint64_t number_value;      // Numeric value if type is NUMBER
        size_t line;                // Source line number
        size_t column;              // Source column number
        
        Token() 
            : type(TokenType::UNKNOWN), number_value(0), line(0), column(0) {}
            
        Token(TokenType t, const std::string& lex, size_t ln, size_t col)
            : type(t), lexeme(lex), value(lex), number_value(0), line(ln), column(col) {}
            
        bool is_keyword() const {
            return type >= TokenType::KEYWORD_DATA && type <= TokenType::KEYWORD_IN;
        }
        
        bool is_operator() const {
            return type >= TokenType::COLON && type <= TokenType::MINUS;
        }
        
        std::string to_string() const;
    };

    /**
     * Converts token type to string for debugging/errors
     */
    const char* token_type_to_string(TokenType type);

} // namespace assembler
} // namespace lvm
