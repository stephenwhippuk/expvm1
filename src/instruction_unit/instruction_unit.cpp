#include "instruction_unit.h"
#include "errors.h"
#include "stack_new.h"
#include "vmemunit.h"

#include <iostream>
using namespace lvm;

InstructionUnit::InstructionUnit(VMemUnit& vmem_unit, context_id_t code_context_id, std::unique_ptr<StackAccessor2> stack_ptr, std::shared_ptr<Flags> flags_ptr)
       : vmem_unit_(vmem_unit),
         code_context_id_(code_context_id),
         stack_accessor(std::move(stack_ptr))
{
    // assert stack_ptr is valid
    if (!stack_accessor) {
        throw lvm::runtime_error("InstructionUnit requires a valid StackAccessor2");
    }
    // assert memory is in unprotected mode
    if(vmem_unit_.is_protected()) {
        throw lvm::runtime_error("InstructionUnit must be created in unprotected mode");
    }

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
    const Context* code_ctx = vmem_unit_.get_context(code_context_id_);
    auto code_accessor = code_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_WRITE);
    
    // Write program to code space across pages (optimized)
    const size_t PAGE_SIZE = 256; // Assuming 256 bytes per page
    size_t program_size = program.size();
    addr32_t addr = 0;
    while (addr < program_size) {
        page_t page = addr / PAGE_SIZE;
        code_accessor->set_page(page);
        // Write all bytes for this page in one go
        addr_t offset = addr % PAGE_SIZE;
        size_t bytes_left_in_page = PAGE_SIZE - offset;
        size_t bytes_left_in_program = program_size - addr;
        size_t chunk_size = std::min(bytes_left_in_page, bytes_left_in_program);
        for (size_t i = 0; i < chunk_size; ++i) {
            code_accessor->write_byte(offset + i, program[addr + i]);
        }
        addr += chunk_size;
    }
}

void InstructionUnit::call_subroutine(addr_t address, bool with_return_value){

    ReturnStackItem item;
    item.return_address = ir_register->get_value();
    item.frame_pointer = stack_accessor->get_fp();

    return_stack.push_back(item);

    ir_register->set_value(address);

    if (with_return_value) {
        stack_accessor->push_byte(1);
    }
    else {
        stack_accessor->push_byte(0);
    }

    stack_accessor->set_frame_to_top();
   
}

void InstructionUnit::return_from_subroutine() {
    if (return_stack.empty()) {
        throw lvm::runtime_error("Return stack underflow on return from subroutine");
    }

    ReturnStackItem item = return_stack.back();
    return_stack.pop_back();

    ir_register->set_value(item.return_address);
    

    byte_t has_return_value = stack_accessor->peek_byte_from_frame(0);
    
    if(has_return_value) {
       
        word_t returnValue = stack_accessor->pop_word(); 
        stack_accessor->flush();
        stack_accessor->set_frame_pointer(item.frame_pointer);
        stack_accessor->pop_byte();
        stack_accessor->push_word(returnValue);
    }
    else {
        stack_accessor->flush();
        stack_accessor->set_frame_pointer(item.frame_pointer);
        stack_accessor->pop_byte();
    }
}