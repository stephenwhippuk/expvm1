#include "instruction_rewriter.h"

namespace lvm {
namespace assembler {

    void InstructionRewriter::rewrite(ProgramNode& program) {
        program.accept(*this);
    }

    void InstructionRewriter::visit(ProgramNode& node) {
        for (auto& section : node.sections()) {
            section->accept(*this);
        }
    }

    void InstructionRewriter::visit(DataSectionNode& node) {
        // No rewriting needed in data section
    }

    void InstructionRewriter::visit(PageDirectiveNode& node) {
        // No rewriting needed for page directives
    }

    void InstructionRewriter::visit(CodeSectionNode& node) {
        for (auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
    }

    void InstructionRewriter::visit(DataDefinitionNode& node) {
        // Not in code section
    }

    void InstructionRewriter::visit(LabelNode& node) {
        // Labels don't need rewriting
    }

    void InstructionRewriter::visit(InstructionNode& node) {
        std::string mnemonic = node.mnemonic();
        std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);
        
        // Rewrite sugar syntax: LD with label[expr] → LDA/LDAB based on register size
        // LD AL, label[expr] → LDAB AL, (label + expr)  (8-bit register)
        // LD AX, label[expr] → LDA AX, (label + expr)   (16-bit register)
        // LD label[expr], AL → LDAB (label + expr), AL  (8-bit register)
        if (mnemonic == "LD") {
            for (auto& operand : node.operands()) {
                if (operand->type() == OperandNode::Type::MEMORY_ACCESS && 
                    operand->is_sugar_syntax()) {
                    
                    // Find the register operand to determine instruction size
                    std::string reg_name;
                    for (auto& op : node.operands()) {
                        if (op->type() == OperandNode::Type::REGISTER) {
                            if (op->expression() && 
                                op->expression()->type() == ExpressionNode::Type::REGISTER) {
                                reg_name = op->expression()->register_name();
                                break;
                            }
                        }
                    }
                    
                    if (!reg_name.empty()) {
                        std::transform(reg_name.begin(), reg_name.end(), reg_name.begin(), ::toupper);
                        
                        // Check if it's an 8-bit register (AL, AH, BL, BH, CL, CH, DL, DH, EL, EH)
                        bool is_8bit = (reg_name.length() == 2 && 
                                       (reg_name[1] == 'L' || reg_name[1] == 'H') &&
                                       (reg_name[0] == 'A' || reg_name[0] == 'B' || reg_name[0] == 'C' || 
                                        reg_name[0] == 'D' || reg_name[0] == 'E'));
                        
                        if (is_8bit) {
                            node.set_mnemonic("LDAB");
                        } else {
                            node.set_mnemonic("LDA");
                        }
                    } else {
                        // Default to LDA if we can't determine register size
                        node.set_mnemonic("LDA");
                    }
                    
                    // Change operand type from MEMORY_ACCESS to ADDRESS_EXPR
                    operand->set_type(OperandNode::Type::ADDRESS_EXPR);
                    break;
                }
            }
        }
    }

    void InstructionRewriter::visit(OperandNode& node) {
        // Operands handled in InstructionNode visitor
    }

    void InstructionRewriter::visit(ExpressionNode& node) {
        // Expressions don't need rewriting
    }

    void InstructionRewriter::visit(InlineDataNode& node) {
        // Inline data doesn't need rewriting
    }

} // namespace assembler
} // namespace lvm
