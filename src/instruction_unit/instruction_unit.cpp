#include "instruction_unit.h"
#include "errors.h"
#include "stack.h"
#include "memunit.h"

#include <iostream>
using namespace lvm;

InstructionUnit::InstructionUnit(std::shared_ptr<Memory> memory, std::unique_ptr<Stack_Accessor> stack_ptr, std::shared_ptr<Flags> flags_ptr, addr_t start_address, memsize_t size)
       : stack_accessor(std::move(stack_ptr))
{
    // assert stack_ptr is valid
    if (!stack_accessor) {
        throw lvm::runtime_error("InstructionUnit requires a valid Stack_Accessor");
    }
    // assert memory is in unprotected mode
    if(memory->is_protected_mode()) {
        throw lvm::runtime_error("InstructionUnit must be created in unprotected mode memory");
    }

    memory_accessor = memory->reserve_space(0, start_address, size, MemAccessMode::READ_WRITE);
    ir_register = std::make_unique<Register>(flags_ptr);
    flags = flags_ptr;
}

InstructionUnit::~InstructionUnit() {}

std::unique_ptr<InstructionUnit_Accessor> InstructionUnit::get_accessor(MemAccessMode mode) {
    return std::unique_ptr<InstructionUnit_Accessor>(new InstructionUnit_Accessor(this, mode));
}

void InstructionUnit::set_IR(word_t value) {
    ir_register->set_value(value);
}   
void InstructionUnit::advance_IR(word_t offset) {
    word_t current = ir_register->get_value();
    ir_register->set_value(current + offset);
}

void InstructionUnit::jump_to_address(addr_t address) {
    ir_register->set_value(address);
}

void InstructionUnit::jump_to_address_conditional(addr_t address, Flag flag, bool condition) {
    bool flag_set = flags->is_set(flag);
    if (flag_set == condition) {
        ir_register->set_value(address);
    }
}

void InstructionUnit::load_program(const std::vector<byte_t>& program) {
    if (program.size() > memory_accessor->get_size()) {
        throw lvm::runtime_error("Program size exceeds InstructionUnit memory size");
    }
    memory_accessor->bulk_write(0, program);
}

void InstructionUnit::call_subroutine(addr_t address, bool with_return_value){
    // Save current state
    ReturnStackItem item;
    item.return_address = ir_register->get_value();
    item.frame_pointer = stack_accessor->get_frame_register();

    return_stack.push_back(item);

    // Jump to subroutine address
    ir_register->set_value(address);
    std::cout << "1 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
    if (with_return_value) {
        stack_accessor->push(1);
    }
    else {
        stack_accessor->push(0);
    }
     std::cout << "2 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
    // Now set FP = SP, so the flag is AT FP
    // This protects the flag from user code (which must push before accessing anything)
    stack_accessor->set_frame_to_top();
     std::cout << "fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
}

void InstructionUnit::return_from_subroutine() {
    if (return_stack.empty()) {
        throw lvm::runtime_error("Return stack underflow on return from subroutine");
    }
    // Restore state
    ReturnStackItem item = return_stack.back();
    return_stack.pop_back();

    ir_register->set_value(item.return_address);
    
    // The has_return_value flag is stored AT FP (offset 0)
    // It's protected from user code because they must push before accessing anything
     std::cout << "4 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
    byte_t has_return_value = stack_accessor->peek_byte_from_frame(0);
    
    if(has_return_value) {
        // Pop the return value (should have been pushed by subroutine)
        word_t returnValue = stack_accessor->pop_word();
         std::cout << "5 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        // Flush clears local variables (sets FP = SP), flag is now AT FP=SP
        stack_accessor->flush();
         std::cout << "6 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        // Restore the old frame pointer
        stack_accessor->set_frame_register(item.frame_pointer);
         std::cout << "7 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        // Now pop the flag (SP points to it, FP has been restored so not empty)
        stack_accessor->pop();
         std::cout << "8 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        // Push return value back onto restored stack
        stack_accessor->push_word(returnValue);
         std::cout << "9 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
    }
    else {
        // Flush clears local variables (sets FP = SP), flag is now AT FP=SP
         std::cout << "10 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        stack_accessor->flush();
         std::cout << "11 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        // Restore the old frame pointer
        stack_accessor->set_frame_register(item.frame_pointer);
        // Now pop the flag (SP points to it, FP has been restored so not empty)
         std::cout << "12 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
        stack_accessor->pop();
         std::cout << "13 fp = " << stack_accessor->get_frame_register() << " sp = " << stack_accessor->get_top() << " size = " << stack_accessor->get_size() << std::endl;
    // Push the has_return_value flag first
    }
}