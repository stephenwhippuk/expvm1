#include "token.h"
#include <sstream>

namespace lvm {
namespace assembler {

    const char* token_type_to_string(TokenType type) {
        switch (type) {
            case TokenType::END_OF_FILE: return "END_OF_FILE";
            case TokenType::END_OF_LINE: return "END_OF_LINE";
            case TokenType::KEYWORD_DATA: return "DATA";
            case TokenType::KEYWORD_CODE: return "CODE";
            case TokenType::KEYWORD_DB: return "DB";
            case TokenType::KEYWORD_DW: return "DW";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::REGISTER: return "REGISTER";
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::STRING: return "STRING";
            case TokenType::COLON: return "COLON";
            case TokenType::COMMA: return "COMMA";
            case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
            case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
            case TokenType::LEFT_PAREN: return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
            case TokenType::PLUS: return "PLUS";
            case TokenType::MINUS: return "MINUS";
            case TokenType::COMMENT: return "COMMENT";
            case TokenType::UNKNOWN: return "UNKNOWN";
            default: return "INVALID";
        }
    }

    std::string Token::to_string() const {
        std::ostringstream oss;
        oss << token_type_to_string(type) << " '" << lexeme << "'";
        if (type == TokenType::NUMBER) {
            oss << " (0x" << std::hex << number_value << std::dec << ")";
        }
        oss << " at " << line << ":" << column;
        return oss.str();
    }

} // namespace assembler
} // namespace lvm
