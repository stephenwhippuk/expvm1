#pragma once

#include "../lexer/token.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace lvm {
namespace assembler {

    // Forward declarations
    class ASTVisitor;
    class ASTNode;
    class ProgramNode;
    class SectionNode;
    class DataSectionNode;
    class CodeSectionNode;
    class PageDirectiveNode;
    class DataDefinitionNode;
    class LabelNode;
    class InstructionNode;
    class OperandNode;
    class ExpressionNode;
    class InlineDataNode;

    /**
     * Base class for all AST nodes
     */
    class ASTNode {
    public:
        virtual ~ASTNode() = default;
        
        /**
         * Accept visitor for traversal (Visitor pattern)
         */
        virtual void accept(ASTVisitor& visitor) = 0;
        
        /**
         * Get source location for error reporting
         */
        size_t line() const { return line_; }
        size_t column() const { return column_; }
        void set_location(size_t line, size_t column) {
            line_ = line;
            column_ = column;
        }
        
    protected:
        size_t line_ = 0;
        size_t column_ = 0;
    };

    /**
     * Root node representing entire assembly program
     */
    class ProgramNode : public ASTNode {
    public:
        void add_section(std::unique_ptr<SectionNode> section) {
            sections_.push_back(std::move(section));
        }
        
        const std::vector<std::unique_ptr<SectionNode>>& sections() const {
            return sections_;
        }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::vector<std::unique_ptr<SectionNode>> sections_;
    };

    /**
     * Base class for DATA and CODE sections
     */
    class SectionNode : public ASTNode {
    public:
        enum class Type { DATA, CODE };
        
        explicit SectionNode(Type type) : type_(type) {}
        Type type() const { return type_; }
        
    protected:
        Type type_;
    };

    /**
     * DATA section containing data definitions
     */
    class DataSectionNode : public SectionNode {
    public:
        DataSectionNode() : SectionNode(Type::DATA) {}
        
        void add_item(std::unique_ptr<ASTNode> item) {
            items_.push_back(std::move(item));
        }
        
        const std::vector<std::unique_ptr<ASTNode>>& items() const {
            return items_;
        }
        
        // Legacy accessor for compatibility - implemented in ast.cpp
        std::vector<DataDefinitionNode*> definitions() const;
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::vector<std::unique_ptr<ASTNode>> items_;  // PageDirectiveNode or DataDefinitionNode
    };

    /**
     * Page directive (PAGE pageName)
     */
    class PageDirectiveNode : public ASTNode {
    public:
        explicit PageDirectiveNode(const std::string& name) : name_(name) {}
        
        const std::string& name() const { return name_; }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::string name_;
    };

    /**
     * CODE section containing instructions and labels
     */
    class CodeSectionNode : public SectionNode {
    public:
        CodeSectionNode() : SectionNode(Type::CODE) {}
        
        void add_statement(std::unique_ptr<ASTNode> statement) {
            statements_.push_back(std::move(statement));
        }
        
        const std::vector<std::unique_ptr<ASTNode>>& statements() const {
            return statements_;
        }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::vector<std::unique_ptr<ASTNode>> statements_;  // Labels, Instructions, InlineData
    };

    /**
     * Data definition (DB or DW directive)
     */
    class DataDefinitionNode : public ASTNode {
    public:
        enum class Type { BYTE, WORD, ADDRESS };  // DB, DW, or DA
        
        DataDefinitionNode(const std::string& label, Type type)
            : label_(label), type_(type) {}
        
        const std::string& label() const { return label_; }
        Type type() const { return type_; }
        
        // For DB "string" or DW "string"
        void set_string_data(const std::string& data) {
            string_data_ = data;
            is_string_ = true;
        }
        
        // For DB [1,2,3] or DW [1,2,3]
        void add_numeric_value(uint64_t value) {
            numeric_data_.push_back(value);
            is_string_ = false;
        }
        
        // For DA [label1, label2, ...]
        void add_label_reference(const std::string& label) {
            label_references_.push_back(label);
            is_string_ = false;
        }
        
        bool has_label_references() const { return !label_references_.empty(); }
        const std::vector<std::string>& label_references() const { return label_references_; }
        
        bool is_string() const { return is_string_; }
        const std::string& string_data() const { return string_data_; }
        const std::vector<uint64_t>& numeric_data() const { return numeric_data_; }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::string label_;
        Type type_;
        bool is_string_ = false;
        std::string string_data_;
        std::vector<uint64_t> numeric_data_;
        std::vector<std::string> label_references_;  // For DA
    };

    /**
     * Label definition (LABEL:)
     */
    class LabelNode : public ASTNode {
    public:
        explicit LabelNode(const std::string& name) : name_(name) {}
        
        const std::string& name() const { return name_; }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::string name_;
    };

    /**
     * Expression node for offset calculations: (LABEL + offset + register)
     */
    class ExpressionNode : public ASTNode {
    public:
        enum class Type { 
            IDENTIFIER,     // Just a label/symbol
            NUMBER,         // Literal number
            REGISTER,       // Register name
            BINARY_OP       // label + offset, etc.
        };
        
        explicit ExpressionNode(Type type) : type_(type) {}
        
        Type type() const { return type_; }
        
