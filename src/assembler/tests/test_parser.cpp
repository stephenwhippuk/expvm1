#include <gtest/gtest.h>
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"

using namespace lvm::assembler;

TEST(ParserTest, EmptyProgram) {
    Lexer lexer("");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_NE(ast, nullptr);
    EXPECT_EQ(ast->sections().size(), 0);
}

TEST(ParserTest, EmptyDataSection) {
    Lexer lexer("DATA\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<DataSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->definitions().size(), 0);
}

TEST(ParserTest, EmptyCodeSection) {
    Lexer lexer("CODE\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    EXPECT_EQ(section->statements().size(), 0);
}

TEST(ParserTest, DataDefinitionString) {
    Lexer lexer("DATA\nHELLO: DB \"Hello, World!\"\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<DataSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->definitions().size(), 1);
    
    auto& def = section->definitions()[0];
    EXPECT_EQ(def->label(), "HELLO");
    EXPECT_EQ(def->type(), DataDefinitionNode::Type::BYTE);
    EXPECT_TRUE(def->is_string());
    EXPECT_EQ(def->string_data(), "Hello, World!");
}

TEST(ParserTest, DataDefinitionArray) {
    Lexer lexer("DATA\nVALUES: DW [1, 2, 0xFF]\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<DataSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->definitions().size(), 1);
    
    auto& def = section->definitions()[0];
    EXPECT_EQ(def->label(), "VALUES");
    EXPECT_EQ(def->type(), DataDefinitionNode::Type::WORD);
    EXPECT_FALSE(def->is_string());
    ASSERT_EQ(def->numeric_data().size(), 3);
    EXPECT_EQ(def->numeric_data()[0], 1);
    EXPECT_EQ(def->numeric_data()[1], 2);
    EXPECT_EQ(def->numeric_data()[2], 0xFF);
}

TEST(ParserTest, Label) {
    Lexer lexer("CODE\nSTART:\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements().size(), 1);
    
    auto* label = dynamic_cast<LabelNode*>(section->statements()[0].get());
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->name(), "START");
}

TEST(ParserTest, InstructionNoOperands) {
    Lexer lexer("CODE\nHALT\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 1);
    
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements().size(), 1);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->mnemonic(), "HALT");
    EXPECT_EQ(instr->operands().size(), 0);
}

TEST(ParserTest, InstructionWithRegister) {
    Lexer lexer("CODE\nPUSH AX\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->mnemonic(), "PUSH");
    ASSERT_EQ(instr->operands().size(), 1);
    EXPECT_EQ(instr->operands()[0]->type(), OperandNode::Type::REGISTER);
}

TEST(ParserTest, InstructionWithImmediate) {
    Lexer lexer("CODE\nPUSHW 0x1234\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->mnemonic(), "PUSHW");
    ASSERT_EQ(instr->operands().size(), 1);
    EXPECT_EQ(instr->operands()[0]->type(), OperandNode::Type::IMMEDIATE);
    EXPECT_EQ(instr->operands()[0]->expression()->number(), 0x1234);
}

TEST(ParserTest, InstructionWithMultipleOperands) {
    Lexer lexer("CODE\nLD AX, 0x42\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->mnemonic(), "LD");
    ASSERT_EQ(instr->operands().size(), 2);
    EXPECT_EQ(instr->operands()[0]->type(), OperandNode::Type::REGISTER);
    EXPECT_EQ(instr->operands()[1]->type(), OperandNode::Type::IMMEDIATE);
}

TEST(ParserTest, InstructionWithMemoryAccess) {
    Lexer lexer("CODE\nLDA [HELLO]\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(instr->mnemonic(), "LDA");
    ASSERT_EQ(instr->operands().size(), 1);
    EXPECT_EQ(instr->operands()[0]->type(), OperandNode::Type::MEMORY_ACCESS);
}

TEST(ParserTest, ComplexExpression) {
    Lexer lexer("CODE\nLDA [HELLO + 0x10 + CX]\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    
    auto* instr = dynamic_cast<InstructionNode*>(section->statements()[0].get());
    ASSERT_NE(instr, nullptr);
    ASSERT_EQ(instr->operands().size(), 1);
    
    auto* expr = instr->operands()[0]->expression();
    EXPECT_EQ(expr->type(), ExpressionNode::Type::BINARY_OP);
    EXPECT_EQ(expr->op(), '+');
}

TEST(ParserTest, InlineData) {
    Lexer lexer("CODE\nDB \"inline\"\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    auto* section = dynamic_cast<CodeSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(section, nullptr);
    ASSERT_EQ(section->statements().size(), 1);
    
    auto* inline_data = dynamic_cast<InlineDataNode*>(section->statements()[0].get());
    ASSERT_NE(inline_data, nullptr);
    EXPECT_EQ(inline_data->type(), InlineDataNode::Type::BYTE);
    EXPECT_TRUE(inline_data->is_string());
    EXPECT_EQ(inline_data->string_data(), "inline");
}

TEST(ParserTest, BothSections) {
    Lexer lexer("DATA\nMSG: DB \"Hi\"\nCODE\nHALT\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors());
    ASSERT_EQ(ast->sections().size(), 2);
    
    auto* data_section = dynamic_cast<DataSectionNode*>(ast->sections()[0].get());
    ASSERT_NE(data_section, nullptr);
    EXPECT_EQ(data_section->definitions().size(), 1);
    
    auto* code_section = dynamic_cast<CodeSectionNode*>(ast->sections()[1].get());
    ASSERT_NE(code_section, nullptr);
    EXPECT_EQ(code_section->statements().size(), 1);
}

TEST(ParserTest, CompleteProgram) {
    std::string source = R"(
DATA
HELLO: DB "Hello"

CODE
START:
    LD CX, 0x05
LOOP:
    PUSHB
    DEC CX
    JNZ LOOP
    HALT
)";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    EXPECT_FALSE(parser.has_errors()) << "Parser errors:\n";
    for (const auto& err : parser.errors()) {
        std::cout << err << "\n";
    }
    
    ASSERT_EQ(ast->sections().size(), 2);
}
