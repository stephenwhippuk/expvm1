#pragma once

#include "../ir/code_graph.h"
#include "../semantic/symbol_table.h"
#include <string>
#include <vector>

namespace lvm {
namespace assembler {

    /**
     * Address resolver (Pass 4)
     * 
     * Calculates absolute addresses for all symbols and resolves references:
     * - Data segment starts at 0x0000
     * - Code segment starts after data segment
     * - Labels get code addresses
     * - Data definitions get data addresses
     * - Expression operands get resolved to absolute addresses
     */
    class AddressResolver {
    public:
        AddressResolver(SymbolTable& symbol_table, CodeGraph& graph);
        
        /**
         * Resolve all addresses
         * @return true if successful
         */
        bool resolve();
        
        /**
         * Get errors
         */
        const std::vector<std::string>& errors() const { return errors_; }
        bool has_errors() const { return !errors_.empty(); }
        
        /**
         * Get the starting address of code segment
         */
        uint32_t code_segment_start() const { return code_segment_start_; }
        
    private:
        SymbolTable& symbol_table_;
        CodeGraph& graph_;
        std::vector<std::string> errors_;
        
        uint32_t code_segment_start_;
        
        void error(const std::string& message);
        void resolve_data_addresses();
        void resolve_code_addresses();
        void resolve_operand_addresses();
        uint32_t resolve_expression(const InstructionOperand& operand);
    };

} // namespace assembler
} // namespace lvm
