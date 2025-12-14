#include "code_graph_builder.h"
#include <algorithm>
#include <cctype>

namespace lvm {
namespace assembler {

    CodeGraphBuilder::CodeGraphBuilder(SymbolTable& symbol_table)
        : symbol_table_(symbol_table)
        , in_data_section_(false)
        , in_code_section_(false)
        , anonymous_counter_(0) {
    }

    std::unique_ptr<CodeGraph> CodeGraphBuilder::build(ProgramNode& program) {
        errors_.clear();
        graph_ = std::make_unique<CodeGraph>();
        anonymous_counter_ = 0;
        
        program.accept(*this);
        
        if (has_errors()) {
            return nullptr;
        }
        
        return std::move(graph_);
    }

    void CodeGraphBuilder::visit(ProgramNode& node) {
        for (auto& section : node.sections()) {
            section->accept(*this);
        }
    }

    void CodeGraphBuilder::visit(DataSectionNode& node) {
        in_data_section_ = true;
        in_code_section_ = false;
        
        for (auto& def : node.definitions()) {
            def->accept(*this);
        }
        
        in_data_section_ = false;
    }

    void CodeGraphBuilder::visit(CodeSectionNode& node) {
        in_data_section_ = false;
        in_code_section_ = true;
        
        for (auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        
        in_code_section_ = false;
    }

    void CodeGraphBuilder::visit(DataDefinitionNode& node) {
        // Convert data definition to bytes
        auto bytes = data_definition_to_bytes(node);
        
        // Prepend size word (little-endian)
        uint16_t size = static_cast<uint16_t>(bytes.size());
        std::vector<uint8_t> sized_bytes;
        sized_bytes.push_back(static_cast<uint8_t>(size & 0xFF));
        sized_bytes.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
        sized_bytes.insert(sized_bytes.end(), bytes.begin(), bytes.end());
        
        // Create data block
        auto block = std::make_unique<DataBlockNode>(node.label(), sized_bytes);
        graph_->add_data_block(std::move(block));
    }

    void CodeGraphBuilder::visit(LabelNode& node) {
        // Create label node
        auto label = std::make_unique<CodeLabelNode>(node.name());
        graph_->add_code_node(std::move(label));
    }

    void CodeGraphBuilder::visit(InstructionNode& node) {
        // Get opcode for instruction
        uint8_t opcode = get_opcode_for_instruction(node.mnemonic());
        
        // Store current instruction mnemonic for operand processing
        current_instruction_mnemonic_ = node.mnemonic();
        
        // Create instruction node
        auto instr = std::make_unique<CodeInstructionNode>(node.mnemonic(), opcode);
        
        // Process operands
        for (auto& operand : node.operands()) {
            current_operand_ = InstructionOperand();
            operand->accept(*this);
            instr->add_operand(current_operand_);
        }
        
        graph_->add_code_node(std::move(instr));
    }

    void CodeGraphBuilder::visit(OperandNode& node) {
        switch (node.type()) {
            case OperandNode::Type::IMMEDIATE:
                // Determine if byte or word based on instruction semantics, not value
                if (instruction_expects_word_immediate(current_instruction_mnemonic_)) {
                    current_operand_.type = InstructionOperand::Type::IMMEDIATE_WORD;
                    current_operand_.immediate_value = static_cast<uint16_t>(node.expression()->number());
                } else if (instruction_expects_byte_immediate(current_instruction_mnemonic_)) {
                    current_operand_.type = InstructionOperand::Type::IMMEDIATE_BYTE;
                    current_operand_.immediate_value = static_cast<uint16_t>(node.expression()->number());
                } else {
                    // Fallback to value-based determination for ambiguous cases
                    if (node.expression()->number() <= 0xFF) {
                        current_operand_.type = InstructionOperand::Type::IMMEDIATE_BYTE;
                        current_operand_.immediate_value = static_cast<uint16_t>(node.expression()->number());
                    } else {
                        current_operand_.type = InstructionOperand::Type::IMMEDIATE_WORD;
                        current_operand_.immediate_value = static_cast<uint16_t>(node.expression()->number());
                    }
                }
                break;
                
            case OperandNode::Type::REGISTER:
                current_operand_.type = InstructionOperand::Type::REGISTER;
                current_operand_.register_name = node.expression()->register_name();
                break;
                
            case OperandNode::Type::IDENTIFIER:
                // Symbol reference - will be resolved to address
                current_operand_.type = InstructionOperand::Type::ADDRESS;
                current_operand_.symbol_name = node.expression()->identifier();
                break;
                
            case OperandNode::Type::ADDRESS_EXPR:
                // Parentheses: (expression) - address computation
                current_operand_.type = InstructionOperand::Type::EXPRESSION;
                node.expression()->accept(*this);
                break;
                
            case OperandNode::Type::MEMORY_ACCESS:
                // Square brackets: [expression] - memory dereference
                current_operand_.type = InstructionOperand::Type::EXPRESSION;
                node.expression()->accept(*this);
                break;
                
            case OperandNode::Type::INLINE_DATA:
                // Inline data creates anonymous data block and uses its address
                if (node.inline_data()) {
                    // Create anonymous data block
                    auto bytes = inline_data_to_bytes(*node.inline_data());
                    
                    // Prepend size word (little-endian)
                    uint16_t size = static_cast<uint16_t>(bytes.size());
                    std::vector<uint8_t> sized_bytes;
                    sized_bytes.push_back(static_cast<uint8_t>(size & 0xFF));
                    sized_bytes.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
                    sized_bytes.insert(sized_bytes.end(), bytes.begin(), bytes.end());
                    
                    std::string label = "__anon_" + std::to_string(anonymous_counter_++);
                    
                    auto block = std::make_unique<DataBlockNode>(label, sized_bytes);
                    graph_->add_data_block(std::move(block));
                    
                    // Use the label as an address reference
                    current_operand_.type = InstructionOperand::Type::ADDRESS;
                    current_operand_.symbol_name = label;
                }
                break;
        }
    }

    void CodeGraphBuilder::visit(ExpressionNode& node) {
        // Build expression for address calculation
        switch (node.type()) {
            case ExpressionNode::Type::IDENTIFIER:
                current_operand_.symbol_name = node.identifier();
                current_operand_.offset = 0;
                break;
                
            case ExpressionNode::Type::NUMBER:
                current_operand_.offset = static_cast<int32_t>(node.number());
                break;
                
            case ExpressionNode::Type::REGISTER:
                current_operand_.offset_register = node.register_name();
                break;
                
            case ExpressionNode::Type::BINARY_OP:
                // Recursively process: LABEL + offset + register
                if (node.left()) {
                    node.left()->accept(*this);
                }
                if (node.right()) {
                    if (node.right()->type() == ExpressionNode::Type::NUMBER) {
                        int32_t right_val = static_cast<int32_t>(node.right()->number());
                        if (node.op() == '+') {
                            current_operand_.offset += right_val;
                        } else if (node.op() == '-') {
                            current_operand_.offset -= right_val;
                        }
                    } else if (node.right()->type() == ExpressionNode::Type::REGISTER) {
                        current_operand_.offset_register = node.right()->register_name();
                    } else {
                        node.right()->accept(*this);
                    }
                }
                break;
        }
    }

    void CodeGraphBuilder::visit(InlineDataNode& node) {
        // Create anonymous data block
        auto bytes = inline_data_to_bytes(node);
        
        // Prepend size word (little-endian)
        uint16_t size = static_cast<uint16_t>(bytes.size());
        std::vector<uint8_t> sized_bytes;
        sized_bytes.push_back(static_cast<uint8_t>(size & 0xFF));
        sized_bytes.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
        sized_bytes.insert(sized_bytes.end(), bytes.begin(), bytes.end());
        
        std::string label = "__anon_" + std::to_string(anonymous_counter_++);
        
        auto block = std::make_unique<DataBlockNode>(label, sized_bytes);
        graph_->add_data_block(std::move(block));
        
        // TODO: Need to handle inline data differently - it should be in code segment
        // For now, treating as data block
    }

    void CodeGraphBuilder::error(const std::string& message, size_t line, size_t column) {
        errors_.emplace_back(message, line, column);
    }

    std::vector<uint8_t> CodeGraphBuilder::data_definition_to_bytes(const DataDefinitionNode& node) {
        if (node.is_string()) {
            return string_to_bytes(node.string_data());
        } else {
            std::vector<uint8_t> bytes;
            bool is_word = (node.type() == DataDefinitionNode::Type::WORD);
            
            for (uint64_t value : node.numeric_data()) {
                if (is_word) {
                    // Little-endian word
                    bytes.push_back(static_cast<uint8_t>(value & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
                } else {
                    // Byte
                    bytes.push_back(static_cast<uint8_t>(value & 0xFF));
                }
            }
            
            return bytes;
        }
    }

    std::vector<uint8_t> CodeGraphBuilder::inline_data_to_bytes(const InlineDataNode& node) {
        if (node.is_string()) {
            return string_to_bytes(node.string_data());
        } else {
            std::vector<uint8_t> bytes;
            bool is_word = (node.type() == InlineDataNode::Type::WORD);
            
            for (uint64_t value : node.numeric_data()) {
                if (is_word) {
                    bytes.push_back(static_cast<uint8_t>(value & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
                } else {
                    bytes.push_back(static_cast<uint8_t>(value & 0xFF));
                }
            }
            
            return bytes;
        }
    }

    std::vector<uint8_t> CodeGraphBuilder::string_to_bytes(const std::string& str) {
        return std::vector<uint8_t>(str.begin(), str.end());
    }

    uint8_t CodeGraphBuilder::get_opcode_for_instruction(const std::string& mnemonic) {
        // Convert to uppercase for comparison
        std::string upper = mnemonic;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        // Map instruction mnemonics to opcodes
        // Based on Pendragon VM Operation Reference
        
        // Control Flow
        if (upper == "NOP") return 0x00;
        if (upper == "HALT") return 0x01;
        
        // Load/Store - Register operations
        if (upper == "LD") return 0x02;      // Multiple variants - disambiguation needed
        if (upper == "SWP") return 0x04;
        if (upper == "LDH") return 0x05;     // Multiple variants
        if (upper == "LDL") return 0x07;     // Multiple variants
        if (upper == "LDA") return 0x09;     // Multiple variants
        if (upper == "LDAB") return 0x0A;
        if (upper == "LDAH") return 0x0B;    // Multiple variants
        if (upper == "LDAL") return 0x0C;    // Multiple variants
        
        // Stack operations
        if (upper == "PUSH") return 0x10;
        if (upper == "PUSHH") return 0x11;
        if (upper == "PUSHL") return 0x12;
        if (upper == "POP") return 0x13;
        if (upper == "POPH") return 0x14;
        if (upper == "POPL") return 0x15;
        if (upper == "PEEK") return 0x16;
        if (upper == "PEEKF") return 0x17;
        if (upper == "PEEKB") return 0x18;
        if (upper == "PEEKFB") return 0x19;
        if (upper == "FLSH") return 0x1A;
        
        // Memory management
        if (upper == "PAGE") return 0x1B;    // Multiple variants
        if (upper == "SETF") return 0x1D;
        
        // Jumps
        if (upper == "JMP") return 0x1E;
        if (upper == "JPZ" || upper == "JZ") return 0x1F;
        if (upper == "JPNZ" || upper == "JNZ") return 0x20;
        if (upper == "JPC") return 0x21;
        if (upper == "JPNC") return 0x22;
        if (upper == "JPS") return 0x23;
        if (upper == "JPNS") return 0x24;
        if (upper == "JPO") return 0x25;
        if (upper == "JPNO") return 0x26;
        
        // Procedure calls
        if (upper == "CALL") return 0x27;
        if (upper == "RET") return 0x28;
        
        // Arithmetic - ADD
        if (upper == "ADD") return 0x29;     // Multiple variants
        if (upper == "ADB") return 0x2B;
        if (upper == "ADH") return 0x2C;
        if (upper == "ADL") return 0x2D;
        
        // Arithmetic - SUB
        if (upper == "SUB") return 0x2E;     // Multiple variants
        if (upper == "SBB") return 0x30;
        if (upper == "SBH") return 0x31;
        if (upper == "SBL") return 0x32;
        
        // Arithmetic - MUL
        if (upper == "MUL") return 0x33;     // Multiple variants
        if (upper == "MLB") return 0x35;
        if (upper == "MLH") return 0x36;
        if (upper == "MLL") return 0x37;
        
        // Arithmetic - DIV
        if (upper == "DIV") return 0x38;     // Multiple variants
        if (upper == "DVB") return 0x3A;
        if (upper == "DVH") return 0x3B;
        if (upper == "DVL") return 0x3C;
        
        // Arithmetic - REM
        if (upper == "REM") return 0x3D;     // Multiple variants
        if (upper == "RMB") return 0x3F;
        if (upper == "RMH") return 0x40;
        if (upper == "RML") return 0x41;
        
        // Logical - AND
        if (upper == "AND") return 0x42;     // Multiple variants
        if (upper == "ANB") return 0x44;
        if (upper == "ANH") return 0x45;
        if (upper == "ANL") return 0x46;
        
        // Logical - OR
        if (upper == "OR") return 0x47;      // Multiple variants
        if (upper == "ORB") return 0x49;
        if (upper == "ORH") return 0x4A;
        if (upper == "ORL") return 0x4B;
        
        // Logical - XOR
        if (upper == "XOR") return 0x4C;     // Multiple variants
        if (upper == "XOB") return 0x4E;
        if (upper == "XOH") return 0x4F;
        if (upper == "XOL") return 0x50;
        
        // Logical - NOT
        if (upper == "NOT") return 0x51;     // Multiple variants
        if (upper == "NOTB") return 0x53;
        if (upper == "NOTH") return 0x54;
        if (upper == "NOTL") return 0x55;
        
        // Shift left
        if (upper == "SHL") return 0x56;     // Multiple variants
        if (upper == "SLB") return 0x58;
        if (upper == "SLH") return 0x59;
        if (upper == "SLL") return 0x5A;
        
        // Shift right
        if (upper == "SHR") return 0x5B;     // Multiple variants
        if (upper == "SHRB") return 0x5D;
        if (upper == "SHRH") return 0x5E;
        if (upper == "SHRL") return 0x5F;
        
        // Rotate left
        if (upper == "ROL") return 0x60;     // Multiple variants
        if (upper == "ROLB") return 0x62;
        if (upper == "ROLH") return 0x63;
        if (upper == "ROLL") return 0x64;
        
        // Rotate right
        if (upper == "ROR") return 0x65;     // Multiple variants
        if (upper == "RORB") return 0x67;
        if (upper == "RORH") return 0x68;
        if (upper == "RORL") return 0x69;
        
        // Increment/Decrement
        if (upper == "INC") return 0x6A;
        if (upper == "DEC") return 0x6B;
        
        // Compare
        if (upper == "CMP") return 0x6C;     // Multiple variants
        if (upper == "CPH") return 0x6E;     // Multiple variants
        if (upper == "CPL") return 0x70;     // Multiple variants
        
        // Stack push immediate
        if (upper == "PUSHW") return 0x75;
        if (upper == "PUSHB") return 0x76;
        
        // System call
        if (upper == "SYSCALL" || upper == "SYS") return 0x7F;
        
        // For unknown instructions, return 0x00 (NOP)
        // In production, this should log a warning or error
        return 0x00;
    }

    bool CodeGraphBuilder::instruction_expects_word_immediate(const std::string& mnemonic) const {
        std::string upper = mnemonic;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        // Instructions that always take word immediates
        return (upper == "LD" ||      // LD reg, immediate16
                upper == "PUSHW" ||   // PUSHW immediate16
                upper == "SYS" ||     // SYS immediate16
                upper == "SYSCALL" ||
                upper == "ADD" ||     // ADD AX, immediate16
                upper == "SUB" ||     // SUB AX, immediate16
                upper == "MUL" ||     // MUL AX, immediate16
                upper == "DIV" ||     // DIV AX, immediate16
                upper == "REM" ||     // REM AX, immediate16
                upper == "AND" ||     // AND AX, immediate16
                upper == "OR" ||      // OR AX, immediate16
                upper == "XOR" ||     // XOR AX, immediate16
                upper == "NOT" ||     // NOT immediate16
                upper == "SHL" ||     // SHL AX, immediate16
                upper == "SHR" ||     // SHR AX, immediate16
                upper == "ROL" ||     // ROL AX, immediate16
                upper == "ROR" ||     // ROR AX, immediate16
                upper == "CMP" ||     // CMP reg, immediate16
                upper == "PAGE");     // PAGE immediate16
    }

    bool CodeGraphBuilder::instruction_expects_byte_immediate(const std::string& mnemonic) const {
        std::string upper = mnemonic;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        // Instructions that always take byte immediates
        return (upper == "LDH" ||     // LDH reg, immediate8
                upper == "LDL" ||     // LDL reg, immediate8
                upper == "PUSHB" ||   // PUSHB immediate8
                upper == "ADB" ||     // ADB AX, immediate8
                upper == "SBB" ||     // SBB AX, immediate8
                upper == "MLB" ||     // MLB AX, immediate8
                upper == "DVB" ||     // DVB AX, immediate8
                upper == "RMB" ||     // RMB AX, immediate8
                upper == "ANB" ||     // ANB AL, immediate8
                upper == "ORB" ||     // ORB AL, immediate8
                upper == "XOB" ||     // XOB AL, immediate8
                upper == "NOTB" ||    // NOTB immediate8
                upper == "SLB" ||     // SLB AX, immediate8
                upper == "SHRB" ||    // SHRB AX, immediate8
                upper == "ROLB" ||    // ROLB AX, immediate8
                upper == "RORB");     // RORB AX, immediate8
    }

} // namespace assembler
} // namespace lvm

