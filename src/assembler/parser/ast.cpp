#include "ast.h"

namespace lvm {
namespace assembler {

    // Visitor acceptors for each node type
    
    void ProgramNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void DataSectionNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void CodeSectionNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void DataDefinitionNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void LabelNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void InstructionNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void OperandNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void ExpressionNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

    void InlineDataNode::accept(ASTVisitor& visitor) {
        visitor.visit(*this);
    }

} // namespace assembler
} // namespace lvm
