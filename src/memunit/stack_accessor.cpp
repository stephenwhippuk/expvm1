#include "stack_accessor.h"
#include "vmemunit.h"
#include "errors.h"
#include "helpers.h"
#include <stdexcept>

using namespace lvm;

StackAccessor::StackAccessor(VMemUnit& vmem_unit, context_id_t context_id, addr32_t size)
    : vmem_unit_(vmem_unit),
      context_id_(context_id),
      size_(size) {
    
    // Pre-allocate all physical memory for the stack
    // This ensures stack has guaranteed memory availability
    for (addr32_t addr = 0; addr < size; ++addr) {
        // Touch each block to ensure allocation
        if (addr % vmem_unit.BLOCK_SIZE == 0) {
            vmem_unit_.ensure_physical_memory(context_id_, addr);
        }
    }
}

byte_t StackAccessor::read_byte(addr32_t address) const {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from StackAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    return vmem_unit_.read_byte(context_id_, address);
}

void StackAccessor::write_byte(addr32_t address, byte_t value) {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to StackAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    vmem_unit_.write_byte(context_id_, address, value);
}

word_t StackAccessor::read_word(addr32_t address) const {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from StackAccessor while VMemUnit is in unprotected mode");
    }
    
    if (address + 1 >= size_) {
        throw std::runtime_error("Stack address out of bounds");
    }
    
    // Read two consecutive bytes (little-endian)
    byte_t low = read_byte(address);
    byte_t high = read_byte(address + 1);
    return combine_bytes_to_word(high, low);
}

void StackAccessor::write_word(addr32_t address, word_t value) {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to StackAccessor while VMemUnit is in unprotected mode");
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
