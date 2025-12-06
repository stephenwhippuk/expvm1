#pragma once

#include "memsize.h"
#include "accessMode.h"
#include <memory>

namespace lvm {

/**
 * Memory2 - Next generation memory unit
 * 
 * This is a redesign of the memory management system to support
 * more flexible configurations and better isolation between components.
 */
class Memory2 {
public:
    /**
     * Constructor
     * @param total_size Total memory size in bytes
     */
    explicit Memory2(memsize_t total_size);
    
    /**
     * Destructor
     */
    ~Memory2();
    
    // Prevent copying
    Memory2(const Memory2&) = delete;
    Memory2& operator=(const Memory2&) = delete;
    
    // Allow moving
    Memory2(Memory2&&) noexcept = default;
    Memory2& operator=(Memory2&&) noexcept = default;
    
    /**
     * Get total memory size
     */
    memsize_t size() const;
    
    /**
     * Read a byte from memory
     * @param address Memory address to read from
     * @return The byte value at the address
     */
    byte_t read_byte(addr_t address) const;
    
    /**
     * Write a byte to memory
     * @param address Memory address to write to
     * @param value Byte value to write
     */
    void write_byte(addr_t address, byte_t value);
    
    /**
     * Read a word (16-bit) from memory
     * @param address Memory address to read from (reads 2 bytes)
     * @return The word value at the address
     */
    word_t read_word(addr_t address) const;
    
    /**
     * Write a word (16-bit) to memory
     * @param address Memory address to write to (writes 2 bytes)
     * @param value Word value to write
     */
    void write_word(addr_t address, word_t value);
    
    /**
     * Bulk read from memory
     * @param address Starting address
     * @param buffer Buffer to read into
     * @param size Number of bytes to read
     */
    void read_bulk(addr_t address, byte_t* buffer, memsize_t size) const;
    
    /**
     * Bulk write to memory
     * @param address Starting address
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     */
    void write_bulk(addr_t address, const byte_t* buffer, memsize_t size);
    
    /**
     * Clear a region of memory (set to zero)
     * @param address Starting address
     * @param size Number of bytes to clear
     */
    void clear(addr_t address, memsize_t size);
    
    /**
     * Clear all memory
     */
    void clear_all();

private:
    memsize_t total_size_;
    std::unique_ptr<byte_t[]> memory_;
    
    // Helper to validate address
    void validate_address(addr_t address, memsize_t size = 1) const;
};

} // namespace lvm
