#pragma once
#include "memsize.h"
#include <vector>
#include "memunit.h"
#include "stack.h"
#include "errors.h"
#include "mem_access.h"

namespace lvm {
    class Cpu{
    public:
        Cpu(Memory& memory, Stack& stack);
        ~Cpu();
        void initialize();
        void load_program(const std::vector<byte_t>& program, addr_t load_address);
        void run();
    private:
        Memory& memory_ref;
        Stack& stack_ref;
        std::unique_ptr<MemoryAccessor> code_accessor;
        bool halted = false;
        void step();
    };  
}

