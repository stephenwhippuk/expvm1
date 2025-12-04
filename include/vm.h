#pragma once
#include "memsize.h"
#include <vector>
#include "memunit.h"
#include "stack.h"
#include "cpu.h"
#include <memory>
namespace lvm {
    class vm{
    public:
        vm(page_t pages, memsize_t page_size, memsize_t stack_size);
        ~vm();
        void load_program(char* fileName, addr_t load_address);
        void run();
    private:
        std::shared_ptr<Memory> memory;
        std::shared_ptr<Cpu> cpu_instance;
    };
}