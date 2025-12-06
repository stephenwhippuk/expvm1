#pragma once
#include "memsize.h"
#include <memory>
#include "accessMode.h"
#include <vector>
namespace lvm{
    class Memory;  // Forward declaration

    class MemoryAccessor{
    public:
        // Destructor - releases the reserved space in Memory
        ~MemoryAccessor();

        // Public interface - offset-based access within the reserved space
        // respect the size of the reserved space
        // respect the protected mode of the Memory 

        byte_t read_byte(addr_t offset) const;

        void bulk_read(addr_t offset, std::vector<byte_t>& buffer) const;

        // only if mode is READ_WRITE
        void write_byte(addr_t offset, byte_t value);
        
        void bulk_write(addr_t offset, const std::vector<byte_t>& data);

        memsize_t get_size() const { return size; }
        
    private:
        friend class Memory;  // Only Memory can construct
        MemoryAccessor(Memory& memory, byte_t* data_ptr, page_t page, addr_t start, memsize_t size, MemAccessMode mode);
        
        Memory& memory_ref;    // Reference to the Memory object
        byte_t* data_ptr;      // Direct pointer to the reserved space in the vector
        page_t page;           // Page number of the reservation
        addr_t start;          // Start address of the reservation
        memsize_t size;        // Size of the reservation
        MemAccessMode mode;    // Access mode (READ or WRITE)
    }; 
}