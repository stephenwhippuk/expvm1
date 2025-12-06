#ifndef CONTEXT_H
#define CONTEXT_H

#include "vaddr.h"
#include "accessMode.h"
#include "memsize.h"
#include <cstdint>
#include <memory>

namespace lvm {
    class VMemUnit;  // Forward declaration
    class PagedMemoryAccessor;  // Forward declaration
    class StackAccessor;  // Forward declaration
}

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
    std::unique_ptr<lvm::PagedMemoryAccessor> create_paged_accessor(
        lvm::VMemUnit& vmem_unit, lvm::MemAccessMode mode) const;
    
    // Create a Stack accessor for this context
    // - Only allowed in PROTECTED mode
    // - Pre-allocates all physical memory for the stack
    // - Accessor should be used immediately and discarded, not stored
    std::unique_ptr<lvm::StackAccessor> create_stack_accessor(
        lvm::VMemUnit& vmem_unit) const;

private:
    friend class lvm::VMemUnit;             // Can access context internals and construct
    friend class lvm::PagedMemoryAccessor;  // Can access page state
    friend class lvm::StackAccessor;        // Can access for construction
    
    // Constructor - only VMemUnit can create contexts
    Context(context_id_t id, vaddr_t base_address, uint32_t size);
    
    // Test friends
    friend class ContextTest_Creation_Test;
    friend class ContextTest_Contains_Test;
    friend class ContextTest_InvalidAddress_Test;
    friend class ContextTest_Overflow_Test;
    friend class VMemUnitTest_InitialState_Test;
    friend class VMemUnitTest_ModeSwitch_Test;
    friend class VMemUnitTest_CreateContextUnprotected_Test;
    friend class VMemUnitTest_CreateContextProtectedFails_Test;
    friend class VMemUnitTest_CreateMultipleContexts_Test;
    friend class VMemUnitTest_DestroyContext_Test;
    friend class VMemUnitTest_DestroyContextProtectedFails_Test;
    friend class VMemUnitTest_DestroyNonExistentContext_Test;
    friend class VMemUnitTest_GetNonExistentContext_Test;
    friend class VMemUnitTest_CreateContextZeroSize_Test;
    friend class VMemUnitTest_FindContextForAddress_Test;
    
    // Accessors (private - accessed by VMemUnit and accessors)
    context_id_t get_id() const { return id_; }
    vaddr_t get_base_address() const { return base_address_; }
    uint32_t get_size() const { return size_; }
    vaddr_t get_end_address() const { return base_address_ + size_; }
    
    // Check if a virtual address falls within this context
    bool contains(vaddr_t addr) const;
    
    // Page management for paged memory access (private - accessed via PagedMemoryAccessor)
    void set_current_page(lvm::page_t page) { current_page_ = page; }
    lvm::page_t get_current_page() const { return current_page_; }
    
    context_id_t id_;
    vaddr_t base_address_;  // Starting address in 40-bit space
    uint32_t size_;         // Size in bytes (max 4GB)
    lvm::page_t current_page_;   // Current page for paged memory access
};

#endif // CONTEXT_H
