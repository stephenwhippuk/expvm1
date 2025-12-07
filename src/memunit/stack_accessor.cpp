#include "stack_accessor.h"
#include "vmemunit.h"
#include "errors.h"
#include "helpers.h"
#include <stdexcept>

using namespace lvm;

StackMemoryAccessor::StackMemoryAccessor(const Context& context)
    : context_(context),
      context_id_(context.get_id()),
      size_(context.get_size()) {
    
    // Pre-allocate all physical memory for the stack
    // This ensures stack has guaranteed memory availability
    // Access VMemUnit through Context
    VMemUnit& vmem = static_cast<VMemUnit&>(context_.vmem_unit_);
    for (addr32_t addr = 0; addr < size_; ++addr) {
        // Touch each block to ensure allocation
        if (addr % VMemUnit::BLOCK_SIZE == 0) {
            vmem.ensure_physical_memory(context_id_, addr);
        }
    }
}

byte_t StackMemoryAccessor::read_byte(addr32_t address) const {
    if (!context_.vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from StackMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    VMemUnit& vmem = static_cast<VMemUnit&>(context_.vmem_unit_);
    return vmem.read_byte(context_id_, address);
}

void StackMemoryAccessor::write_byte(addr32_t address, byte_t value) {
    if (!context_.vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to StackMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    VMemUnit& vmem = static_cast<VMemUnit&>(context_.vmem_unit_);
    vmem.write_byte(context_id_, address, value);
}

word_t StackMemoryAccessor::read_word(addr32_t address) const {
    if (!context_.vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from StackMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address + 1 >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    // Read two consecutive bytes (little-endian)
    byte_t low = read_byte(address);
    byte_t high = read_byte(address + 1);
    return combine_bytes_to_word(high, low);
}

void StackMemoryAccessor::write_word(addr32_t address, word_t value) {
    if (!context_.vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to StackMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address + 1 >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    // Write two consecutive bytes (little-endian)
    byte_t low = static_cast<byte_t>(value & 0xFF);
    byte_t high = static_cast<byte_t>((value >> 8) & 0xFF);
    write_byte(address, low);
    write_byte(address + 1, high);
}
