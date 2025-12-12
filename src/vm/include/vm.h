#pragma once
#include "memsize.h"
#include <vector>
#include "vmemunit.h"
#include "stack.h"
#include "instruction_unit.h"
#include "cpu.h"
#include "basic_io.h"
#include <memory>
namespace lvm {
    class vm{
    public:
        vm(addr32_t stack_capacity, addr32_t code_capacity, addr32_t data_capacity);
        ~vm();
        void load_program(char* fileName, addr_t load_address);
        void run();
    private:
        std::shared_ptr<VMemUnit> vmem_unit;
        std::shared_ptr<Stack> stack;
        std::shared_ptr<BasicIO> basic_io;
        std::shared_ptr<InstructionUnit> instruction_unit;
        std::shared_ptr<Cpu> cpu_instance;
        std::shared_ptr<Flags> flags;
        context_id_t code_context_id_;
        context_id_t data_context_id_;
    };
}