#ifndef CONTEXT_H
#define CONTEXT_H

#include "vaddr.h"
#include "accessMode.h"
#include "memsize.h"
#include <cstdint>
#include <memory>

namespace lvm {
    class IVMemUnit;  // Forward declaration
    class PagedMemoryAccessor;  // Forward declaration
    class StackMemoryAccessor;  // Forward declaration

// Context: represents a contiguous virtual address space up to 4GB
// - Each context has a unique ID
// - Occupies a region in the 40-bit virtual address space
// - Physical memory is allocated on-demand by VMemUnit
// - Has an associated accessor type that defines format/interface
// - Accessors are created on-demand in PROTECTED mode and should not be held
class Context {
public:
    // Delete copy operations
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    
    // Allow move operations
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;
    
    ~Context() = default;
    
    // Create a paged memory accessor for this context
    // - Only allowed in PROTECTED mode
    // - Accessor should be used immediately and discarded, not stored
    std::unique_ptr<PagedMemoryAccessor> create_paged_accessor(MemAccessMode mode) const;
    
    // Create a Stack accessor for this context
    // - Only allowed in PROTECTED mode
    // - Pre-allocates all physical memory for the stack
    // - Accessor should be used immediately and discarded, not stored
    std::unique_ptr<StackMemoryAccessor> create_stack_accessor() const;

private:
    friend class VMemUnit;             // Can access context internals and construct
    friend class PagedMemoryAccessor;  // Can access page state
    friend class StackMemoryAccessor;  // Can access for construction
    
    // Constructor - only VMemUnit can create contexts
    Context(IVMemUnit& vmem_unit, context_id_t id, vaddr_t base_address, uint32_t size);
    
    // Accessors (private - accessed by VMemUnit and accessors)
    context_id_t get_id() const { return id_; }
    vaddr_t get_base_address() const { return base_address_; }
    uint32_t get_size() const { return size_; }
    vaddr_t get_end_address() const { return base_address_ + size_; }
    
    // Check if a virtual address falls within this context
    bool contains(vaddr_t addr) const;
    
    // Page management for paged memory access (private - accessed via PagedMemoryAccessor)
    void set_current_page(page_t page) { current_page_ = page; }
    page_t get_current_page() const { return current_page_; }
    
    IVMemUnit& vmem_unit_;  // Reference to VMemUnit (VMemUnit outlives Context)
    context_id_t id_;
    vaddr_t base_address_;  // Starting address in 40-bit space
    uint32_t size_;         // Size in bytes (max 4GB)
    page_t current_page_;   // Current page for paged memory access
};

}  // namespace lvm

#endif // CONTEXT_H
