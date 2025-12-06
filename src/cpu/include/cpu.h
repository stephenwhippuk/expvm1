#pragma once
#include "memsize.h"
#include <vector>
#include "memunit.h"
#include "stack.h"
#include "errors.h"
#include "mem_access.h"
#include "register.h"
#include "instruction_unit.h"
#include "alu.h"
#include "vaddr.h"
#include <memory>

namespace lvm {
    enum register_codes {
        REG_AX = 0x01,
        REG_BX = 0x02,
        REG_CX = 0x03,
        REG_DX = 0x04,
        REG_EX = 0x05,
        REG_IR = 0x06,
        REG_SP = 0x07,
        REG_SI,
    };
    class Cpu{
    public:
        Cpu(std::shared_ptr<Memory> memory, memsize_t stack_size, addr_t code_start_address, memsize_t code_size);
        ~Cpu();
        void initialize();
        void load_program(const std::vector<byte_t>& program);
        void run();
    private:
        std::shared_ptr<Memory> memory_ref;
        Stack stack;
        std::unique_ptr<MemoryAccessor> code_accessor;
        bool halted = false;
        void step();
        // Additional CPU state (registers, flags, etc.) would go here
        // Gneeral purpose regsisters
        std::shared_ptr<Register> AX;
        std::shared_ptr<Register> BX;
        std::shared_ptr<Register> CX;
        std::shared_ptr<Register> DX;
        std::shared_ptr<Register> EX;

        std::unique_ptr<Alu> alu;
        std::shared_ptr<Flags> flags;
        std::unique_ptr<InstructionUnit> instruction_unit;

        std::shared_ptr<Register> get_register_by_code(byte_t code);
        void execute_jump(byte_t opcode, addr_t address);
        void execute_add_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_sub_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_mul_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_div_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_rem_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_and_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_or_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_xor_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_shift_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_rotate_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_cmp_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_memory_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_inc_dec_operation(byte_t opcode, const std::vector<byte_t>& params);
        void execute_subroutine_operation(byte_t opcode, const std::vector<byte_t>& params);
    };  
}

