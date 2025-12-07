#ifndef PAGED_MEMORY_ACCESSOR_H
#define PAGED_MEMORY_ACCESSOR_H

#include "memsize.h"
#include "accessMode.h"
#include "vaddr.h"
#include <memory>
#include <vector>
#include <cstdint>

namespace lvm {
    class Context;  // Forward declaration
    class IVMemUnit;  // Forward declaration

    // PagedMemoryAccessor: Provides page+address access to a context's virtual memory
    // - Translates 16-bit page + 16-bit address into 32-bit virtual address
    // - Manages on-demand allocation of physical memory through VMemUnit
    // - Compatible with existing code that uses page-based addressing
    class PagedMemoryAccessor {
    public:
        ~PagedMemoryAccessor() = default;

        // Page management (delegates to context)
        void set_page(page_t page);
        page_t get_page() const;

        // Byte operations (using current page from context + offset)
        byte_t read_byte(addr_t offset) const;
        void write_byte(addr_t offset, byte_t value);

        // Word operations (little-endian, using current page from context + offset)
        word_t read_word(addr_t offset) const;
        void write_word(addr_t offset, word_t value);

        // Bulk operations
        void bulk_read(addr_t offset, std::vector<byte_t>& buffer, memsize_t size) const;
        void bulk_write(addr_t offset, const std::vector<byte_t>& data);

        // Get context information
        context_id_t get_context_id() const { return context_id_; }
        uint32_t get_context_size() const { return context_size_; }

    private:
        friend class VMemUnit;  // VMemUnit can access internals
        friend class Context;   // Context can construct accessors
        PagedMemoryAccessor(Context& context, MemAccessMode mode);

        // Convert page + offset to 32-bit virtual address within context
        uint32_t page_offset_to_address(page_t page, addr_t offset) const;

        Context& context_;        // Reference to context (Context outlives accessor)
        context_id_t context_id_;
        uint32_t context_size_;     // Total size of context (max 4GB)
        MemAccessMode mode_;
    };
}

#endif // PAGED_MEMORY_ACCESSOR_H
