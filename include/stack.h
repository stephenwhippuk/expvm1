/* the stack is ,ike the machine stack sharing system memory though the 
Memory Accessor interface. It works downwards from a high address to a low address.

 */

#pragma once    
#include "memsize.h"
#include "mem_access.h"
#include <memory>
#include "register.h"
namespace lvm{

    class Stack; // Forward declaration

    class Stack_Accessor{
    public:
        ~Stack_Accessor();
        // READ Access Methods
        byte_t peek_byte() const;
        word_t peek_word() const;
        byte_t peek_byte_from_base(page_offset_t offset) const;
        word_t peek_word_from_base(page_offset_t offset) const;
        byte_t peek_byte_from_frame(page_offset_t offset) const;
        word_t peek_word_from_frame(page_offset_t offset) const;
        bool is_empty() const;
        bool is_full() const;
        memsize_t get_size() const;
        addr_t get_frame_register() const;

        // READ/WRITE Access Methods
        void push(byte_t value);
        byte_t pop();
        void push_word(word_t value);
        word_t pop_word();
        void set_frame_register(word_t value);
        void set_frame_to_top();
        void flush();
    private:
        friend class Stack;
        Stack_Accessor(Stack* stack, MemAccessMode access_mode);
        Stack* stack_ref;
        MemAccessMode mode;
    };
    class Stack{
        public:
            // the stack uses a MemoryAccessor to manage its memory
            // therefore can only be created in uprotected mode
            Stack(std::shared_ptr<Memory> memory, page_t page, addr_t base_address, memsize_t stack_size);
            ~Stack();
           
            std::unique_ptr<Stack_Accessor> get_accessor(MemAccessMode mode);
        private:
            friend class Stack_Accessor;
            // unique ptr to memory accessor so memory is released when stack is destroyed
            std::unique_ptr<MemoryAccessor> mem_accessor;
            memsize_t stack_size;
            addr_t sp; // Stack pointer     
            std::shared_ptr<Register> FP; // frame pointer register
            std::shared_ptr<Register> BP; // base pointer register
            addr_t bottom_address; // base address of the stack in memory
             void push(byte_t value);
            byte_t pop();
            byte_t peek() const;
            void push_word(word_t value);
            word_t pop_word();
            word_t peek_word() const;

            byte_t peek_byte_from_base(addr_t offset) const;
            word_t peek_word_from_base(addr_t offset) const;

            byte_t peek_byte_from_frame(addr_t offset) const;
            word_t peek_word_from_frame(addr_t offset) const;

            bool is_empty() const  ;
            bool is_full() const;
            memsize_t get_size() const;
            void flush();

            void set_frame_to_top();
            void set_frame_register(word_t value);
            addr_t get_frame_register() const;
    };
}