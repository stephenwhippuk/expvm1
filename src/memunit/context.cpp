#include "context.h"
#include "vmemunit.h"
#include "paged_memory_accessor.h"
#include "stack_accessor.h"
#include "errors.h"
#include <stdexcept>

lvm::Context::Context(IVMemUnit& vmem_unit, context_id_t id, vaddr_t base_address, uint32_t size)
    : vmem_unit_(vmem_unit), id_(id), base_address_(base_address), size_(size), current_page_(0) {
    
    // Validate that base address is within 40-bit range
    if (!is_valid_vaddr(base_address)) {
        throw std::invalid_argument("Base address exceeds 40-bit virtual address space");
    }
    
    // Validate that the context doesn't overflow the 40-bit space
    vaddr_t end_address = base_address + static_cast<vaddr_t>(size);
    if (!is_valid_vaddr(end_address) || end_address < base_address) {
        throw std::invalid_argument("Context size causes overflow of 40-bit virtual address space");
    }
}

bool lvm::Context::contains(vaddr_t addr) const {
    return addr >= base_address_ && addr < get_end_address();
}

std::unique_ptr<lvm::PagedMemoryAccessor> lvm::Context::create_paged_accessor(MemAccessMode mode) const {
    
    // Accessor creation only allowed in protected mode
    if (!vmem_unit_.is_protected()) {
        throw std::runtime_error("Cannot create accessor in UNPROTECTED mode");
    }
    
    // Need to cast away const since accessor needs mutable access to page state
    return std::unique_ptr<PagedMemoryAccessor>(
        new PagedMemoryAccessor(const_cast<Context&>(*this), mode));
}

std::unique_ptr<lvm::StackMemoryAccessor> lvm::Context::create_stack_accessor() const {
    
    // Accessor creation only allowed in protected mode
    if (!vmem_unit_.is_protected()) {
        throw std::runtime_error("Cannot create stack accessor in UNPROTECTED mode");
    }
    
    return std::unique_ptr<StackMemoryAccessor>(
        new StackMemoryAccessor(*this));
}
