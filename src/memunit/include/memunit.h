#pragma once
#include "memsize.h"
#include <vector>
#include <memory>
#include "mem_access.h"
#include "accessMode.h"

namespace lvm { 

    class Memory{
    public:
        Memory(memsize_t page_size, page_t pages);
        // if in protected mode and there are reservations then ERROR
        ~Memory(); 

        void check_can_destroy() const;
        
        // only in protected mode
        // only to unreserved space 
        void write(addr_t address, byte_t value);
        byte_t read(addr_t address) const;
        void copy(addr_t dest_address, addr_t src_address, memsize_t size);
        void fill(addr_t address, byte_t value, memsize_t size);
        void bulk_write(addr_t address, const std::vector<byte_t>& data);
        void bulk_read(addr_t address, std::vector<byte_t>& buffer, memsize_t size) const;         
        void switch_page(page_t page_number);

        void protected_mode(); 
        void unprotected_mode();
        bool is_protected_mode() const;
        // only in unprotected mode
        std::unique_ptr<MemoryAccessor> reserve_space(page_t page, addr_t start, memsize_t size, MemAccessMode mode);

    private:
        friend class MemoryAccessor;  // Allow accessor privileged access
        
        // Privileged methods for MemoryAccessor only
        byte_t* get_data_ptr(addr_t address);
        void release_space(page_t page, addr_t start, memsize_t size);


        struct reserved_block{
            page_t page;
            addr_t start;
            memsize_t size;
            MemAccessMode mode;
        };
        std::vector<reserved_block> reserved_blocks;

        std::vector<byte_t> memory_data;
        memsize_t page_size;
        page_t total_pages;
        page_t current_page;


        // in unprotected mode no writes or reads allowed
        // accessor should respect this
        bool protected_mode_enabled;
    };
}