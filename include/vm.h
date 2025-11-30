#pragma once
#include "memsize.h"
#include <vector>
#include "memunit.h"
#include "stack.h"
#include "cpu.h"

namespace lvm {
    class vm{
    public:
        vm(page_t pages, memsize_t page_size, memsize_t stack_size);
        ~vm();
        void load_program(char* fileName, addr_t load_address);
        void run();
    private:
        Memory memory;
        Stack stack;
        Cpu cpu_instance;
    };
}