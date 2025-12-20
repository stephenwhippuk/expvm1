#include <gtest/gtest.h>
#include "../ir/code_graph.h"
#include "../ir/code_graph_builder.h"
#include "../codegen/address_resolver.h"
#include "../semantic/symbol_table.h"
#include "../semantic/semantic_analyzer.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

using namespace lvm::assembler;

TEST(CodeGraphTest, DataBlock) {
    std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    DataBlockNode block("HELLO", data);
    
    EXPECT_EQ(block.label(), "HELLO");
    EXPECT_EQ(block.size(), 5);
    EXPECT_FALSE(block.is_anonymous());
}

TEST(CodeGraphTest, AnonymousDataBlock) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    DataBlockNode block("", data);
    
    EXPECT_TRUE(block.is_anonymous());
    EXPECT_EQ(block.size(), 3);
}

TEST(CodeGraphTest, CodeInstruction) {
    CodeInstructionNode instr("HALT", 0x01);
    
    EXPECT_EQ(instr.mnemonic(), "HALT");
    EXPECT_EQ(instr.opcode(), 0x01);
    EXPECT_EQ(instr.size(), 1);  // Just opcode, no operands
}

TEST(CodeGraphTest, InstructionWithOperand) {
    CodeInstructionNode instr("PUSHB", 0x76);
    
    InstructionOperand operand;
    operand.type = InstructionOperand::Type::IMMEDIATE_BYTE;
    operand.immediate_value = 0x42;
    instr.add_operand(operand);
    
    EXPECT_EQ(instr.size(), 2);  // Opcode + byte operand
}

TEST(CodeGraphTest, CodeLabel) {
    CodeLabelNode label("START");
    
    EXPECT_EQ(label.name(), "START");
    EXPECT_EQ(label.size(), 0);  // Labels don't take space
}

TEST(CodeGraphBuilderTest, SimpleDataSection) {
    std::string source = "DATA\nHELLO: DB \"Hi\"\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    ASSERT_NE(graph, nullptr);
    EXPECT_FALSE(builder.has_errors());
    EXPECT_EQ(graph->data_blocks().size(), 1);
    EXPECT_EQ(graph->data_blocks()[0]->label(), "HELLO");
    EXPECT_EQ(graph->data_blocks()[0]->size(), 4);  // 2 bytes data + 2 byte size prefix
}

TEST(CodeGraphBuilderTest, DataArray) {
    std::string source = "DATA\nVALUES: DW [0x100, 0x200]\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->data_blocks().size(), 1);
    EXPECT_EQ(graph->data_blocks()[0]->size(), 6);  // 2 words (4 bytes) + 2 byte size prefix
}

TEST(CodeGraphBuilderTest, SimpleCodeSection) {
    std::string source = "CODE\nHALT\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->code_nodes().size(), 1);
}

TEST(CodeGraphBuilderTest, LabelAndInstruction) {
    std::string source = "CODE\nSTART:\nHALT\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->code_nodes().size(), 2);  // Label + instruction
}

TEST(AddressResolverTest, SimpleDataAddress) {
    std::string source = "DATA\nDATA1: DB [1,2,3]\nDATA2: DB [4,5]\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    AddressResolver resolver(table, *graph);
    EXPECT_TRUE(resolver.resolve());
    
    // DATA1 should be at 0x0000
    const Symbol* data1 = table.get("DATA1");
    ASSERT_NE(data1, nullptr);
    EXPECT_TRUE(data1->address_resolved);
    EXPECT_EQ(data1->address, 0x0000);
    
    // DATA2 should be at 0x0005 (after DATA1's 3 bytes + 2 byte size prefix)
    const Symbol* data2 = table.get("DATA2");
    ASSERT_NE(data2, nullptr);
    EXPECT_TRUE(data2->address_resolved);
    EXPECT_EQ(data2->address, 0x0005);
}

