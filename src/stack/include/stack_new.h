/* Stack: A managed stack that operates in its own address space via StackAccessor
 * - Grows upward from address 0
 * - Frame pointer (FP) acts as a movable bottom, preserving everything below it
 * - FP sits at -1 relative to the frame (first position is FP+1)
 * - Stack pointer (SP) points to the next free position
 * - Fixed capacity allocated at creation
 */

#pragma once
#include "memsize.h"
#include "accessMode.h"
#include "vaddr.h"
#include "vmemunit.h"
#include "stack_accessor.h"
#include <memory>

namespace lvm {

    class Stack2; // Forward declaration

    class StackAccessor2 {
    public:
        ~StackAccessor2() = default;
        
        // READ Access Methods
        byte_t peek_byte() const;           // Peek at top of stack
        word_t peek_word() const;           // Peek word at top of stack
        byte_t peek_byte_from_base(addr32_t offset) const;   // Read from base + offset
        word_t peek_word_from_base(addr32_t offset) const;   // Read word from base + offset
        byte_t peek_byte_from_frame(addr32_t offset) const;  // Read from frame + offset
        word_t peek_word_from_frame(addr32_t offset) const;  // Read word from frame + offset
        
        // Stack state queries
        bool is_empty() const;
        bool is_full() const;
        addr32_t get_size() const;          // Current number of items on stack
        addr32_t get_capacity() const;      // Maximum capacity
        addr32_t get_sp() const;            // Get stack pointer
        int32_t get_fp() const;             // Get frame pointer

        // READ/WRITE Access Methods
        void push_byte(byte_t value);
        byte_t pop_byte();
        void push_word(word_t value);
        word_t pop_word();
        
        // Frame management
        void set_frame_pointer(int32_t value);
        void set_frame_to_top();            // Set FP to current SP position
        
        // Stack management
        void flush();                       // Reset stack to empty state

    private:
        friend class Stack2;
        StackAccessor2(Stack2* stack, MemAccessMode access_mode);
        Stack2* stack_ref;
        MemAccessMode mode;
    };

    class Stack2 {
    public:
        // Creates a stack with its own context in the virtual memory unit
        // - Only allowed in UNPROTECTED mode
        // - Allocates a dedicated context with specified capacity
        Stack2(VMemUnit& vmem_unit, addr32_t capacity);
        ~Stack2() = default;
        
        // Delete copy operations
        Stack2(const Stack2&) = delete;
        Stack2& operator=(const Stack2&) = delete;
        
        // Allow move operations
        Stack2(Stack2&&) = default;
        Stack2& operator=(Stack2&&) = default;
        
        // Get an accessor to perform stack operations
        // - Only allowed in PROTECTED mode
        std::unique_ptr<StackAccessor2> get_accessor(MemAccessMode mode);

    private:
        friend class StackAccessor2;
        
        VMemUnit& vmem_unit_;
        context_id_t context_id_;
        addr32_t capacity_;     // Maximum capacity in bytes
        addr32_t sp_;           // Stack pointer (points to next free position)
        int32_t fp_;            // Frame pointer (movable bottom, sits at -1 relative to frame)
        
        // Internal operations (called by Stack_Accessor)
        void push_byte(byte_t value);
        byte_t pop_byte();
        byte_t peek_byte() const;
        void push_word(word_t value);
        word_t pop_word();
        word_t peek_word() const;
        
        byte_t peek_byte_from_base(addr32_t offset) const;
        word_t peek_word_from_base(addr32_t offset) const;
        byte_t peek_byte_from_frame(addr32_t offset) const;
        word_t peek_word_from_frame(addr32_t offset) const;
        
        bool is_empty() const;
        bool is_full() const;
        addr32_t get_size() const { return sp_; }
        addr32_t get_capacity() const { return capacity_; }
        addr32_t get_sp() const { return sp_; }
        int32_t get_fp() const { return fp_; }
        
        void set_frame_pointer(int32_t value);
        void set_frame_to_top();
        void flush();
    };

} // namespace lvm
