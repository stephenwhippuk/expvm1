#include "vmemunit.h"
#include "paged_memory_accessor.h"
#include "errors.h"
#include "memsize.h"
#include "accessMode.h"
#include <stdexcept>

using namespace lvm;

lvm::VMemUnit::VMemUnit()
    : mode_(Mode::UNPROTECTED),
      next_context_id_(0),
      next_free_address_(0) {
}

void lvm::VMemUnit::set_mode(Mode mode) {
    mode_ = mode;
}

context_id_t lvm::VMemUnit::create_context(uint32_t size) {
    // Context creation only allowed in unprotected mode
    if (is_protected()) {
        throw std::runtime_error("Cannot create context in PROTECTED mode");
    }
    
    // Validate size
    if (size == 0) {
        throw std::invalid_argument("Context size must be greater than 0");
    }
    
    if (size > MAX_CONTEXT_SIZE) {
        throw std::invalid_argument("Context size exceeds maximum (4GB)");
    }
    
    // Allocate virtual space for the context
    vaddr_t base_address = allocate_virtual_space(size);
    
    // Create the context (using new + unique_ptr since constructor is private)
    context_id_t id = next_context_id_++;
    contexts_[id] = std::unique_ptr<Context>(new Context(id, base_address, size));
    
    return id;
}

void lvm::VMemUnit::destroy_context(context_id_t id) {
    // Context destruction only allowed in unprotected mode
    if (is_protected()) {
        throw std::runtime_error("Cannot destroy context in PROTECTED mode");
    }
    
    auto it = contexts_.find(id);
    if (it == contexts_.end()) {
        throw std::invalid_argument("Context ID does not exist");
    }
    
    contexts_.erase(it);
    // Note: In a complete implementation, we would free the virtual space
    // and any associated physical memory here
}

const Context* lvm::VMemUnit::get_context(context_id_t id) const {
    auto it = contexts_.find(id);
    if (it == contexts_.end()) {
        return nullptr;
    }
    return it->second.get();
}

const Context* lvm::VMemUnit::find_context_for_address(vaddr_t addr) const {
    for (const auto& [id, context] : contexts_) {
        if (context->contains(addr)) {
            return context.get();
        }
    }
    return nullptr;
}

vaddr_t lvm::VMemUnit::allocate_virtual_space(uint32_t size) {
    vaddr_t base = next_free_address_;
    vaddr_t end = base + static_cast<vaddr_t>(size);
    
    // Check for overflow or exceeding 40-bit space
    if (!is_valid_vaddr(end) || end < base) {
        throw std::runtime_error("Virtual address space exhausted");
    }
    
    next_free_address_ = end;
    return base;
}

void lvm::VMemUnit::ensure_physical_memory(context_id_t context_id, uint32_t address) {
    uint32_t block_index = get_block_index(address);
    
    // Check if block already exists
    auto& context_blocks = physical_memory_[context_id];
    if (context_blocks.find(block_index) == context_blocks.end()) {
        // Allocate new block (initialized to zero)
        context_blocks[block_index] = std::vector<byte_t>(BLOCK_SIZE, 0);
    }
}

byte_t lvm::VMemUnit::read_byte(context_id_t context_id, uint32_t address) const {
    // Verify context exists
    auto ctx_it = contexts_.find(context_id);
    if (ctx_it == contexts_.end()) {
        throw std::invalid_argument("Context ID does not exist");
    }
    
    // Validate address is within context bounds
    if (address >= ctx_it->second->get_size()) {
        throw std::runtime_error("Address exceeds context size");
    }
    
    // Get physical memory block
    auto mem_it = physical_memory_.find(context_id);
    if (mem_it == physical_memory_.end()) {
        // No memory allocated yet, return 0
        return 0;
    }
    
    uint32_t block_index = get_block_index(address);
    uint32_t block_offset = get_block_offset(address);
    
    auto block_it = mem_it->second.find(block_index);
    if (block_it == mem_it->second.end()) {
        // Block not allocated yet, return 0
        return 0;
    }
    
    return block_it->second[block_offset];
}

void lvm::VMemUnit::write_byte(context_id_t context_id, uint32_t address, byte_t value) {
    // Verify context exists
    auto ctx_it = contexts_.find(context_id);
    if (ctx_it == contexts_.end()) {
        throw std::invalid_argument("Context ID does not exist");
    }
    
    // Validate address is within context bounds
    if (address >= ctx_it->second->get_size()) {
        throw std::runtime_error("Address exceeds context size");
    }
    
    // Ensure physical memory is allocated
    const_cast<lvm::VMemUnit*>(this)->ensure_physical_memory(context_id, address);
    
    uint32_t block_index = get_block_index(address);
    uint32_t block_offset = get_block_offset(address);
    
    physical_memory_[context_id][block_index][block_offset] = value;
}
