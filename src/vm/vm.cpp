#include "vm.h"

using namespace lvm;

vm::vm(addr32_t stack_capacity, addr32_t code_capacity)
    : vmem_unit(std::make_shared<VMemUnit>()),
      flags(std::make_shared<Flags>())
{
    // Create CPU first (creates contexts)
    cpu_instance = std::make_shared<Cpu>(vmem_unit, stack_capacity, code_capacity);
    
    // Create stack in UNPROTECTED mode
    stack = std::make_shared<Stack>(vmem_unit, stack_capacity);
    
    // Create BasicIO with stack and memory
    basic_io = std::make_shared<BasicIO>(vmem_unit, stack);
    
    // Create code context for instruction unit
    code_context_id_ = vmem_unit->create_context(code_capacity);
    
    // Create instruction unit in UNPROTECTED mode with stack interface
    instruction_unit = std::make_shared<InstructionUnit>(vmem_unit, code_context_id_, *stack, flags, basic_io);
    
    // Inject dependencies into CPU
    cpu_instance->set_stack(stack);
    cpu_instance->set_instruction_unit(instruction_unit);
    
    // Initialize CPU
    cpu_instance->initialize();
}

vm::~vm() {
    // Destructor
}

void vm::load_program(char* fileName, addr_t load_address) {
    // Load program from file into memory at specified address
    // (File loading code not implemented here)
    std::vector<byte_t> program = {/* ... load from file ... */};
    cpu_instance->load_program(program);
}

void vm::run() {
    cpu_instance->run();
}

