#pragma once

#include "../parser/ast.h"
#include <algorithm>
#include <cctype>

namespace lvm {
namespace assembler {

    /**
     * Rewrites syntactic sugar in instructions
     * 
     * Specifically handles:
     * - LD reg, label[expr] → LDA reg, (label + expr)
     */
    class InstructionRewriter : public ASTVisitor {
    public:
        InstructionRewriter() = default;
        
        void rewrite(ProgramNode& program);
        
        // Visitor methods
        void visit(ProgramNode& node) override;
        void visit(DataSectionNode& node) override;
        void visit(CodeSectionNode& node) override;
        void visit(DataDefinitionNode& node) override;
        void visit(LabelNode& node) override;
        void visit(InstructionNode& node) override;
        void visit(OperandNode& node) override;
        void visit(ExpressionNode& node) override;
        void visit(InlineDataNode& node) override;
    };

} // namespace assembler
} // namespace lvm
