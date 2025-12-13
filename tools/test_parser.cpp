#include "../src/assembler/lexer/lexer.h"
#include "../src/assembler/parser/parser.h"
#include "../src/assembler/parser/ast.h"
#include "../src/assembler/semantic/symbol_table.h"
#include "../src/assembler/semantic/semantic_analyzer.h"
#include "../src/assembler/semantic/instruction_rewriter.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace lvm::assembler;

/**
 * Simple AST printer visitor for debugging
 */
class ASTPrinter : public ASTVisitor {
public:
    void visit(ProgramNode& node) override {
        std::cout << "Program:\n";
        indent_++;
        for (auto& section : node.sections()) {
            section->accept(*this);
        }
        indent_--;
    }
    
    void visit(DataSectionNode& node) override {
        print_indent();
        std::cout << "DATA Section:\n";
        indent_++;
        for (auto& def : node.definitions()) {
            def->accept(*this);
        }
        indent_--;
    }
    
    void visit(CodeSectionNode& node) override {
        print_indent();
        std::cout << "CODE Section:\n";
        indent_++;
        for (auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        indent_--;
    }
    
    void visit(DataDefinitionNode& node) override {
        print_indent();
        std::cout << "Data: " << node.label() << " " 
                  << (node.type() == DataDefinitionNode::Type::BYTE ? "DB" : "DW");
        if (node.is_string()) {
            std::cout << " \"" << node.string_data() << "\"";
        } else {
            std::cout << " [";
            bool first = true;
            for (auto val : node.numeric_data()) {
                if (!first) std::cout << ", ";
                std::cout << "0x" << std::hex << val << std::dec;
                first = false;
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }
    
    void visit(LabelNode& node) override {
        print_indent();
        std::cout << "Label: " << node.name() << "\n";
    }
    
    void visit(InstructionNode& node) override {
        print_indent();
        std::cout << "Instruction: " << node.mnemonic();
        if (!node.operands().empty()) {
            std::cout << " ";
            bool first = true;
            for (auto& operand : node.operands()) {
                if (!first) std::cout << ", ";
                print_operand(*operand);
                first = false;
            }
        }
        std::cout << "\n";
    }
    
    void visit(OperandNode& node) override {
        // Handled by print_operand
    }
    
    void visit(ExpressionNode& node) override {
        // Handled by print_expression
    }
    
    void visit(InlineDataNode& node) override {
        print_indent();
        std::cout << "Inline " << (node.type() == InlineDataNode::Type::BYTE ? "DB" : "DW");
        if (node.is_string()) {
            std::cout << " \"" << node.string_data() << "\"";
        } else {
            std::cout << " [";
            bool first = true;
            for (auto val : node.numeric_data()) {
                if (!first) std::cout << ", ";
                std::cout << "0x" << std::hex << val << std::dec;
                first = false;
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }
    
private:
    int indent_ = 0;
    
    void print_indent() {
        for (int i = 0; i < indent_; i++) {
            std::cout << "  ";
        }
    }
    
    void print_operand(OperandNode& operand) {
        switch (operand.type()) {
            case OperandNode::Type::IMMEDIATE:
                std::cout << "0x" << std::hex << operand.expression()->number() << std::dec;
                break;
            case OperandNode::Type::REGISTER:
                std::cout << operand.expression()->register_name();
                break;
            case OperandNode::Type::IDENTIFIER:
                std::cout << operand.expression()->identifier();
                if (operand.is_sugar_syntax()) {
                    std::cout << " (sugar)";
                }
                break;
            case OperandNode::Type::ADDRESS_EXPR:
                std::cout << "(";
                print_expression(*operand.expression());
                std::cout << ")";
                break;
            case OperandNode::Type::MEMORY_ACCESS:
                std::cout << "[";
                print_expression(*operand.expression());
                std::cout << "]";
                if (operand.is_sugar_syntax()) {
                    std::cout << " (sugar)";
                }
                break;
        }
    }
    
    void print_expression(ExpressionNode& expr) {
        switch (expr.type()) {
            case ExpressionNode::Type::NUMBER:
                std::cout << "0x" << std::hex << expr.number() << std::dec;
                break;
            case ExpressionNode::Type::REGISTER:
                std::cout << expr.register_name();
                break;
            case ExpressionNode::Type::IDENTIFIER:
                std::cout << expr.identifier();
                break;
            case ExpressionNode::Type::BINARY_OP:
                print_expression(*expr.left());
                std::cout << " " << expr.op() << " ";
                print_expression(*expr.right());
                break;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <assembly-file>\n";
        return 1;
    }
    
    // Read source file
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Cannot open file '" << argv[1] << "'\n";
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    std::cout << "=== Parsing: " << argv[1] << " ===\n\n";
    
    try {
        // Tokenize
        Lexer lexer(source);
        std::cout << "=== Tokens ===\n";
        auto tokens = lexer.tokenize();
        for (const auto& token : tokens) {
            if (token.type != TokenType::END_OF_LINE && 
                token.type != TokenType::END_OF_FILE) {
                std::cout << token.to_string() << "\n";
            }
        }
        std::cout << "\n";
        
        // Parse
        Lexer lexer2(source);  // Create new lexer for parsing
        Parser parser(lexer2);
        auto ast = parser.parse();
        
        if (parser.has_errors()) {
            std::cout << "=== Parse Errors ===\n";
            for (const auto& error : parser.errors()) {
                std::cout << error << "\n";
            }
            return 1;
        }
        
        std::cout << "=== AST (Before Rewriting) ===\n";
        ASTPrinter printer;
        ast->accept(printer);
        
        // Rewrite syntactic sugar
        std::cout << "\n=== Rewriting Syntactic Sugar ===\n";
        InstructionRewriter rewriter;
        rewriter.rewrite(*ast);
        
        std::cout << "=== AST (After Rewriting) ===\n";
        ASTPrinter printer2;
        ast->accept(printer2);
        
        // Semantic analysis
        std::cout << "\n=== Semantic Analysis ===\n";
        SymbolTable symbol_table;
        SemanticAnalyzer analyzer(symbol_table);
        
        if (!analyzer.analyze(*ast)) {
            std::cout << "Semantic errors found:\n";
            for (const auto& error : analyzer.errors()) {
                std::cout << "  " << error.to_string() << "\n";
            }
            return 1;
        }
        
        std::cout << "No semantic errors\n\n";
        
        // Print symbol table
        std::cout << "=== Symbol Table ===\n";
        for (const auto& pair : symbol_table.symbols()) {
            const Symbol& sym = pair.second;
            std::cout << "  " << sym.name << ": " 
                      << symbol_type_to_string(sym.type);
            if (sym.size > 0) {
                std::cout << " (size: " << sym.size << " bytes)";
            }
            if (sym.references.size() > 0) {
                std::cout << " (refs: " << sym.references.size() << ")";
            }
            std::cout << "\n";
        }
        
        std::cout << "\n=== Analysis successful ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
