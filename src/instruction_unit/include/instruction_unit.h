#pragma once
#include "register.h"
#include "accessMode.h"
#include <memory>
#include "memsize.h"
#include "paged_memory_accessor.h"
#include "vmemunit.h"
#include "istack.h"
#include "basic_io.h"
#include "iinstruction_unit.h"
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
        void system_call(word_t syscall_number);

    private:
        friend class InstructionUnit;
        InstructionUnit_Accessor(InstructionUnit* instruction_unit, MemAccessMode access_mode);
        InstructionUnit* instruction_unit_ref;
        MemAccessMode mode;

    };

    class InstructionUnit : public IInstructionUnit {
    public:
        InstructionUnit(std::shared_ptr<IVMemUnit> vmem_unit, context_id_t code_context_id, IStack& stack, std::shared_ptr<Flags> flags_ptr, std::shared_ptr<BasicIO> basic_io);
        ~InstructionUnit();
        
        // IInstructionUnit interface implementation
        std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode) override;
    private:
    friend class InstructionUnit_Accessor;    
        std::shared_ptr<IVMemUnit> vmem_unit_;
        context_id_t code_context_id_;
        std::unique_ptr<Register> ir_register;
        std::shared_ptr<Flags> flags;
        IStack& stack_;

        struct ReturnStackItem {
            addr_t return_address;
            int32_t frame_pointer;  // Signed to support -1 initial value
        };
        std::vector<ReturnStackItem> return_stack;
        std::shared_ptr<BasicIO> basic_io_;
        
        void set_IR(word_t value);
        void advance_IR(word_t offset);
        void jump_to_address(addr_t address);
        void jump_to_address_conditional(addr_t address, Flag flag, bool condition);
        void load_program(const std::vector<byte_t>& program);
        void call_subroutine(addr_t address, bool with_return_value = false);
        void return_from_subroutine();
        void system_call(word_t syscall_number);
    };
}