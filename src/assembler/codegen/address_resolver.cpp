#include "address_resolver.h"

namespace lvm {
namespace assembler {

    AddressResolver::AddressResolver(SymbolTable& symbol_table, CodeGraph& graph)
        : symbol_table_(symbol_table)
        , graph_(graph)
        , code_segment_start_(0) {
    }

    bool AddressResolver::resolve() {
        errors_.clear();
        
        // Pass 1: Resolve data block addresses
        resolve_data_addresses();
        
        // Pass 2: Resolve code addresses (labels and instructions)
        resolve_code_addresses();
        
        // Pass 3: Resolve operand expressions
        resolve_operand_addresses();
        
        return !has_errors();
    }

    void AddressResolver::resolve_data_addresses() {
        uint32_t current_address = 0;
        
        for (auto& block : graph_.data_blocks()) {
            block->set_address(current_address);
            
            // Update symbol table (including anonymous blocks with generated labels)
            symbol_table_.set_address(block->label(), current_address);
            
            current_address += block->size();
        }
        
        // Code segment starts after data segment
        code_segment_start_ = current_address;
    }

    void AddressResolver::resolve_code_addresses() {
        uint32_t current_address = code_segment_start_;
        
        for (auto& node : graph_.code_nodes()) {
            node->set_address(current_address);
            
            // If it's a label, update symbol table
            if (auto* label = dynamic_cast<CodeLabelNode*>(node.get())) {
                symbol_table_.set_address(label->name(), current_address);
            }
            
            current_address += node->size();
        }
    }

    void AddressResolver::resolve_operand_addresses() {
        for (auto& node : graph_.code_nodes()) {
            auto* instr = dynamic_cast<CodeInstructionNode*>(node.get());
            if (!instr) continue;
            
            for (auto& operand : instr->operands()) {
                if (operand.type == InstructionOperand::Type::ADDRESS) {
                    // Simple symbol reference
                    const Symbol* symbol = symbol_table_.get(operand.symbol_name);
                    if (!symbol) {
                        error("Undefined symbol '" + operand.symbol_name + "'");
                        continue;
                    }
                    if (!symbol->address_resolved) {
                        error("Symbol '" + operand.symbol_name + "' address not resolved");
                        continue;
                    }
                    operand.address = symbol->address;
                    
                } else if (operand.type == InstructionOperand::Type::EXPRESSION) {
                    // Complex expression: LABEL + offset + register
                    operand.address = resolve_expression(operand);
                }
            }
        }
    }

    uint32_t AddressResolver::resolve_expression(const InstructionOperand& operand) {
        uint32_t base_address = 0;
        
        // Get base address from symbol
        if (!operand.symbol_name.empty()) {
            const Symbol* symbol = symbol_table_.get(operand.symbol_name);
            if (!symbol) {
                error("Undefined symbol '" + operand.symbol_name + "'");
                return 0;
            }
            if (!symbol->address_resolved) {
                error("Symbol '" + operand.symbol_name + "' address not resolved");
                return 0;
            }
            base_address = symbol->address;
        }
        
        // Add constant offset
        base_address += operand.offset;
        
        // Register offset handled at runtime
        // For now, we just return the base + constant
        // Runtime will add register value
        
        return base_address;
    }

    void AddressResolver::error(const std::string& message) {
        errors_.push_back(message);
    }

} // namespace assembler
} // namespace lvm
