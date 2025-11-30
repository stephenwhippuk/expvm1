#include "cpu.h"

namespace lvm {

    Cpu::Cpu(Memory& memory, Stack& stack)
        : memory_ref(memory), stack_ref(stack) {}

    Cpu::~Cpu() {}

    void Cpu::initialize() {
        // Initialize CPU state if needed
    }

    void Cpu::load_program(const std::vector<byte_t>& program, addr_t load_address) {
        memory_ref.unprotected_mode();
        code_accessor = memory_ref.reserve_space(0, load_address, static_cast<memsize_t>(program.size()), MemAccessMode::READ_WRITE);
        memory_ref.protected_mode();
        code_accessor->bulk_write(0, program);
    }

    void Cpu::run() {
        while (!halted) {
            step();
        }
    }

    void Cpu::step() {
        // for now just halt immediately
        halted = true;
    }

} // namespace lvm