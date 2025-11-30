#include "mem_access.h"
#include "memunit.h"
#include "errors.h"
using namespace lvm;

MemoryAccessor::MemoryAccessor(Memory& memory, byte_t* data_ptr, page_t page, addr_t start, memsize_t size, MemAccessMode mode)
    : memory_ref(memory), data_ptr(data_ptr), page(page), start(start), size(size), mode(mode) {}

MemoryAccessor::~MemoryAccessor() {
    // Release the reserved space in the Memory object
    memory_ref.release_space(page, start, size);
}

byte_t MemoryAccessor::read_byte(addr_t offset) const {
    if(!memory_ref.is_protected_mode()) {
        throw lvm::runtime_error("Cannot read from MemoryAccessor while Memory is in unprotected mode");
    }
    if (offset >= size) {
        throw lvm::runtime_error("Read offset out of bounds");
    }
    return data_ptr[offset];
}

void MemoryAccessor::write_byte(addr_t offset, byte_t value) {
    if(!memory_ref.is_protected_mode()) {
        throw lvm::runtime_error("Cannot write to MemoryAccessor while Memory is in unprotected mode");
    }
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to write to READ_ONLY memory");
    }
    if (offset >= size) {
        throw lvm::runtime_error("Write offset out of bounds");
    }
    data_ptr[offset] = value;
}

void MemoryAccessor::bulk_write(addr_t offset, const std::vector<byte_t>& data) {
    if(!memory_ref.is_protected_mode()) {
        throw lvm::runtime_error("Cannot write to MemoryAccessor while Memory is in unprotected mode");
    }
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to bulk write to READ_ONLY memory");
    }
    if (offset + data.size() > size) {
        throw lvm::runtime_error("Bulk write exceeds reserved memory bounds");
    }
    memory_ref.bulk_write(start + offset, data);
}

void MemoryAccessor::bulk_read(addr_t offset, std::vector<byte_t>& buffer) const {
    if(!memory_ref.is_protected_mode()) {
        throw lvm::runtime_error("Cannot read from MemoryAccessor while Memory is in unprotected mode");
    }
    if (offset + buffer.size() > size) {
        throw lvm::runtime_error("Bulk read exceeds reserved memory bounds");
    }
    memory_ref.bulk_read(start + offset, buffer, static_cast<memsize_t>(buffer.size()));
}