TEST(AddressResolverTest, CodeAfterData) {
    std::string source = "DATA\nDATA1: DB [1,2,3,4,5]\nCODE\nSTART:\nHALT\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    AddressResolver resolver(table, *graph);
    EXPECT_TRUE(resolver.resolve());
    
    // Code segment has separate address space starting at 0
    EXPECT_EQ(resolver.code_segment_start(), 0);
    
    // START label is at offset 0 in code segment
    const Symbol* start = table.get("START");
    ASSERT_NE(start, nullptr);
    EXPECT_TRUE(start->address_resolved);
    EXPECT_EQ(start->address, 0x0000);
}

TEST(AddressResolverTest, MultipleLabelAddresses) {
    std::string source = "CODE\nLABEL1:\nPUSHB 0x42\nLABEL2:\nHALT\n";
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    analyzer.analyze(*ast);
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    
    AddressResolver resolver(table, *graph);
    EXPECT_TRUE(resolver.resolve());
    
    // LABEL1 at 0x0000
    const Symbol* label1 = table.get("LABEL1");
    ASSERT_NE(label1, nullptr);
    EXPECT_EQ(label1->address, 0x0000);
    
    // LABEL2 at 0x0002 (after PUSHB which is 2 bytes)
    const Symbol* label2 = table.get("LABEL2");
    ASSERT_NE(label2, nullptr);
    EXPECT_EQ(label2->address, 0x0002);
}

TEST(IntegrationTest, CompleteProgram) {
    std::string source = R"(
DATA
HELLO: DB "Hello, World!"

CODE
START:
    LD CX, 0xB
LOOP:
    DEC CX
    LDA AX, [HELLO + CX]
    PUSHB AX
    CMP CX, 0x00
    JNZ LOOP
    PUSHW 0x000B
    SYS 0x0011
    HALT
)";
    
    // Parse
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    EXPECT_FALSE(parser.has_errors());
    
    // Semantic analysis
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    EXPECT_TRUE(analyzer.analyze(*ast));
    
    // Build code graph
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    ASSERT_NE(graph, nullptr);
    EXPECT_FALSE(builder.has_errors());
    
    // Resolve addresses
    AddressResolver resolver(table, *graph);
    EXPECT_TRUE(resolver.resolve());
    
    // Verify addresses
    const Symbol* hello = table.get("HELLO");
    ASSERT_NE(hello, nullptr);
    EXPECT_EQ(hello->address, 0x0000);
    EXPECT_EQ(hello->size, 13);  // Symbol size is the data size (without size prefix)
    
    // Code segment has separate address space starting at 0
    EXPECT_EQ(resolver.code_segment_start(), 0);
    
    const Symbol* start = table.get("START");
    ASSERT_NE(start, nullptr);
    EXPECT_EQ(start->address, 0);  // First code label is at address 0
    
    const Symbol* loop = table.get("LOOP");
    ASSERT_NE(loop, nullptr);
    EXPECT_GT(loop->address, start->address);
}

TEST(InstructionEncodingTest, SimpleHalt) {
    CodeInstructionNode instr("HALT", 0x01);
    auto bytes = instr.encode();
    
    ASSERT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 0x01);
}

TEST(InstructionEncodingTest, PushByte) {
    CodeInstructionNode instr("PUSHB", 0x76);
    
    InstructionOperand operand;
    operand.type = InstructionOperand::Type::IMMEDIATE_BYTE;
    operand.immediate_value = 0x42;
    instr.add_operand(operand);
    
    auto bytes = instr.encode();
    
    ASSERT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x76);  // Opcode
    EXPECT_EQ(bytes[1], 0x42);  // Operand
}

TEST(InstructionEncodingTest, PushWord) {
    CodeInstructionNode instr("PUSHW", 0x75);
    
    InstructionOperand operand;
    operand.type = InstructionOperand::Type::IMMEDIATE_WORD;
    operand.immediate_value = 0x1234;
    instr.add_operand(operand);
    
    auto bytes = instr.encode();
    
    ASSERT_EQ(bytes.size(), 3);
    EXPECT_EQ(bytes[0], 0x75);  // Opcode
    EXPECT_EQ(bytes[1], 0x34);  // Low byte (little-endian)
    EXPECT_EQ(bytes[2], 0x12);  // High byte
}

