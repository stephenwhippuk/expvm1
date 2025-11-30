#include "memunit.h"
#include <cstring> // for std::memmove
#include "errors.h"
using namespace lvm;

// Helper functions local to implementation file

void AssertNotProtectedMode(bool protected_mode_enabled, const std::string& action) {
    if (protected_mode_enabled) {
        throw lvm::runtime_error("Cannot " + action + " in protected mode");
    }
}

void AssertProtectedMode(bool protected_mode_enabled, const std::string& action) {
    if (!protected_mode_enabled) {
        throw lvm::runtime_error("Cannot " + action + " in unprotected mode");
    }
}


// MEMORY CLASS IMPLEMENTATION

Memory::Memory(memsize_t page_size, page_t pages)
    : page_size(page_size), total_pages(pages)
{
    memory_data.resize(page_size * pages, 0); // Initialize memory with zeros
    current_page = 0;
    protected_mode_enabled = false; // Start in unprotected mode
}

Memory::~Memory() {
    // Note: Caller should ensure Memory is not destroyed with active reservations in protected mode
    // Destructor cannot throw, so we just clean up silently
}

void Memory::check_can_destroy() const {
    if (protected_mode_enabled && !reserved_blocks.empty()) {
        throw lvm::runtime_error("Cannot destroy Memory while in protected mode with active reservations");
    }
}

void Memory::protected_mode() {
    protected_mode_enabled = true;
}

void Memory::unprotected_mode() {
    protected_mode_enabled = false;
}

bool Memory::is_protected_mode() const {
    return protected_mode_enabled;
}

// PRIMARY OPERATIONS (UNPROTECTED MODE ONLY)


void Memory::write(addr_t address, byte_t value) {
    AssertProtectedMode(protected_mode_enabled, "write");
    // test cannot write ouside of current page
    if (address < current_page * page_size || address >= (current_page + 1) * page_size) {
        throw lvm::runtime_error("Write address outside current page");
    }
    // test cannot write to reserved space
    for (const auto& block : reserved_blocks) {
        if (block.page == current_page &&
            address >= block.start && address < block.start + block.size) {
            throw lvm::runtime_error("Write address inside reserved memory space");
        }
    }
    memory_data[address] = value;
}

byte_t Memory::read(addr_t address) const {
    AssertProtectedMode(protected_mode_enabled, "read");
    // test cannot read ouside of current page
    if (address < current_page * page_size || address >= (current_page + 1) * page_size) {
        throw lvm::runtime_error("Read address outside current page");
    }
    // test cannot read from reserved space
    for (const auto& block : reserved_blocks) {
        if (block.page == current_page &&
            address >= block.start && address < block.start + block.size) {
            throw lvm::runtime_error("Read address inside reserved memory space");
        }
    }
    return memory_data[address];
}

void Memory::switch_page(page_t page_number) {
    AssertProtectedMode(protected_mode_enabled, "switch pages");
    
    if (page_number >= total_pages) {
        throw lvm::runtime_error("Invalid page number");
    }
    current_page = page_number;
}

void Memory::copy(addr_t dest_address, addr_t src_address, memsize_t size) {
    AssertProtectedMode(protected_mode_enabled, "copy memory");
    // test cannot copy outside of current page
    if (dest_address < current_page * page_size || dest_address + size > (current_page + 1) * page_size ||
        src_address < current_page * page_size || src_address + size > (current_page + 1) * page_size) {
        throw lvm::runtime_error("Copy addresses outside current page");
    }
    // test cannot copy to or from reserved space
    for (const auto& block : reserved_blocks) {
        if (block.page == current_page &&
            ((dest_address >= block.start && dest_address < block.start + block.size) ||
             (src_address >= block.start && src_address < block.start + block.size))) {
            throw lvm::runtime_error("Copy address inside reserved memory space");
        }
    }
    std::memmove(&memory_data[dest_address], &memory_data[src_address], size);
}

void Memory::fill(addr_t address, byte_t value, memsize_t size) {
    AssertProtectedMode(protected_mode_enabled, "fill memory");
    // test cannot fill outside of current page
    if (address < current_page * page_size || address + size > (current_page + 1) * page_size) {
        throw lvm::runtime_error("Fill address outside current page");
    }
    // test cannot fill inside reserved space
    for (const auto& block : reserved_blocks) {
        if (block.page == current_page &&
            !(address + size <= block.start || address >= block.start + block.size)) {
            throw lvm::runtime_error("Fill address inside reserved memory space");
        }
    }
    std::memset(&memory_data[address], value, size);
}

void Memory::bulk_write(addr_t address, const std::vector<byte_t>& data) {
    AssertProtectedMode(protected_mode_enabled, "bulk write memory");
    // test cannot bulk write outside of current page
    if (address < current_page * page_size || address + data.size() > (current_page + 1) * page_size) {
        throw lvm::runtime_error("Bulk write address outside current page");
    }
    if (address + data.size() > memory_data.size()) {
        throw lvm::runtime_error("Bulk write exceeds memory bounds");
    }
    std::memcpy(&memory_data[address], data.data(), data.size());
}

void Memory::bulk_read(addr_t address, std::vector<byte_t>& buffer, memsize_t size) const {
    AssertProtectedMode(protected_mode_enabled, "bulk read memory");
    // test cannot bulk read outside of current page
    if (address < current_page * page_size || address + size > (current_page + 1) * page_size) {
        throw lvm::runtime_error("Bulk read address outside current page");
    }
    if (address + size > memory_data.size()) {
        throw lvm::runtime_error("Bulk read exceeds memory bounds");
    }
    buffer.resize(size);
    std::memcpy(buffer.data(), &memory_data[address], size);
}

// RESERVEED SPACE MANAGEMENT

byte_t* Memory::get_data_ptr(addr_t address) {
    return &memory_data[address];
}

void Memory::release_space(page_t page, addr_t start, memsize_t size) {
    // Find and remove the reserved block
    for (auto it = reserved_blocks.begin(); it != reserved_blocks.end(); ++it) {
        if (it->page == page && it->start == start && it->size == size) {
            reserved_blocks.erase(it);
            return;
        }
    }
    throw lvm::runtime_error("Attempt to release unreserved memory space");
}


std::unique_ptr<MemoryAccessor> Memory::reserve_space(page_t page, addr_t start, memsize_t size, MemAccessMode mode) {
    AssertNotProtectedMode(protected_mode_enabled, "reserve space");

    // Check for overlapping reservations
    for (const auto& block : reserved_blocks) {
        if (block.page == page &&
            !(start + size <= block.start || start >= block.start + block.size)) {
            throw lvm::runtime_error("Memory reservation overlaps with existing reservation");
        }
    }

    // Create a new reserved block
    reserved_block new_block{page, start, size, mode};
    reserved_blocks.push_back(new_block);

    // Get pointer to the start of the reserved space
    byte_t* data_ptr = get_data_ptr(page * page_size + start);

    return std::unique_ptr<MemoryAccessor>(new MemoryAccessor(*this, data_ptr, page, start, size, mode));
}



