#include "vm.h"

using namespace lvm;

vm::vm(addr32_t stack_capacity, addr32_t code_capacity)
    : vmem_unit(std::make_unique<VMemUnit>()),
      cpu_instance(std::make_shared<Cpu>(*vmem_unit, stack_capacity, code_capacity))
{
    // VM initialized with virtual memory unit
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

