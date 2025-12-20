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

    void PageDirectiveNode::accept(ASTVisitor& visitor) {
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

    // DataSectionNode legacy accessor implementation
    std::vector<DataDefinitionNode*> DataSectionNode::definitions() const {
        std::vector<DataDefinitionNode*> defs;
        for (const auto& item : items_) {
            if (auto* def = dynamic_cast<DataDefinitionNode*>(item.get())) {
                defs.push_back(def);
            }
        }
        return defs;
    }

} // namespace assembler
} // namespace lvm
