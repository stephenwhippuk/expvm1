#include "vm.h"

using namespace lvm;

vm::vm(page_t pages, memsize_t page_size, memsize_t stack_size)
    : memory(std::make_shared<Memory>(page_size, pages)),
      cpu_instance(std::make_shared<Cpu>(memory, stack_size, 0, 32765))
{
    // VM initialized with memory and stack
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