// PAGE directive tests
TEST(CodeGraphBuilderTest, PageInstructionInjection) {
    // Program with data on multiple pages
    Lexer lexer("DATA\nPAGE page1\nvar1: DB [1]\nPAGE page2\nvar2: DB [2]\n\nCODE\n    LDA AX, var1\n    LDA BX, var2\n    HALT\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    ASSERT_NE(ast, nullptr);
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    ASSERT_TRUE(analyzer.analyze(*ast));
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    ASSERT_NE(graph, nullptr);
    
    // Verify code nodes: should be PAGE, LDA, PAGE, LDA, HALT
    const auto& code_nodes = graph->code_nodes();
    ASSERT_GE(code_nodes.size(), 5);
    
    // Check first PAGE instruction
    auto* page1 = dynamic_cast<CodeInstructionNode*>(code_nodes[0].get());
    ASSERT_NE(page1, nullptr);
    EXPECT_EQ(page1->mnemonic(), "PAGE");
    EXPECT_EQ(page1->opcode(), 0x1B);
    
    // Check second PAGE instruction (after first LDA)
    auto* page2 = dynamic_cast<CodeInstructionNode*>(code_nodes[2].get());
    ASSERT_NE(page2, nullptr);
    EXPECT_EQ(page2->mnemonic(), "PAGE");
}

TEST(CodeGraphBuilderTest, PageInstructionNotInjectedForSamePage) {
    // Two references to same page should only inject PAGE once
    Lexer lexer("DATA\nPAGE page1\nvar1: DB [1]\nvar2: DB [2]\n\nCODE\n    LDA AX, var1\n    LDA BX, var2\n    HALT\n");
    Parser parser(lexer);
    auto ast = parser.parse();
    ASSERT_NE(ast, nullptr);
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    ASSERT_TRUE(analyzer.analyze(*ast));
    
    CodeGraphBuilder builder(table);
    auto graph = builder.build(*ast);
    ASSERT_NE(graph, nullptr);
    
    // Verify code nodes: should be PAGE, LDA, LDA (no second PAGE), HALT
    const auto& code_nodes = graph->code_nodes();
    ASSERT_GE(code_nodes.size(), 4);
    
    // First node should be PAGE
    auto* page1 = dynamic_cast<CodeInstructionNode*>(code_nodes[0].get());
    ASSERT_NE(page1, nullptr);
    EXPECT_EQ(page1->mnemonic(), "PAGE");
    
    // Second node should be LDA (not PAGE)
    auto* lda1 = dynamic_cast<CodeInstructionNode*>(code_nodes[1].get());
    ASSERT_NE(lda1, nullptr);
    EXPECT_EQ(lda1->mnemonic(), "LDA");
}

TEST(CodeGraphBuilderTest, PageInstructionEncoding) {
    // Test that PAGE instruction encodes correctly
    CodeInstructionNode page_instr("PAGE", 0x1B);
    
    // Page number = 1
    InstructionOperand page_op;
    page_op.type = InstructionOperand::Type::IMMEDIATE_WORD;
    page_op.immediate_value = 1;
    page_instr.add_operand(page_op);
    
    // Context = 0
    InstructionOperand ctx_op;
    ctx_op.type = InstructionOperand::Type::IMMEDIATE_WORD;
    ctx_op.immediate_value = 0;
    page_instr.add_operand(ctx_op);
    
    auto bytes = page_instr.encode();
    
    ASSERT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 0x1B);  // Opcode
    EXPECT_EQ(bytes[1], 0x01);  // Page low byte
    EXPECT_EQ(bytes[2], 0x00);  // Page high byte
    EXPECT_EQ(bytes[3], 0x00);  // Context low byte
    EXPECT_EQ(bytes[4], 0x00);  // Context high byte
}

