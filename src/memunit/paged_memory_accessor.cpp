#include "paged_memory_accessor.h"
#include "vmemunit.h"
#include "errors.h"
#include "helpers.h"
#include <stdexcept>

using namespace lvm;

PagedMemoryAccessor::PagedMemoryAccessor(VMemUnit& vmem_unit, ::Context& context,
                                     context_id_t context_id, uint32_t context_size, MemAccessMode mode)
    : vmem_unit_(vmem_unit),
      context_(context),
      context_id_(context_id),
      context_size_(context_size),
      mode_(mode) {
}

void PagedMemoryAccessor::set_page(page_t page) {
    context_.set_current_page(page);
}

page_t PagedMemoryAccessor::get_page() const {
    return context_.get_current_page();
}

uint32_t PagedMemoryAccessor::page_offset_to_address(page_t page, addr_t offset) const {
    // Combine 16-bit page and 16-bit offset into 32-bit address
    // Page is high 16 bits, offset is low 16 bits
    uint32_t address = (static_cast<uint32_t>(page) << 16) | static_cast<uint32_t>(offset);
    
    // Validate address is within context bounds
    if (address >= context_size_) {
        throw std::runtime_error("Address exceeds context size");
    }
    
    return address;
}

byte_t PagedMemoryAccessor::read_byte(addr_t offset) const {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    uint32_t address = page_offset_to_address(context_.get_current_page(), offset);
    return vmem_unit_.read_byte(context_id_, address);
}

void PagedMemoryAccessor::write_byte(addr_t offset, byte_t value) {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (mode_ != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to write to READ_ONLY memory");
    }
    
    uint32_t address = page_offset_to_address(context_.get_current_page(), offset);
    vmem_unit_.write_byte(context_id_, address, value);
}

word_t PagedMemoryAccessor::read_word(addr_t offset) const {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    // Read two consecutive bytes (little-endian)
    byte_t low = read_byte(offset);
    byte_t high = read_byte(offset + 1);
    return combine_bytes_to_word(high, low);
}

void PagedMemoryAccessor::write_word(addr_t offset, word_t value) {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (mode_ != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to write to READ_ONLY memory");
    }
    
    // Write two consecutive bytes (little-endian)
    byte_t low = static_cast<byte_t>(value & 0xFF);
    byte_t high = static_cast<byte_t>((value >> 8) & 0xFF);
    write_byte(offset, low);
    write_byte(offset + 1, high);
}

void PagedMemoryAccessor::bulk_read(addr_t offset, std::vector<byte_t>& buffer, memsize_t size) const {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot read from PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    buffer.resize(size);
    for (memsize_t i = 0; i < size; ++i) {
        buffer[i] = read_byte(offset + i);
    }
}

void PagedMemoryAccessor::bulk_write(addr_t offset, const std::vector<byte_t>& data) {
    if (!vmem_unit_.is_protected()) {
        throw lvm::runtime_error("Cannot write to PagedMemoryAccessor while VMemUnit is in unprotected mode");
    }
    
    if (mode_ != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to bulk write to READ_ONLY memory");
    }
    
    for (size_t i = 0; i < data.size(); ++i) {
        write_byte(offset + static_cast<addr_t>(i), data[i]);
    }
}
