/* the stack is ,ike the machine stack sharing system memory though the 
Memory Accessor interface. It works downwards from a high address to a low address.

 */

#pragma once    
#include "memsize.h"
#include "mem_access.h"

namespace lvm{
    class Stack{
        public:
            // the stack uses a MemoryAccessor to manage its memory
            // therefore can only be created in uprotected mode
            Stack(Memory& memory, page_t page, addr_t base_address, memsize_t stack_size);
            ~Stack();
            void push(byte_t value);
            byte_t pop();
            byte_t peek() const;
            void push_word(word_t value);
            word_t pop_word();
            word_t peek_word() const;

            byte_t peek_at_offset(addr_t offset) const;
            word_t peek_word_at_offset(addr_t offset) const;
            
            bool is_empty() const;
            bool is_full() const;
            memsize_t get_size() const;
        private:
            // unique ptr to memory accessor so memory is released when stack is destroyed
            std::unique_ptr<MemoryAccessor> mem_accessor;
            memsize_t stack_size;
            addr_t sp; // Stack pointer     
    };
}