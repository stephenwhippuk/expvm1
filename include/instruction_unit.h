#pragma once
#include "register.h"
#include "accessMode.h"
#include <memory>
#include "memsize.h"
#include "mem_access.h"
#include "stack.h"
namespace lvm{

    class InstructionUnit; // Forward declaration

    class InstructionUnit_Accessor{
    public:
        ~InstructionUnit_Accessor();
        // Read Access Methods    
        word_t get_IR() const ;
        word_t readByte_At_IR() const ;
        word_t readWWord_At_IR() const ;

        // Read/Write Access Methods
        void advance_IR(word_t offset);
        void set_IR(word_t value);
        void Jump_To_Address(addr_t address);
        void Jump_To_Address_Conditional(addr_t address, Flag flag, bool condition);
        void Load_Program(const std::vector<byte_t>& program);

        // subroutines
        void call_subroutine(addr_t address, bool with_return_value = false);
        void return_from_subroutine();

    private:
        friend class InstructionUnit;
        InstructionUnit_Accessor(InstructionUnit* instruction_unit, MemAccessMode access_mode);
        InstructionUnit* instruction_unit_ref;
        MemAccessMode mode;

    };

    class InstructionUnit{
    public:
        InstructionUnit(std::shared_ptr<Memory> memory, std::unique_ptr<Stack_Accessor> stack_ptr, std::shared_ptr<Flags> flags_ptr, addr_t start_address = 0, memsize_t size = 32765);
        ~InstructionUnit();
        std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode);
    private:
    friend class InstructionUnit_Accessor;    
        std::unique_ptr<Register> ir_register;
        std::shared_ptr<Flags> flags;
        std::unique_ptr<MemoryAccessor> memory_accessor;
        std::unique_ptr<Stack_Accessor> stack_accessor;

        struct ReturnStackItem {
            addr_t return_address;
            addr_t frame_pointer;
        };
        std::vector<ReturnStackItem> return_stack;
        
        void set_IR(word_t value);
        void advance_IR(word_t offset);
        void jump_to_address(addr_t address);
        void jump_to_address_conditional(addr_t address, Flag flag, bool condition);
        void load_program(const std::vector<byte_t>& program);
        void call_subroutine(addr_t address, bool with_return_value = false);
        void return_from_subroutine();
    };
}