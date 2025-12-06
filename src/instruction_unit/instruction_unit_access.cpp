#include "instruction_unit.h"
#include "errors.h"
#include "stack_new.h"
#include "vmemunit.h"

using namespace lvm;

InstructionUnit_Accessor::InstructionUnit_Accessor(InstructionUnit* iu, MemAccessMode mode)
    : instruction_unit_ref(iu), mode(mode) {}    

InstructionUnit_Accessor::~InstructionUnit_Accessor() {}

// Read Access Methods

word_t InstructionUnit_Accessor::get_IR() const {
    return instruction_unit_ref->ir_register->get_value();
}

word_t InstructionUnit_Accessor::readByte_At_IR() const {
    addr32_t ir_value = instruction_unit_ref->ir_register->get_value();
    const Context* code_ctx = instruction_unit_ref->vmem_unit_.get_context(instruction_unit_ref->code_context_id_);
    auto code_accessor = code_ctx->create_paged_accessor(instruction_unit_ref->vmem_unit_, MemAccessMode::READ_ONLY);
    
    page_t page = ir_value / 256;  // Assuming 256 bytes per page
    addr_t offset = ir_value % 256;
    code_accessor->set_page(page);
    
    return code_accessor->read_byte(offset);
}

word_t InstructionUnit_Accessor::readWWord_At_IR() const {
    addr32_t ir_value = instruction_unit_ref->ir_register->get_value();
    const Context* code_ctx = instruction_unit_ref->vmem_unit_.get_context(instruction_unit_ref->code_context_id_);
    auto code_accessor = code_ctx->create_paged_accessor(instruction_unit_ref->vmem_unit_, MemAccessMode::READ_ONLY);
    
    page_t page = ir_value / 256;
    addr_t offset = ir_value % 256;
    code_accessor->set_page(page);
    
    return code_accessor->read_word(offset);
}

// Read/Write Access Methods

void InstructionUnit_Accessor::advance_IR(word_t offset) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to advance IR in READ_ONLY mode");
    }
    instruction_unit_ref->advance_IR(offset);
}

void InstructionUnit_Accessor::set_IR(word_t value) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to set IR in READ_ONLY mode");
    }
    instruction_unit_ref->set_IR(value);
}

void InstructionUnit_Accessor::Jump_To_Address(addr_t address) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to jump to address in READ_ONLY mode");
    }
    instruction_unit_ref->jump_to_address(address);
}

void InstructionUnit_Accessor::Jump_To_Address_Conditional(addr_t address, Flag flag, bool condition) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to jump to address conditional in READ_ONLY mode");
    }
    instruction_unit_ref->jump_to_address_conditional(address, flag, condition);
}

void InstructionUnit_Accessor::Load_Program(const std::vector<byte_t>& program) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to load program in READ_ONLY mode");
    }
    instruction_unit_ref->load_program(program);
}

// subroutines

void InstructionUnit_Accessor::call_subroutine(addr_t address, bool with_return_value) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to call subroutine in READ_ONLY mode");
    }
    instruction_unit_ref->call_subroutine(address, with_return_value);
}

void InstructionUnit_Accessor::return_from_subroutine() {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to return from subroutine in READ_ONLY mode");
    }
    instruction_unit_ref->return_from_subroutine();
}

