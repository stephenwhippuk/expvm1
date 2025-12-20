#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace lvm {
namespace assembler {

    /**
     * Base class for code graph nodes
     */
    class CodeGraphNode {
    public:
        virtual ~CodeGraphNode() = default;
        
        /**
         * Get the size of this node in bytes
         */
        virtual uint32_t size() const = 0;
        
        /**
         * Get the address of this node (set during address resolution)
         */
        uint32_t address() const { return address_; }
        void set_address(uint32_t addr) { address_ = addr; }
        
        /**
         * Check if address has been resolved
         */
        bool has_address() const { return address_resolved_; }
        
    protected:
        uint32_t address_ = 0;
        bool address_resolved_ = false;
    };

    /**
     * Data block node (from DATA section or inline data)
     */
    class DataBlockNode : public CodeGraphNode {
    public:
        DataBlockNode(const std::string& label, const std::vector<uint8_t>& data)
            : label_(label), data_(data) {}
        
        const std::string& label() const { return label_; }
        const std::vector<uint8_t>& data() const { return data_; }
        std::vector<uint8_t>& mutable_data() { return data_; }
        
        uint32_t size() const override {
            return static_cast<uint32_t>(data_.size());
        }
        
        bool is_anonymous() const { return label_.empty(); }
        
        // For DA (address arrays) - store label references for later resolution
        void set_address_references(const std::vector<std::string>& refs) {
            address_references_ = refs;
            is_address_array_ = true;
        }
        
        bool is_address_array() const { return is_address_array_; }
        const std::vector<std::string>& address_references() const { return address_references_; }
        
    private:
        std::string label_;
        std::vector<uint8_t> data_;
        bool is_address_array_ = false;
        std::vector<std::string> address_references_;
    };

    /**
     * Instruction operand
     */
    struct InstructionOperand {
        enum class Type {
            IMMEDIATE_BYTE,     // 8-bit immediate value
            IMMEDIATE_WORD,     // 16-bit immediate value
            REGISTER,           // Register reference
            ADDRESS,            // Absolute address (resolved from label)
            EXPRESSION          // Complex expression (needs resolution)
        };
        
        Type type;
        uint16_t immediate_value;   // For IMMEDIATE_BYTE/WORD
        std::string register_name;  // For REGISTER
        uint32_t address;           // For ADDRESS (resolved)
        std::string symbol_name;    // For EXPRESSION/ADDRESS (symbol reference)
        int32_t offset;             // For EXPRESSION (constant offset)
        std::string offset_register;// For EXPRESSION (register offset)
        
        InstructionOperand() : type(Type::IMMEDIATE_BYTE), immediate_value(0), address(0), offset(0) {}
    };

    /**
     * Code instruction node
     */
    class CodeInstructionNode : public CodeGraphNode {
    public:
        CodeInstructionNode(const std::string& mnemonic, uint8_t opcode)
            : mnemonic_(mnemonic), opcode_(opcode) {}
        
        const std::string& mnemonic() const { return mnemonic_; }
        uint8_t opcode() const { return opcode_; }
        
        void add_operand(const InstructionOperand& operand) {
            operands_.push_back(operand);
        }
        
        const std::vector<InstructionOperand>& operands() const { return operands_; }
        std::vector<InstructionOperand>& operands() { return operands_; }
        
        uint32_t size() const override;
        
        /**
         * Encode instruction to bytes (for code generation)
         */
        std::vector<uint8_t> encode() const;
        
    private:
        std::string mnemonic_;
        uint8_t opcode_;
        std::vector<InstructionOperand> operands_;
    };

    /**
     * Label node (marks a position in code)
     */
    class CodeLabelNode : public CodeGraphNode {
    public:
        explicit CodeLabelNode(const std::string& name) : name_(name) {}
        
        const std::string& name() const { return name_; }
        
        uint32_t size() const override { return 0; }  // Labels don't take space
        
    private:
        std::string name_;
    };

    /**
     * Code graph - intermediate representation of the program
     * 
     * Separates DATA and CODE sections with layout:
     * - Data segment starts at 0x0000
     * - Code segment starts after data segment
     */
    class CodeGraph {
    public:
        CodeGraph() = default;
        
        /**
         * Add a data block
         */
        void add_data_block(std::unique_ptr<DataBlockNode> block) {
            data_blocks_.push_back(std::move(block));
        }
        
        /**
         * Add a code node (instruction or label)
         */
        void add_code_node(std::unique_ptr<CodeGraphNode> node) {
            code_nodes_.push_back(std::move(node));
        }
        
        /**
         * Get data blocks
         */
        const std::vector<std::unique_ptr<DataBlockNode>>& data_blocks() const {
            return data_blocks_;
        }
        
        std::vector<std::unique_ptr<DataBlockNode>>& data_blocks() {
            return data_blocks_;
        }
        
        /**
         * Get code nodes
         */
        const std::vector<std::unique_ptr<CodeGraphNode>>& code_nodes() const {
            return code_nodes_;
        }
        
        std::vector<std::unique_ptr<CodeGraphNode>>& code_nodes() {
            return code_nodes_;
        }
        
        /**
         * Calculate total data segment size
         */
        uint32_t data_segment_size() const;
        
        /**
         * Calculate total code segment size
         */
        uint32_t code_segment_size() const;
        
    private:
        std::vector<std::unique_ptr<DataBlockNode>> data_blocks_;
        std::vector<std::unique_ptr<CodeGraphNode>> code_nodes_;
    };

} // namespace assembler
} // namespace lvm
