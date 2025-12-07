#ifndef STACK_MEMORY_ACCESSOR_H
#define STACK_MEMORY_ACCESSOR_H

#include "memsize.h"
#include "accessMode.h"
#include "vaddr.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace lvm {
    class Context;  // Forward declaration
    class IVMemUnit;  // Forward declaration

    // StackMemoryAccessor: Provides direct 32-bit addressing for stack memory
    // - Uses simple 32-bit addressing (no page translation)
    // - Pre-allocates all physical memory upfront
    // - Provides byte and word read/write operations
    // - Designed for stack operations with predictable memory usage
    class StackMemoryAccessor {
    public:
        ~StackMemoryAccessor() = default;

        // Byte operations (using 32-bit address)
        byte_t read_byte(addr32_t address) const;
        void write_byte(addr32_t address, byte_t value);

        // Word operations (little-endian, using 32-bit address)
        word_t read_word(addr32_t address) const;
        void write_word(addr32_t address, word_t value);

        // Get context information
        context_id_t get_context_id() const { return context_id_; }
        addr32_t get_size() const { return size_; }

    private:
        friend class VMemUnit;  // VMemUnit can access internals
        friend class Context;   // Context can construct accessors
        StackMemoryAccessor(const Context& context);

        const Context& context_;
        context_id_t context_id_;
        addr32_t size_;  // Total size of stack
    };
}

#endif // STACK_MEMORY_ACCESSOR_H
