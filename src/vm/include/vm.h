#pragma once
#include "memsize.h"
#include <vector>
#include "vmemunit.h"
#include "stack_new.h"
#include "cpu.h"
#include <memory>
namespace lvm {
    class vm{
    public:
        vm(addr32_t stack_capacity, addr32_t code_capacity);
        ~vm();
        void load_program(char* fileName, addr_t load_address);
        void run();
    private:
        std::unique_ptr<VMemUnit> vmem_unit;
        std::shared_ptr<Cpu> cpu_instance;
    };
}