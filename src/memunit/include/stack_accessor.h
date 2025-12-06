#ifndef STACK_ACCESSOR_H
#define STACK_ACCESSOR_H

#include "memsize.h"
#include "accessMode.h"
#include "vaddr.h"
#include <cstdint>
#include <memory>
#include <vector>

class Context;  // Forward declaration (global namespace)

namespace lvm {
    class VMemUnit;  // Forward declaration

    // StackAccessor: Provides direct 32-bit addressing for stack memory
    // - Uses simple 32-bit addressing (no page translation)
    // - Pre-allocates all physical memory upfront
    // - Provides byte and word read/write operations
    // - Designed for stack operations with predictable memory usage
    class StackAccessor {
    public:
        ~StackAccessor() = default;

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
        friend class ::Context;   // Context can construct accessors
        StackAccessor(VMemUnit& vmem_unit, context_id_t context_id, addr32_t size);

        VMemUnit& vmem_unit_;
        context_id_t context_id_;
        addr32_t size_;  // Total size of stack
    };
}

#endif // STACK_ACCESSOR_H