        // For IDENTIFIER
        void set_identifier(const std::string& name) { identifier_ = name; }
        const std::string& identifier() const { return identifier_; }
        
        // For NUMBER
        void set_number(uint64_t value) { number_ = value; }
        uint64_t number() const { return number_; }
        
        // For REGISTER
        void set_register(const std::string& reg) { register_name_ = reg; }
        const std::string& register_name() const { return register_name_; }
        
        // For BINARY_OP
        void set_left(std::unique_ptr<ExpressionNode> left) { left_ = std::move(left); }
        void set_right(std::unique_ptr<ExpressionNode> right) { right_ = std::move(right); }
        void set_operator(char op) { operator_ = op; }
        
        ExpressionNode* left() const { return left_.get(); }
        ExpressionNode* right() const { return right_.get(); }
        char op() const { return operator_; }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        Type type_;
        std::string identifier_;
        uint64_t number_ = 0;
        std::string register_name_;
        char operator_ = 0;
        std::unique_ptr<ExpressionNode> left_;
        std::unique_ptr<ExpressionNode> right_;
    };

    /**
     * Operand for an instruction
     */
    class OperandNode : public ASTNode {
    public:
        enum class Type {
            IMMEDIATE,      // Literal number
            REGISTER,       // Register name
            ADDRESS_EXPR,   // Parentheses (expression) - address computation
            MEMORY_ACCESS,  // Square brackets [expression] - memory dereference
            IDENTIFIER,     // Label reference
            INLINE_DATA     // Inline DB/DW data definition
        };
        
        explicit OperandNode(Type type) : type_(type), is_sugar_syntax_(false) {}
        
        Type type() const { return type_; }
        void set_type(Type type) { type_ = type; }
        
        void set_expression(std::unique_ptr<ExpressionNode> expr) {
            expression_ = std::move(expr);
        }
        
        ExpressionNode* expression() const { return expression_.get(); }
        
        // Sugar syntax tracking (for label[expr] → LDA conversion)
        bool is_sugar_syntax() const { return is_sugar_syntax_; }
        void set_sugar_syntax(bool value) { is_sugar_syntax_ = value; }
        
        // Inline data support (for DB/DW as operands)
        void set_inline_data(std::unique_ptr<InlineDataNode> data) {
            inline_data_ = std::move(data);
        }
        InlineDataNode* inline_data() const { return inline_data_.get(); }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        Type type_;
        std::unique_ptr<ExpressionNode> expression_;
        std::unique_ptr<InlineDataNode> inline_data_;
        bool is_sugar_syntax_;  // True if this came from label[index] syntax
    };

    /**
     * Instruction node
     */
    class InstructionNode : public ASTNode {
    public:
        explicit InstructionNode(const std::string& mnemonic)
            : mnemonic_(mnemonic) {}
        
        const std::string& mnemonic() const { return mnemonic_; }
        void set_mnemonic(const std::string& mnemonic) { mnemonic_ = mnemonic; }
        
        void add_operand(std::unique_ptr<OperandNode> operand) {
            operands_.push_back(std::move(operand));
        }
        
        std::vector<std::unique_ptr<OperandNode>>& operands() {
            return operands_;
        }
        
        const std::vector<std::unique_ptr<OperandNode>>& operands() const {
            return operands_;
        }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        std::string mnemonic_;
        std::vector<std::unique_ptr<OperandNode>> operands_;
    };

    /**
     * Inline data definition in CODE section (DB "string")
     */
    class InlineDataNode : public ASTNode {
    public:
        enum class Type { BYTE, WORD };
        
        InlineDataNode(Type type, const std::string& data)
            : type_(type), string_data_(data), is_string_(true) {}
            
        InlineDataNode(Type type, const std::vector<uint64_t>& data)
            : type_(type), numeric_data_(data), is_string_(false) {}
        
        Type type() const { return type_; }
        bool is_string() const { return is_string_; }
        const std::string& string_data() const { return string_data_; }
        const std::vector<uint64_t>& numeric_data() const { return numeric_data_; }
        
        // Page specification for inline data (optional)
        void set_page_name(const std::string& page) { page_name_ = page; }
        const std::string& page_name() const { return page_name_; }
        bool has_page_name() const { return !page_name_.empty(); }
        
        void accept(ASTVisitor& visitor) override;
        
    private:
        Type type_;
        std::string string_data_;
        std::vector<uint64_t> numeric_data_;
        bool is_string_;
        std::string page_name_;  // Optional page name for inline data
    };

    /**
     * Visitor interface for traversing AST
     * (Enables adding new operations without modifying node classes)
     */
    class ASTVisitor {
    public:
        virtual ~ASTVisitor() = default;
        
        virtual void visit(ProgramNode& node) = 0;
        virtual void visit(DataSectionNode& node) = 0;
        virtual void visit(CodeSectionNode& node) = 0;
        virtual void visit(PageDirectiveNode& node) = 0;
        virtual void visit(DataDefinitionNode& node) = 0;
        virtual void visit(LabelNode& node) = 0;
        virtual void visit(InstructionNode& node) = 0;
        virtual void visit(OperandNode& node) = 0;
        virtual void visit(ExpressionNode& node) = 0;
        virtual void visit(InlineDataNode& node) = 0;
    };

} // namespace assembler
} // namespace lvm
