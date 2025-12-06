#include "cpu.h"
#include "opcodes.h"
#include "helpers.h"
namespace lvm {

    // Array indexed by opcode byte, value is number of additional parameter bytes
    // Calculated from ops.txt: BYTE=1, WORD=2, sum all arg sizes


    Cpu::Cpu(VMemUnit& vmem_unit, addr32_t stack_capacity, addr32_t code_capacity)
        :   vmem_unit_(vmem_unit),
            flags(std::make_shared<Flags>()),
            AX(std::make_shared<Register>(flags)),
            BX(std::make_shared<Register>(flags)),
            CX(std::make_shared<Register>(flags)),
            DX(std::make_shared<Register>(flags)),
            EX(std::make_shared<Register>(flags)),
            alu(std::make_unique<Alu>(AX))
        {
            // Create stack in own context
            stack = std::make_unique<Stack2>(vmem_unit_, stack_capacity);
            
            // Create code context
            code_context_id_ = vmem_unit_.create_context(code_capacity);
            
            // Create data context (for general purpose memory)
            data_context_id_ = vmem_unit_.create_context(65536); // 64KB data space
            
            // Create instruction unit with stack accessor
            vmem_unit_.set_mode(VMemUnit::Mode::PROTECTED);
            auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
            vmem_unit_.set_mode(VMemUnit::Mode::UNPROTECTED);
            
            instruction_unit = std::make_unique<InstructionUnit>(vmem_unit_, code_context_id_, std::move(stack_accessor), flags);
        }

        Cpu::~Cpu() {}

    std::shared_ptr<Register> Cpu::get_register_by_code(byte_t code) {
        switch (code) {
            case REG_AX: return AX;
            case REG_BX: return BX;
            case REG_CX: return CX;
            case REG_DX: return DX;
            case REG_EX: return EX;
            default:
                throw std::runtime_error("Invalid register code: " + std::to_string(code));
        }
    }

    void Cpu::initialize() {
        // Initialize CPU state if needed
    }

    void Cpu::load_program(const std::vector<byte_t>& program) {
        vmem_unit_.set_mode(VMemUnit::Mode::PROTECTED);
        auto accessor = instruction_unit->get_accessor(MemAccessMode::READ_WRITE);
        accessor->Load_Program(program);
    }

    void Cpu::run() {
        vmem_unit_.set_mode(VMemUnit::Mode::PROTECTED);
        while (!halted) {
            // a timer will be here to control processor frame rate
            step();
        }
        vmem_unit_.set_mode(VMemUnit::Mode::UNPROTECTED);
    }

