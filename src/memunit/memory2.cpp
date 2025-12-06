#include "memory2.h"
#include "errors.h"
#include "helpers.h"
#include <cstring>

namespace lvm {

Memory2::Memory2(memsize_t total_size)
    : total_size_(total_size)
    , memory_(std::make_unique<byte_t[]>(total_size))
{
    if (total_size == 0) {
        throw lvm::runtime_error("Memory size cannot be zero");
    }
    
    // Initialize memory to zero
    clear_all();
}

Memory2::~Memory2() = default;

memsize_t Memory2::size() const {
    return total_size_;
}

void Memory2::validate_address(addr_t address, memsize_t size) const {
    if (address + size > total_size_) {
        throw lvm::runtime_error("Memory address out of bounds");
    }
}

byte_t Memory2::read_byte(addr_t address) const {
    validate_address(address, 1);
    return memory_[address];
}

void Memory2::write_byte(addr_t address, byte_t value) {
    validate_address(address, 1);
    memory_[address] = value;
}

word_t Memory2::read_word(addr_t address) const {
    validate_address(address, 2);
    byte_t low = memory_[address];
    byte_t high = memory_[address + 1];
    return combine_bytes_to_word(high, low);
}

void Memory2::write_word(addr_t address, word_t value) {
    validate_address(address, 2);
    memory_[address] = static_cast<byte_t>(value & 0xFF);        // Low byte
    memory_[address + 1] = static_cast<byte_t>((value >> 8) & 0xFF); // High byte
}

void Memory2::read_bulk(addr_t address, byte_t* buffer, memsize_t size) const {
    if (size == 0) return;
    
    validate_address(address, size);
    std::memcpy(buffer, &memory_[address], size);
}

void Memory2::write_bulk(addr_t address, const byte_t* buffer, memsize_t size) {
    if (size == 0) return;
    
    validate_address(address, size);
    std::memcpy(&memory_[address], buffer, size);
}

void Memory2::clear(addr_t address, memsize_t size) {
    if (size == 0) return;
    
    validate_address(address, size);
    std::memset(&memory_[address], 0, size);
}

void Memory2::clear_all() {
    std::memset(memory_.get(), 0, total_size_);
}

} // namespace lvm
