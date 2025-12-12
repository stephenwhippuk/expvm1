#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace lvm {
namespace assembler {

    /**
     * Type of symbol
     */
    enum class SymbolType {
        LABEL,              // Code label
        DATA_BYTE,          // DB definition
        DATA_WORD,          // DW definition
        INLINE_DATA,        // Anonymous inline data
        UNDEFINED           // Forward reference not yet resolved
    };

    /**
     * Scope of symbol
     */
    enum class SymbolScope {
        GLOBAL,             // Accessible everywhere
        LOCAL               // Local to current section
    };

    /**
     * Symbol table entry
     */
    struct Symbol {
        std::string name;
        SymbolType type;
        SymbolScope scope;
        
        // Location information
        size_t defined_line;
        size_t defined_column;
        
        // Address information (resolved in pass 4)
        bool address_resolved;
        uint32_t address;
        
        // Size information (for data symbols)
        uint32_t size;
        
        // References to this symbol
        struct Reference {
            size_t line;
            size_t column;
            std::string context;  // For error messages
        };
        std::vector<Reference> references;
        
        Symbol() 
            : type(SymbolType::UNDEFINED)
            , scope(SymbolScope::GLOBAL)
            , defined_line(0)
            , defined_column(0)
            , address_resolved(false)
            , address(0)
            , size(0) {}
    };

    /**
     * Symbol table for assembler
     * Tracks all labels, data definitions, and their references
     */
    class SymbolTable {
    public:
        SymbolTable() = default;
        
        /**
         * Define a new symbol
         * @return true if successful, false if already defined
         */
        bool define(const std::string& name, SymbolType type, 
                   size_t line, size_t column);
        
        /**
         * Add a reference to a symbol (creates undefined symbol if doesn't exist)
         */
        void add_reference(const std::string& name, size_t line, 
                          size_t column, const std::string& context);
        
        /**
         * Check if symbol exists
         */
        bool exists(const std::string& name) const;
        
        /**
         * Get symbol (returns nullptr if not found)
         */
        Symbol* get(const std::string& name);
        const Symbol* get(const std::string& name) const;
        
        /**
         * Set symbol address (during address resolution pass)
         */
        bool set_address(const std::string& name, uint32_t address);
        
        /**
         * Set symbol size (for data definitions)
         */
        bool set_size(const std::string& name, uint32_t size);
        
        /**
         * Get all undefined symbols (forward references not resolved)
         */
        std::vector<std::string> get_undefined_symbols() const;
        
        /**
         * Get all symbols of a specific type
         */
        std::vector<std::string> get_symbols_by_type(SymbolType type) const;
        
        /**
         * Get all symbols
         */
        const std::unordered_map<std::string, Symbol>& symbols() const {
            return symbols_;
        }
        
        /**
         * Clear all symbols
         */
        void clear() { symbols_.clear(); }
        
        /**
         * Get symbol count
         */
        size_t size() const { return symbols_.size(); }
        
    private:
        std::unordered_map<std::string, Symbol> symbols_;
    };

    /**
     * Convert symbol type to string
     */
    const char* symbol_type_to_string(SymbolType type);

} // namespace assembler
} // namespace lvm