    void Cpu::step() {
        // for now just halt immediately
        auto accessor = instruction_unit->get_accessor(MemAccessMode::READ_WRITE);
        byte_t opcode = static_cast<byte_t>(accessor->readByte_At_IR());
        accessor->advance_IR(1);
        if (opcode == OPCODE_HALT) { // HALT instruction
            halted = true;
            return;
        }
        if (opcode == OPCODE_NOP) { // NOP instruction
            return;
        }

        std::vector<byte_t> params;
        int param_count = get_additional_bytes(opcode);
        for (int i = 0; i < param_count; ++i) {
            params.push_back(static_cast<byte_t>(accessor->readByte_At_IR() + 1 + i));
            accessor->advance_IR(1);
        }
        accessor->advance_IR(1); // Move past opcode

        if(opcode >= OPCODE_LD_REG_IMM_W && opcode <= OPCODE_STAL_ADDR_REG_B) {
            execute_memory_operation(opcode, params);
            return;
        }

        // Register-indirect load operations
        if(opcode >= OPCODE_LDA_REG_REGADDR_W && opcode <= OPCODE_LDAL_REG_REGADDR_B) {
            execute_memory_operation(opcode, params);
            return;
        }

        if(opcode >= OPCODE_PUSH_REG_W && opcode <= OPCODE_SETF_ADDR) {
            execute_memory_operation(opcode, params);
            return;
        }

        if(opcode >= OPCODE_INC_REG && opcode <= OPCODE_DEC_REG) {
            execute_inc_dec_operation(opcode, params);
            return;
        }

        if(opcode >= OPCODE_JMP_ADDR && opcode <= OPCODE_JPNO_ADDR) {
            execute_jump(opcode, combine_bytes_to_address(params[0], params[1]));
            return;
        } 

        if(opcode >= OPCODE_CALL_ADDR && opcode <= OPCODE_RET) {
            execute_subroutine_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_ADD_REG_W && opcode <= OPCODE_ADL_REG_B)) {
            execute_add_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_SUB_REG_W && opcode <= OPCODE_SBL_REG_B)) {
            execute_sub_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_MUL_REG_W && opcode <= OPCODE_MLL_REG_B)) {
            execute_mul_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_DIV_REG_W && opcode <= OPCODE_DVL_REG_B)) {
            execute_div_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_REM_REG_W && opcode <= OPCODE_RML_REG_B)) {
            execute_rem_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_AND_REG_W && opcode <= OPCODE_ANL_REG_B)) {
            execute_and_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_OR_REG_W && opcode <= OPCODE_ORL_REG_B)) {
            execute_or_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_XOR_REG_W && opcode <= OPCODE_XOL_REG_B)) {
            execute_xor_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_SHL_REG_W && opcode <= OPCODE_SHRL_REG_B)) {
            execute_shift_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_ROL_REG_W && opcode <= OPCODE_RORL_REG_B)) {
            execute_rotate_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_CMP_REG_REG && opcode <= OPCODE_CPL_REG_IMM_B)) {
            execute_cmp_operation(opcode, params);
            return;
        }
        // Advance instruction pointer by param count
        // now parse and execute the instructions based on opcode and params
        throw runtime_error("Unknown opcode encountered");

    }
    
    void Cpu::execute_jump(byte_t opcode, addr_t address) {
        auto accessor = instruction_unit->get_accessor(MemAccessMode::READ_WRITE);
        switch(opcode) {
            case OPCODE_JMP_ADDR:
                accessor->Jump_To_Address(address);
                break;
            case OPCODE_JPZ_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::ZERO, true);
                break;
            case OPCODE_JPNZ_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::ZERO, false);
                break;
            case OPCODE_JPC_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::CARRY, true);
                break;
            case OPCODE_JPNC_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::CARRY, false);
                break;
            case OPCODE_JPS_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::SIGN, true);
                break;
            case OPCODE_JPNS_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::SIGN, false);
                break;
            case OPCODE_JPO_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::OVERFLOW, false);
                break;
            case OPCODE_JPNO_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::OVERFLOW, true);
                break;
            default:
                throw runtime_error("Invalid jump opcode");
        
        }
    }

    void Cpu::execute_subroutine_operation(byte_t opcode, const std::vector<byte_t>& params) {
        auto accessor = instruction_unit->get_accessor(MemAccessMode::READ_WRITE);
        switch(opcode) {
            case OPCODE_CALL_ADDR: {
                addr_t address = combine_bytes_to_address(params[0], params[1]);
                accessor->call_subroutine(address, params[2]);
                break;
            }
            case OPCODE_RET: {
                accessor->return_from_subroutine();
                break;
            }
            default:
                throw runtime_error("Invalid subroutine opcode");
        }
    }

    // stack ops
    void Cpu::execute_memory_operation(byte_t opcode, const std::vector<byte_t>& params) {
        auto stack_access = stack->get_accessor(MemAccessMode::READ_WRITE);

        switch(opcode) {
            // load ope
            
            case OPCODE_LD_REG_IMM_W: {
                auto reg = get_register_by_code(params[0]);
                word_t value = combine_bytes_to_word(params[1], params[2]);
                reg->set_value(value);
                break;
            }

            case OPCODE_LDA_REG_ADDR_W: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_WRITE);
                
                // Calculate page and offset
                page_t page = address / 256; // Assuming 256 bytes per page
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                
                word_t value = data_accessor->read_word(offset);
                reg->set_value(value);
                break;
            }

            case OPCODE_LD_REG_REG_W: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->set_value(src_reg->get_value());
                break;
            }

            case OPCODE_STA_ADDR_REG_W: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                word_t value = reg->get_value();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_WRITE);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                data_accessor->write_word(offset, value);
                break;
            }
            case OPCODE_LDH_REG_IMM_B: {
            
                auto reg = get_register_by_code(params[0]);
                byte_t value = params[1];
                reg->set_high_byte(value);
                break;
            }
            case OPCODE_LDH_REG_REG_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->set_high_byte(src_reg->get_high_byte());
                break;
            }
            case OPCODE_LDAH_REG_ADDR_B: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_ONLY);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                reg->set_high_byte(value);
                break;
            }

            case OPCODE_STAH_ADDR_REG_B: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                byte_t value = reg->get_high_byte();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_WRITE);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                data_accessor->write_byte(offset, value);
                break;
            }

            case OPCODE_LDL_REG_IMM_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = params[1];
                reg->set_low_byte(value);
                break;
            }
            case OPCODE_LDL_REG_REG_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->set_low_byte(src_reg->get_low_byte());
                break;
            }
            case OPCODE_LDAL_REG_ADDR_B: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_ONLY);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                reg->set_low_byte(value);
                break;
            }       
            case OPCODE_STAL_ADDR_REG_B: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                byte_t value = reg->get_low_byte();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_WRITE);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                data_accessor->write_byte(offset, value);
                break;
            }

            // Register-indirect load operations
            case OPCODE_LDA_REG_REGADDR_W: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_value();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_ONLY);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                word_t value = data_accessor->read_word(offset);
                dest_reg->set_value(value);
                break;
            }
            case OPCODE_LDAH_REG_REGADDR_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_value();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_ONLY);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                dest_reg->set_high_byte(value);
                break;
            }
            case OPCODE_LDAL_REG_REGADDR_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_value();
                const Context* data_ctx = vmem_unit_.get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(vmem_unit_, MemAccessMode::READ_ONLY);
                page_t page = address / 256;
                addr_t offset = address % 256;
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                dest_reg->set_low_byte(value);
                break;
            }   

            case OPCODE_SWP_REG_REG: {
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                word_t temp = reg1->get_value();
                reg1->set_value(reg2->get_value());
                reg2->set_value(temp);
                break;
            }
            // stack operations

            case OPCODE_PUSH_REG_W: {
                auto reg = get_register_by_code(params[0]);
                stack_access->push_word(reg->get_value());
                break;
            }
            case OPCODE_POP_REG_W: {
                auto reg = get_register_by_code(params[0]);
                word_t value = stack_access->pop_word();
                reg->set_value(value);
                break;
            }
            case OPCODE_PUSHH_REG_B: {
                byte_t value = params[0];
                stack_access->push_byte(value);
                break;
            }
            case OPCODE_POPH_REG_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = stack_access->pop_byte();
                reg->set_high_byte(value);
                break;
            }
            case OPCODE_PUSHL_REG_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = reg->get_low_byte();
                stack_access->push_byte(value);
                break;
            }
            case OPCODE_POPL_REG_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = stack_access->pop_byte();
                reg->set_low_byte(value);
                break;
            }
            case OPCODE_PEEK_REG_OFF_W: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                word_t value = stack_access->peek_word_from_base(offset);
                reg->set_value(value);
                break;
            }
            case OPCODE_PEEKF_REG_OFF_W: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                word_t value = stack_access->peek_word_from_frame(offset);
                reg->set_value(value);
                break;
            }
            case OPCODE_PEEKB_REG_OFF_B: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                byte_t value = stack_access->peek_byte_from_base(offset);
                reg->set_low_byte(value);
                break;
            }
            case OPCODE_PEEKFB_REG_OFF_B: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                byte_t value = stack_access->peek_byte_from_frame(offset);
                reg->set_low_byte(value);
                break;
            }
            case OPCODE_FLSH: {
                stack_access->flush();
                break;
            }
            case OPCODE_SETF_ADDR: {
                int32_t address = static_cast<int32_t>(combine_bytes_to_address(params[0], params[1]));
                stack_access->set_frame_pointer(address);
                break;
            }

            case OPCODE_PAGE_IMM_CTX: {
                // Context switching is now handled at accessor level
                // This opcode is deprecated with the new memory system
                break;
            }
            case OPCODE_PAGE_REG_CTX: {
                // Context switching is now handled at accessor level
                // This opcode is deprecated with the new memory system
                break;
            }
            default:
                throw runtime_error("Invalid stack operation opcode");
        }
    }

    void Cpu::execute_inc_dec_operation(byte_t opcode, const std::vector<byte_t>& params) {
        switch(opcode) {
            case OPCODE_INC_REG:
                {
                    auto reg = get_register_by_code(params[0]);
                    reg->inc();
                }
                break;
            case OPCODE_DEC_REG:
                {
                    auto reg = get_register_by_code(params[0]);
                    reg->dec();
                }
                break;
            default:
                throw runtime_error("Invalid inc/dec opcode");
        }
    }


    // Additional CPU methods would go here

} // namespace lvm