#include "code_graph.h"

namespace lvm {
namespace assembler {

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
                    // Registers are typically encoded in the opcode or as 1 byte
                    // For now, assume no additional bytes (handled by opcode variants)
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
                    // Register encoding handled by opcode selection
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
