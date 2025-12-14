#include "symbol_table.h"

namespace lvm {
namespace assembler {

    const char* symbol_type_to_string(SymbolType type) {
        switch (type) {
            case SymbolType::LABEL: return "Label";
            case SymbolType::DATA_BYTE: return "Data (DB)";
            case SymbolType::DATA_WORD: return "Data (DW)";
            case SymbolType::INLINE_DATA: return "Inline Data";
            case SymbolType::UNDEFINED: return "Undefined";
            default: return "Unknown";
        }
    }

    bool SymbolTable::define(const std::string& name, SymbolType type, 
                             size_t line, size_t column) {
        auto it = symbols_.find(name);
        
        // If symbol already defined, check if it was previously undefined (forward ref)
        if (it != symbols_.end()) {
            if (it->second.type != SymbolType::UNDEFINED) {
                return false;  // Already defined
            }
            
            // Resolve forward reference
            it->second.type = type;
            it->second.defined_line = line;
            it->second.defined_column = column;
            return true;
        }
        
        // Create new symbol
        Symbol symbol;
        symbol.name = name;
        symbol.type = type;
        symbol.defined_line = line;
        symbol.defined_column = column;
        
        symbols_[name] = symbol;
        return true;
    }

    void SymbolTable::add_reference(const std::string& name, size_t line,
                                    size_t column, const std::string& context) {
        auto it = symbols_.find(name);
        
        if (it == symbols_.end()) {
            // Create undefined symbol for forward reference
            Symbol symbol;
            symbol.name = name;
            symbol.type = SymbolType::UNDEFINED;
            symbols_[name] = symbol;
            it = symbols_.find(name);
        }
        
        // Add reference
        Symbol::Reference ref;
        ref.line = line;
        ref.column = column;
        ref.context = context;
        it->second.references.push_back(ref);
    }

    bool SymbolTable::exists(const std::string& name) const {
        return symbols_.find(name) != symbols_.end();
    }

    Symbol* SymbolTable::get(const std::string& name) {
        auto it = symbols_.find(name);
        return (it != symbols_.end()) ? &it->second : nullptr;
    }

    const Symbol* SymbolTable::get(const std::string& name) const {
        auto it = symbols_.find(name);
        return (it != symbols_.end()) ? &it->second : nullptr;
    }

    bool SymbolTable::set_address(const std::string& name, uint32_t address) {
        auto it = symbols_.find(name);
        if (it == symbols_.end()) {
            // Auto-create symbol for anonymous data blocks (e.g., __anon_0)
            if (name.find("__anon_") == 0) {
                Symbol symbol;
                symbol.name = name;
                symbol.type = SymbolType::INLINE_DATA;
                symbol.scope = SymbolScope::GLOBAL;
                symbol.address = address;
                symbol.address_resolved = true;
                symbols_[name] = symbol;
                return true;
            }
            return false;
        }
        
        it->second.address = address;
        it->second.address_resolved = true;
        return true;
    }

    bool SymbolTable::set_size(const std::string& name, uint32_t size) {
        auto it = symbols_.find(name);
        if (it == symbols_.end()) {
            return false;
        }
        
        it->second.size = size;
        return true;
    }

    std::vector<std::string> SymbolTable::get_undefined_symbols() const {
        std::vector<std::string> undefined;
        for (const auto& pair : symbols_) {
            if (pair.second.type == SymbolType::UNDEFINED) {
                undefined.push_back(pair.first);
            }
        }
        return undefined;
    }

    std::vector<std::string> SymbolTable::get_symbols_by_type(SymbolType type) const {
        std::vector<std::string> result;
        for (const auto& pair : symbols_) {
            if (pair.second.type == type) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

} // namespace assembler
} // namespace lvm
