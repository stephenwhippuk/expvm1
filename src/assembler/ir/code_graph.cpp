#include "code_graph.h"
#include <algorithm>
#include <stdexcept>

namespace lvm {
namespace assembler {

    // Helper function to convert register name to byte code
    // NOTE: CPU uses 1-based register codes (AX=1, BX=2, etc.)
    static uint8_t register_name_to_code(const std::string& name) {
        std::string upper = name;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        if (upper == "AX" || upper == "AL" || upper == "AH") return 0x01;
        if (upper == "BX" || upper == "BL" || upper == "BH") return 0x02;
        if (upper == "CX" || upper == "CL" || upper == "CH") return 0x03;
        if (upper == "DX" || upper == "DL" || upper == "DH") return 0x04;
        if (upper == "EX" || upper == "EL" || upper == "EH") return 0x05;
        
        throw std::runtime_error("Unknown register name: " + name);
    }

    uint32_t CodeInstructionNode::size() const {
        // Opcode (1 byte) + operands
        uint32_t total = 1;
        
        for (const auto& operand : operands_) {
            switch (operand.type) {
                case InstructionOperand::Type::IMMEDIATE_BYTE:
                    total += 1;
                    break;
                case InstructionOperand::Type::IMMEDIATE_WORD:
                    total += 2;
                    break;
                case InstructionOperand::Type::ADDRESS:
                    total += 4;  // 32-bit address
                    break;
                case InstructionOperand::Type::REGISTER:
                    // Registers are encoded as 1 byte
                    total += 1;
                    break;
                case InstructionOperand::Type::EXPRESSION:
                    // Complex expressions become addresses after resolution
                    total += 4;
                    break;
            }
        }
        
        return total;
    }

    std::vector<uint8_t> CodeInstructionNode::encode() const {
        std::vector<uint8_t> bytes;
        bytes.push_back(opcode_);
        
        for (const auto& operand : operands_) {
            switch (operand.type) {
                case InstructionOperand::Type::IMMEDIATE_BYTE:
                    bytes.push_back(static_cast<uint8_t>(operand.immediate_value));
                    break;
                    
                case InstructionOperand::Type::IMMEDIATE_WORD:
                    // Little-endian
                    bytes.push_back(static_cast<uint8_t>(operand.immediate_value & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((operand.immediate_value >> 8) & 0xFF));
                    break;
                    
                case InstructionOperand::Type::ADDRESS:
                case InstructionOperand::Type::EXPRESSION:
                    // 32-bit address, little-endian
                    bytes.push_back(static_cast<uint8_t>(operand.address & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((operand.address >> 8) & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((operand.address >> 16) & 0xFF));
                    bytes.push_back(static_cast<uint8_t>((operand.address >> 24) & 0xFF));
                    break;
                    
                case InstructionOperand::Type::REGISTER:
                    // Encode register as 1 byte
                    bytes.push_back(register_name_to_code(operand.register_name));
                    break;
            }
        }
        
        return bytes;
    }

    uint32_t CodeGraph::data_segment_size() const {
        uint32_t total = 0;
        for (const auto& block : data_blocks_) {
            total += block->size();
        }
        return total;
    }

    uint32_t CodeGraph::code_segment_size() const {
        uint32_t total = 0;
        for (const auto& node : code_nodes_) {
            total += node->size();
        }
        return total;
    }

} // namespace assembler
} // namespace lvm
