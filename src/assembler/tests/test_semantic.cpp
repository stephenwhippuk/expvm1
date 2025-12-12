#include <gtest/gtest.h>
#include "../semantic/symbol_table.h"
#include "../semantic/semantic_analyzer.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"

using namespace lvm::assembler;

TEST(SymbolTableTest, DefineSymbol) {
    SymbolTable table;
    
    EXPECT_TRUE(table.define("LABEL1", SymbolType::LABEL, 1, 1));
    EXPECT_TRUE(table.exists("LABEL1"));
    EXPECT_EQ(table.size(), 1);
}

TEST(SymbolTableTest, DuplicateDefinition) {
    SymbolTable table;
    
    EXPECT_TRUE(table.define("LABEL1", SymbolType::LABEL, 1, 1));
    EXPECT_FALSE(table.define("LABEL1", SymbolType::LABEL, 2, 1));
}

TEST(SymbolTableTest, GetSymbol) {
    SymbolTable table;
    
    table.define("LABEL1", SymbolType::LABEL, 1, 1);
    
    const Symbol* symbol = table.get("LABEL1");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->name, "LABEL1");
    EXPECT_EQ(symbol->type, SymbolType::LABEL);
    EXPECT_EQ(symbol->defined_line, 1);
}

TEST(SymbolTableTest, AddReference) {
    SymbolTable table;
    
    table.define("LABEL1", SymbolType::LABEL, 1, 1);
    table.add_reference("LABEL1", 5, 10, "instruction operand");
    
    const Symbol* symbol = table.get("LABEL1");
    ASSERT_NE(symbol, nullptr);
    ASSERT_EQ(symbol->references.size(), 1);
    EXPECT_EQ(symbol->references[0].line, 5);
    EXPECT_EQ(symbol->references[0].column, 10);
}

TEST(SymbolTableTest, ForwardReference) {
    SymbolTable table;
    
    // Reference before definition
    table.add_reference("FORWARD", 1, 1, "jump target");
    
    // Should create undefined symbol
    EXPECT_TRUE(table.exists("FORWARD"));
    const Symbol* symbol = table.get("FORWARD");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::UNDEFINED);
    
    // Now define it
    EXPECT_TRUE(table.define("FORWARD", SymbolType::LABEL, 10, 1));
    
    // Should be resolved
    symbol = table.get("FORWARD");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::LABEL);
    EXPECT_EQ(symbol->references.size(), 1);
}

TEST(SymbolTableTest, SetAddress) {
    SymbolTable table;
    
    table.define("LABEL1", SymbolType::LABEL, 1, 1);
    EXPECT_TRUE(table.set_address("LABEL1", 0x1000));
    
    const Symbol* symbol = table.get("LABEL1");
    ASSERT_NE(symbol, nullptr);
    EXPECT_TRUE(symbol->address_resolved);
    EXPECT_EQ(symbol->address, 0x1000);
}

TEST(SymbolTableTest, SetSize) {
    SymbolTable table;
    
    table.define("DATA1", SymbolType::DATA_BYTE, 1, 1);
    EXPECT_TRUE(table.set_size("DATA1", 13));
    
    const Symbol* symbol = table.get("DATA1");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->size, 13);
}

TEST(SymbolTableTest, GetUndefinedSymbols) {
    SymbolTable table;
    
    table.add_reference("UNDEFINED1", 1, 1, "test");
    table.add_reference("UNDEFINED2", 2, 1, "test");
    table.define("DEFINED", SymbolType::LABEL, 3, 1);
    
    auto undefined = table.get_undefined_symbols();
    EXPECT_EQ(undefined.size(), 2);
}

TEST(SymbolTableTest, GetSymbolsByType) {
    SymbolTable table;
    
    table.define("LABEL1", SymbolType::LABEL, 1, 1);
    table.define("LABEL2", SymbolType::LABEL, 2, 1);
    table.define("DATA1", SymbolType::DATA_BYTE, 3, 1);
    
    auto labels = table.get_symbols_by_type(SymbolType::LABEL);
    EXPECT_EQ(labels.size(), 2);
    
    auto data = table.get_symbols_by_type(SymbolType::DATA_BYTE);
    EXPECT_EQ(data.size(), 1);
}

