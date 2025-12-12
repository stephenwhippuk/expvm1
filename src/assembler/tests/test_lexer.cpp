#include <gtest/gtest.h>
#include "../lexer/lexer.h"
#include "../lexer/token.h"

using namespace lvm::assembler;

TEST(LexerTest, EmptyInput) {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    // tokenize() includes EOF at end
    ASSERT_GE(tokens.size(), 1);
    EXPECT_EQ(tokens.back().type, TokenType::END_OF_FILE);
}

TEST(LexerTest, Keywords) {
    Lexer lexer("DATA CODE DB DW");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::KEYWORD_DATA);
    EXPECT_EQ(tokens[1].type, TokenType::KEYWORD_CODE);
    EXPECT_EQ(tokens[2].type, TokenType::KEYWORD_DB);
    EXPECT_EQ(tokens[3].type, TokenType::KEYWORD_DW);
}

TEST(LexerTest, CaseInsensitiveKeywords) {
    Lexer lexer("data Data DaTa");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::KEYWORD_DATA);
    EXPECT_EQ(tokens[1].type, TokenType::KEYWORD_DATA);
    EXPECT_EQ(tokens[2].type, TokenType::KEYWORD_DATA);
}

TEST(LexerTest, Identifiers) {
    Lexer lexer("label123 _test my_label");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "label123");
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].lexeme, "_test");
    EXPECT_EQ(tokens[2].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].lexeme, "my_label");
}

TEST(LexerTest, Registers) {
    Lexer lexer("AX BX CX DX EX ax bx");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 7);
    EXPECT_EQ(tokens[0].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[1].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[2].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[3].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[4].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[5].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[6].type, TokenType::REGISTER);
}

TEST(LexerTest, DecimalNumbers) {
    Lexer lexer("0 42 255 1000");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].number_value, 0);
    EXPECT_EQ(tokens[1].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[1].number_value, 42);
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[2].number_value, 255);
    EXPECT_EQ(tokens[3].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[3].number_value, 1000);
}

TEST(LexerTest, HexNumbers) {
    Lexer lexer("0x00 0xFF 0xDEADBEEF 0X10");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].number_value, 0x00);
    EXPECT_EQ(tokens[1].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[1].number_value, 0xFF);
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[2].number_value, 0xDEADBEEF);
    EXPECT_EQ(tokens[3].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[3].number_value, 0x10);
}

TEST(LexerTest, Strings) {
    Lexer lexer("\"Hello\" 'World'");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::STRING);
    EXPECT_EQ(tokens[0].value, "Hello");
    EXPECT_EQ(tokens[1].type, TokenType::STRING);
    EXPECT_EQ(tokens[1].value, "World");
}

TEST(LexerTest, StringEscapes) {
    Lexer lexer(R"("Hello\nWorld" "Tab\there" "Quote\"here")");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 3);
    EXPECT_EQ(tokens[0].type, TokenType::STRING);
    EXPECT_EQ(tokens[0].value, "Hello\nWorld");
    EXPECT_EQ(tokens[1].type, TokenType::STRING);
    EXPECT_EQ(tokens[1].value, "Tab\there");
    EXPECT_EQ(tokens[2].type, TokenType::STRING);
    EXPECT_EQ(tokens[2].value, "Quote\"here");
}

TEST(LexerTest, Operators) {
    Lexer lexer(": , [ ] ( ) + -");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 8);
    EXPECT_EQ(tokens[0].type, TokenType::COLON);
    EXPECT_EQ(tokens[1].type, TokenType::COMMA);
    EXPECT_EQ(tokens[2].type, TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[3].type, TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[4].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[5].type, TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokens[6].type, TokenType::PLUS);
    EXPECT_EQ(tokens[7].type, TokenType::MINUS);
}

TEST(LexerTest, Comments) {
    Lexer lexer("; This is a comment\nDATA ; Another comment");
    auto tokens = lexer.tokenize();
    
    // Comments are filtered out in tokenize()
    bool found_data = false;
    for (const auto& token : tokens) {
        EXPECT_NE(token.type, TokenType::COMMENT);
        if (token.type == TokenType::KEYWORD_DATA) {
            found_data = true;
        }
    }
    EXPECT_TRUE(found_data);
}

TEST(LexerTest, NewLines) {
    Lexer lexer("DATA\nCODE\n");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::KEYWORD_DATA);
    EXPECT_EQ(tokens[1].type, TokenType::END_OF_LINE);
    EXPECT_EQ(tokens[2].type, TokenType::KEYWORD_CODE);
    EXPECT_EQ(tokens[3].type, TokenType::END_OF_LINE);
}

TEST(LexerTest, ComplexExpression) {
    Lexer lexer("LABEL + 0x10 + CX");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 5);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "LABEL");
    EXPECT_EQ(tokens[1].type, TokenType::PLUS);
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[2].number_value, 0x10);
    EXPECT_EQ(tokens[3].type, TokenType::PLUS);
    EXPECT_EQ(tokens[4].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[4].lexeme, "CX");
}

TEST(LexerTest, DataDefinition) {
    Lexer lexer("HELLO: DB \"Hello, World!\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "HELLO");
    EXPECT_EQ(tokens[1].type, TokenType::COLON);
    EXPECT_EQ(tokens[2].type, TokenType::KEYWORD_DB);
    EXPECT_EQ(tokens[3].type, TokenType::STRING);
    EXPECT_EQ(tokens[3].value, "Hello, World!");
}

TEST(LexerTest, ArrayDefinition) {
    Lexer lexer("VALUES: DW [1, 2, 0xFF, 100]");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 11);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].type, TokenType::COLON);
    EXPECT_EQ(tokens[2].type, TokenType::KEYWORD_DW);
    EXPECT_EQ(tokens[3].type, TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[4].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[5].type, TokenType::COMMA);
    EXPECT_EQ(tokens[6].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[7].type, TokenType::COMMA);
    EXPECT_EQ(tokens[8].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[8].number_value, 0xFF);
}

TEST(LexerTest, Instruction) {
    Lexer lexer("PUSHB AX, 0x42");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].lexeme, "PUSHB");
    EXPECT_EQ(tokens[1].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[2].type, TokenType::COMMA);
    EXPECT_EQ(tokens[3].type, TokenType::NUMBER);
}

TEST(LexerTest, MemoryAccess) {
    Lexer lexer("[HELLO + CX] (LABEL + 0x10)");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 10);
    EXPECT_EQ(tokens[0].type, TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].type, TokenType::PLUS);
    EXPECT_EQ(tokens[3].type, TokenType::REGISTER);
    EXPECT_EQ(tokens[4].type, TokenType::RIGHT_BRACKET);
    EXPECT_EQ(tokens[5].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[6].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[7].type, TokenType::PLUS);
    EXPECT_EQ(tokens[8].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[9].type, TokenType::RIGHT_PAREN);
}