TEST(SemanticAnalyzerTest, EmptyProgram) {
    std::string source = "";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
}

TEST(SemanticAnalyzerTest, SimpleLabel) {
    std::string source = "CODE\nSTART:\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    EXPECT_TRUE(table.exists("START"));
    
    const Symbol* symbol = table.get("START");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::LABEL);
}

TEST(SemanticAnalyzerTest, DataDefinition) {
    std::string source = "DATA\nHELLO: DB \"Hello\"\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    EXPECT_TRUE(table.exists("HELLO"));
    
    const Symbol* symbol = table.get("HELLO");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::DATA_BYTE);
    EXPECT_EQ(symbol->size, 5);  // "Hello" = 5 bytes
}

TEST(SemanticAnalyzerTest, DataArray) {
    std::string source = "DATA\nVALUES: DW [1, 2, 3]\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    
    const Symbol* symbol = table.get("VALUES");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::DATA_WORD);
    EXPECT_EQ(symbol->size, 6);  // 3 words * 2 bytes = 6 bytes
}

TEST(SemanticAnalyzerTest, DuplicateLabel) {
    std::string source = "CODE\nLABEL:\nLABEL:\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_FALSE(analyzer.analyze(*ast));
    EXPECT_TRUE(analyzer.has_errors());
    ASSERT_GE(analyzer.errors().size(), 1);
    EXPECT_NE(analyzer.errors()[0].message.find("Duplicate"), std::string::npos);
}

TEST(SemanticAnalyzerTest, ValidRegister) {
    std::string source = "CODE\nLD AX, 0x42\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
}

TEST(SemanticAnalyzerTest, LabelReference) {
    std::string source = "CODE\nSTART:\nJMP START\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    
    const Symbol* symbol = table.get("START");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->references.size(), 1);
}

TEST(SemanticAnalyzerTest, ForwardReference) {
    std::string source = "CODE\nJMP END\nEND:\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    
    const Symbol* symbol = table.get("END");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->type, SymbolType::LABEL);
    EXPECT_EQ(symbol->references.size(), 1);
}

TEST(SemanticAnalyzerTest, UndefinedSymbol) {
    std::string source = "CODE\nJMP NOWHERE\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_FALSE(analyzer.analyze(*ast));
    EXPECT_TRUE(analyzer.has_errors());
    ASSERT_GE(analyzer.errors().size(), 1);
    EXPECT_NE(analyzer.errors()[0].message.find("Undefined"), std::string::npos);
}

TEST(SemanticAnalyzerTest, ComplexExpression) {
    std::string source = "DATA\nDATA1: DB \"test\"\nCODE\nLDA AX, [DATA1 + 2 + CX]\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
    
    const Symbol* symbol = table.get("DATA1");
    ASSERT_NE(symbol, nullptr);
    EXPECT_EQ(symbol->references.size(), 1);
}

TEST(SemanticAnalyzerTest, InlineDataInCodeSection) {
    std::string source = "CODE\nDB \"inline\"\n";
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors());
}

TEST(SemanticAnalyzerTest, CompleteProgram) {
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
    
    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();
    
    SymbolTable table;
    SemanticAnalyzer analyzer(table);
    
    EXPECT_TRUE(analyzer.analyze(*ast));
    EXPECT_FALSE(analyzer.has_errors()) << "Errors found:\n";
    for (const auto& err : analyzer.errors()) {
        std::cout << err.to_string() << "\n";
    }
    
    // Check symbols
    EXPECT_TRUE(table.exists("HELLO"));
    EXPECT_TRUE(table.exists("START"));
    EXPECT_TRUE(table.exists("LOOP"));
    
    const Symbol* hello = table.get("HELLO");
    ASSERT_NE(hello, nullptr);
    EXPECT_EQ(hello->type, SymbolType::DATA_BYTE);
    EXPECT_EQ(hello->size, 13);
    EXPECT_GE(hello->references.size(), 1);
}
