#include "cpu.h"
#include "opcodes.h"
#include "helpers.h"
#include "instruction_unit.h"
#include "stack.h"
#include "vmemunit.h"
#include "alu.h"
#include "context.h"
#include "flags.h"
#include <iostream>
#include <iomanip>

namespace lvm {

// Helper function to get concrete VMemUnit from interface
// Safe because VM always injects a VMemUnit instance
inline VMemUnit& get_concrete_vmemunit(std::shared_ptr<IVMemUnit>& interface) {
    return static_cast<VMemUnit&>(*interface);
}

    // Array indexed by opcode byte, value is number of additional parameter bytes
    // Calculated from ops.txt: BYTE=1, WORD=2, sum all arg sizes


    Cpu::Cpu(std::shared_ptr<IVMemUnit> vmem_unit, addr32_t stack_capacity, addr32_t code_capacity)
        :   vmem_unit_(std::move(vmem_unit)),
            flags(std::make_shared<Flags>()),
            AX(std::make_shared<Register>(flags)),
            BX(std::make_shared<Register>(flags)),
            CX(std::make_shared<Register>(flags)),
            DX(std::make_shared<Register>(flags)),
            EX(std::make_shared<Register>(flags)),
            alu_(std::make_unique<Alu>(AX))
        {
            // Note: Stack and InstructionUnit are now created externally
            // and passed in via set_stack() and set_instruction_unit()
            // This is because they need to be created in the proper modes
            // and CPU should depend on interfaces, not concrete types
            // ALU is created internally as it needs the AX register
            
            // Create code context
            code_context_id_ = vmem_unit_->create_context(code_capacity);
            
            // Create data context (for general purpose memory)
            data_context_id_ = vmem_unit_->create_context(65536); // 64KB data space
        }

        Cpu::~Cpu() {}

    void Cpu::set_stack(std::shared_ptr<IStack> stack) {
        stack_ = stack;
    }

    void Cpu::set_instruction_unit(std::shared_ptr<IInstructionUnit> instruction_unit) {
        instruction_unit_ = instruction_unit;
    }

    std::shared_ptr<Register> Cpu::get_register_by_code(byte_t code) {
        switch (code) {
            case REG_AX: return AX;
            case REG_BX: return BX;
            case REG_CX: return CX;
            case REG_DX: return DX;
            case REG_EX: return EX;
            default:
                std::cerr << "[CPU ERROR] Invalid register code: " << (int)code 
                          << " (0x" << std::hex << (int)code << std::dec << ")\n";
                throw std::runtime_error("Invalid register code: " + std::to_string(code));
        }
    }

    void Cpu::initialize() {
        // Initialize CPU state if needed
        if (!stack_ || !instruction_unit_) {
            throw std::runtime_error("Stack or Instruction Unit not set before CPU initialization");
        }
    }

    void Cpu::load_program(const std::vector<byte_t>& program) {
        vmem_unit_->set_mode(IVMemUnit::Mode::PROTECTED);
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_WRITE);
        accessor->Load_Program(program);
        vmem_unit_->set_mode(IVMemUnit::Mode::UNPROTECTED);
    }

    void Cpu::run() {
        vmem_unit_->set_mode(IVMemUnit::Mode::PROTECTED);
        try {
            while (!halted) {
                // a timer will be here to control processor frame rate
                step();
            }
        } catch (const std::exception& e) {
            // Log registers on error
            log_registers();
            vmem_unit_->set_mode(IVMemUnit::Mode::UNPROTECTED);
            throw;
        }
        // Log registers on normal halt
        log_registers();
        vmem_unit_->set_mode(IVMemUnit::Mode::UNPROTECTED);
    }

    void Cpu::step() {
        // for now just halt immediately
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_WRITE);
        byte_t opcode = static_cast<byte_t>(accessor->readByte_At_IR());
        accessor->advance_IR(1);
        
        std::vector<byte_t> params;
        int param_count = get_additional_bytes(opcode);
        for (int i = 0; i < param_count; ++i) {
            params.push_back(static_cast<byte_t>(accessor->readByte_At_IR()));
            accessor->advance_IR(1);
        }
        
        // Log instruction before executing
        log_instruction(opcode, params);
        
        if (opcode == OPCODE_HALT) { // HALT instruction
            halted = true;
            return;
        }
        if (opcode == OPCODE_NOP) { // NOP instruction
            return;
        }

        if(opcode >= OPCODE_LD_REG_IMM_W && opcode <= OPCODE_STAL_ADDR_REG_B) {
            execute_memory_operation(opcode, params);
            return;
        }

        // Register-indirect load operations
        if(opcode >= OPCODE_LDA_REG_REGADDR_W && opcode <= OPCODE_LDAL_REG_REGADDR_B) {
            execute_memory_operation(opcode, params);
            return;
        }

        // Immediate push operations
        if(opcode == OPCODE_PUSHW_IMM_W || opcode == OPCODE_PUSHB_IMM_B) {
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
        
        // System call
        if(opcode == OPCODE_SYS_FUNC) {
            execute_system_operation(opcode, params);
            return;
        }

        if(opcode >= OPCODE_JMP_ADDR && opcode <= OPCODE_JPNO_ADDR) {
            // Address in little-endian: params[0]=low, params[1]=high
            execute_jump(opcode, combine_bytes_to_address(params[0], params[1]));
            return;
        } 

        if(opcode >= OPCODE_CALL_ADDR && opcode <= OPCODE_RET) {
            execute_subroutine_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_ADD_IMM_W && opcode <= OPCODE_ADL_REG_B)) {
            execute_add_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_SUB_IMM_W && opcode <= OPCODE_SBL_REG_B)) {
            execute_sub_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_MUL_IMM_W && opcode <= OPCODE_MLL_REG_B)) {
            execute_mul_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_DIV_IMM_W && opcode <= OPCODE_DVL_REG_B)) {
            execute_div_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_REM_IMM_W && opcode <= OPCODE_RML_REG_B)) {
            execute_rem_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_AND_IMM_W && opcode <= OPCODE_ANL_REG_B)) {
            execute_and_operation(opcode, params);
            return;
        }

        if((opcode >= OPCODE_OR_IMM_W && opcode <= OPCODE_ORL_REG_B)) {
            execute_or_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_XOR_IMM_W && opcode <= OPCODE_XOL_REG_B)) {
            execute_xor_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_NOT_IMM_W && opcode <= OPCODE_NOTL_REG_B)) {
            execute_not_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_SHL_IMM_W && opcode <= OPCODE_SHRL_REG_B)) {
            execute_shift_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_ROL_IMM_W && opcode <= OPCODE_RORL_REG_B)) {
            execute_rotate_operation(opcode, params);
            return;
        }
        if((opcode >= OPCODE_CMP_REG_REG && opcode <= OPCODE_CPL_REG_IMM_B)) {
            execute_cmp_operation(opcode, params);
            return;
        }
        // Advance instruction pointer by param count
        // now parse and execute the instructions based on opcode and params
        std::cerr << "[CPU ERROR] Unknown opcode: 0x" << std::hex << (int)opcode << std::dec << "\n";
        throw runtime_error("Unknown opcode encountered");

    }
    
    void Cpu::execute_jump(byte_t opcode, addr_t address) {
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_WRITE);
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
                accessor->Jump_To_Address_Conditional(address, Flag::OVERFLOW, true);
                break;
            case OPCODE_JPNO_ADDR:
                accessor->Jump_To_Address_Conditional(address, Flag::OVERFLOW, false);
                break;
            default:
                throw runtime_error("Invalid jump opcode");
        
        }
    }

    void Cpu::execute_subroutine_operation(byte_t opcode, const std::vector<byte_t>& params) {
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_WRITE);
        switch(opcode) {
            case OPCODE_CALL_ADDR: {
                // Address is encoded in little-endian (low byte first)
                addr_t address = combine_bytes_to_address(params[0], params[1]);
                // CALL instruction has 3 bytes: 2 for address, 1 for return value flag
                bool return_value = (params.size() > 2) ? (params[2] != 0) : false;
                accessor->call_subroutine(address, return_value);
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
        auto stack_access = stack_->get_accessor(MemAccessMode::READ_WRITE);

        switch(opcode) {
            // load ope
            
            case OPCODE_LD_REG_IMM_W: {
                auto reg = get_register_by_code(params[0]);
                // params[1] is low byte, params[2] is high byte (little-endian)
                word_t value = combine_bytes_to_word(params[1], params[2]);
                reg->get_accessor()->set_value(value);
                break;
            }

            case OPCODE_LDA_REG_ADDR_W: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                
                // Calculate page and offset (64KB pages)
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                
                word_t value = data_accessor->read_word(offset);
                reg->get_accessor()->set_value(value);
                break;
            }

            case OPCODE_LDAB_REG_ADDR_B: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
                
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                
                byte_t value = data_accessor->read_byte(offset);
                reg->get_accessor()->set_low_byte(value);  // LDAB loads into low byte
                break;
            }

            case OPCODE_LD_REG_REG_W: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->get_accessor()->set_value(src_reg->get_accessor()->get_value());
                break;
            }

            case OPCODE_STA_ADDR_REG_W: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                word_t value = reg->get_accessor()->get_value();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                data_accessor->write_word(offset, value);
                break;
            }
            case OPCODE_LDH_REG_IMM_B: {
            
                auto reg = get_register_by_code(params[0]);
                byte_t value = params[1];
                reg->get_accessor()->set_high_byte(value);
                break;
            }
            case OPCODE_LDH_REG_REG_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->get_accessor()->set_high_byte(src_reg->get_accessor()->get_high_byte());
                break;
            }
            case OPCODE_LDAH_REG_ADDR_B: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                reg->get_accessor()->set_high_byte(value);
                break;
            }

            case OPCODE_STAH_ADDR_REG_B: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                byte_t value = reg->get_accessor()->get_high_byte();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                data_accessor->write_byte(offset, value);
                break;
            }

            case OPCODE_LDL_REG_IMM_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = params[1];
                reg->get_accessor()->set_low_byte(value);
                break;
            }
            case OPCODE_LDL_REG_REG_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto src_reg = get_register_by_code(params[1]);
                dest_reg->get_accessor()->set_low_byte(src_reg->get_accessor()->get_low_byte());
                break;
            }
            case OPCODE_LDAL_REG_ADDR_B: {
                auto reg = get_register_by_code(params[0]);
                addr32_t address = combine_bytes_to_address(params[1], params[2]);
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                reg->get_accessor()->set_low_byte(value);
                break;
            }       
            case OPCODE_STAL_ADDR_REG_B: {
                addr32_t address = combine_bytes_to_address(params[0], params[1]);
                auto reg = get_register_by_code(params[2]);
                byte_t value = reg->get_accessor()->get_low_byte();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                data_accessor->write_byte(offset, value);
                break;
            }

            // Register-indirect load operations
            case OPCODE_LDA_REG_REGADDR_W: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_accessor()->get_value();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                word_t value = data_accessor->read_word(offset);
                dest_reg->get_accessor()->set_value(value);
                break;
            }
            case OPCODE_LDAH_REG_REGADDR_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_accessor()->get_value();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                dest_reg->get_accessor()->set_high_byte(value);
                break;
            }
            case OPCODE_LDAL_REG_REGADDR_B: {
                auto dest_reg = get_register_by_code(params[0]);
                auto addr_reg = get_register_by_code(params[1]);
                addr32_t address = addr_reg->get_accessor()->get_value();
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_ONLY);
                page_t page = address >> 16;  // High 16 bits
                addr_t offset = address & 0xFFFF;  // Low 16 bits
                data_accessor->set_page(page);
                byte_t value = data_accessor->read_byte(offset);
                dest_reg->get_accessor()->set_low_byte(value);
                break;
            }   

            case OPCODE_SWP_REG_REG: {
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                word_t temp = reg1->get_accessor()->get_value();
                reg1->get_accessor()->set_value(reg2->get_accessor()->get_value());
                reg2->get_accessor()->set_value(temp);
                break;
            }
            // stack operations

            case OPCODE_PUSHW_IMM_W: {
                // params are in little-endian order: low byte first, high byte second
                word_t value = combine_bytes_to_address(params[0], params[1]);
                stack_access->push_word(value);
                break;
            }
            case OPCODE_PUSHB_IMM_B: {
                byte_t value = params[0];
                stack_access->push_byte(value);
                break;
            }
            case OPCODE_PUSH_REG_W: {
                auto reg = get_register_by_code(params[0]);
                stack_access->push_word(reg->get_accessor()->get_value());
                break;
            }
            case OPCODE_POP_REG_W: {
                auto reg = get_register_by_code(params[0]);
                word_t value = stack_access->pop_word();
                reg->get_accessor()->set_value(value);
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
                reg->get_accessor()->set_high_byte(value);
                break;
            }
            case OPCODE_PUSHL_REG_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = reg->get_accessor()->get_low_byte();
                stack_access->push_byte(value);
                break;
            }
            case OPCODE_POPL_REG_B: {
                auto reg = get_register_by_code(params[0]);
                byte_t value = stack_access->pop_byte();
                reg->get_accessor()->set_low_byte(value);
                break;
            }
            case OPCODE_PEEK_REG_OFF_W: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                word_t value = stack_access->peek_word_from_base(offset);
                reg->get_accessor()->set_value(value);
                break;
            }
            case OPCODE_PEEKF_REG_OFF_W: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                word_t value = stack_access->peek_word_from_frame(offset);
                reg->get_accessor()->set_value(value);
                break;
            }
            case OPCODE_PEEKB_REG_OFF_B: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                byte_t value = stack_access->peek_byte_from_base(offset);
                reg->get_accessor()->set_low_byte(value);
                break;
            }
            case OPCODE_PEEKFB_REG_OFF_B: {
                auto reg = get_register_by_code(params[0]);
                page_offset_t offset = combine_bytes_to_address(params[1], params[2]);
                byte_t value = stack_access->peek_byte_from_frame(offset);
                reg->get_accessor()->set_low_byte(value);
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
                // Set page for data context via accessor
                // params[0-1]: page number (16-bit little-endian)
                // params[2-3]: context id (16-bit little-endian) - currently ignored
                page_t page = combine_bytes_to_word(params[0], params[1]);
                
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                data_accessor->set_page(page);
                break;
            }
            case OPCODE_PAGE_REG_CTX: {
                // Set page from register for data context via accessor
                // params[0]: register code
                // params[1-2]: context id (16-bit little-endian) - currently ignored
                auto reg = get_register_by_code(params[0]);
                page_t page = reg->get_accessor()->get_value();
                
                auto data_ctx = vmem_unit_->get_context(data_context_id_);
                auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
                data_accessor->set_page(page);
                break;
            }
            default:
                std::cerr << "[CPU ERROR] Invalid stack operation opcode: 0x" 
                          << std::hex << (int)opcode << std::dec << "\n";
                throw runtime_error("Invalid stack operation opcode");
        }
    }

    void Cpu::execute_inc_dec_operation(byte_t opcode, const std::vector<byte_t>& params) {
        switch(opcode) {
            case OPCODE_INC_REG:
                {
                    auto reg = get_register_by_code(params[0]);
                    reg->get_accessor()->inc();
                }
                break;
            case OPCODE_DEC_REG:
                {
                    auto reg = get_register_by_code(params[0]);
                    reg->get_accessor()->dec();
                }
                break;
            default:
                throw runtime_error("Invalid inc/dec opcode");
        }
    }

    void Cpu::execute_system_operation(byte_t opcode, const std::vector<byte_t>& params) {
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_WRITE);
        switch(opcode) {
            case OPCODE_SYS_FUNC:
                {
                    // params are in little-endian order: low byte first, high byte second
                    word_t syscall_number = combine_bytes_to_word(params[0], params[1]);
                    accessor->system_call(syscall_number);  
                    break;
                }
            default:
                throw runtime_error("Invalid system operation opcode");
        }   
    }

    void Cpu::enable_logging(const std::string& logfile) {
        log_file_.open(logfile);
        if (!log_file_.is_open()) {
            throw runtime_error("Failed to open log file: " + logfile);
        }
        logging_enabled_ = true;
        log_file_ << "=== Pendragon VM Execution Log ===" << std::endl;
        log_file_ << std::endl;
    }

    void Cpu::log_instruction(byte_t opcode, const std::vector<byte_t>& params) {
        if (!logging_enabled_) return;
        
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_ONLY);
        addr_t ir_value = accessor->get_IR();
        
        log_file_ << "IR: 0x" << std::hex << std::setw(4) << std::setfill('0') << ir_value 
                  << " | Opcode: 0x" << std::setw(2) << static_cast<int>(opcode);
        
        if (!params.empty()) {
            log_file_ << " | Params: ";
            for (size_t i = 0; i < params.size(); ++i) {
                if (i > 0) log_file_ << " ";
                log_file_ << "0x" << std::setw(2) << static_cast<int>(params[i]);
            }
        }
        
        auto ax_acc = AX->get_accessor();
        auto bx_acc = BX->get_accessor();
        auto cx_acc = CX->get_accessor();
        auto dx_acc = DX->get_accessor();
        auto ex_acc = EX->get_accessor();
        
        log_file_ << " | AX: 0x" << std::setw(4) << ax_acc->get_value()
                  << " BX: 0x" << std::setw(4) << bx_acc->get_value()
                  << " CX: 0x" << std::setw(4) << cx_acc->get_value()
                  << " DX: 0x" << std::setw(4) << dx_acc->get_value()
                  << " EX: 0x" << std::setw(4) << ex_acc->get_value();
        
        log_file_ << std::dec << std::endl;
    }

    void Cpu::log_registers() {
        if (!logging_enabled_) return;
        
        auto ax_acc = AX->get_accessor();
        auto bx_acc = BX->get_accessor();
        auto cx_acc = CX->get_accessor();
        auto dx_acc = DX->get_accessor();
        auto ex_acc = EX->get_accessor();
        
        log_file_ << std::endl;
        log_file_ << "=== Final Register State ===" << std::endl;
        log_file_ << "AX: 0x" << std::hex << std::setw(4) << std::setfill('0') << ax_acc->get_value() << std::endl;
        log_file_ << "BX: 0x" << std::setw(4) << bx_acc->get_value() << std::endl;
        log_file_ << "CX: 0x" << std::setw(4) << cx_acc->get_value() << std::endl;
        log_file_ << "DX: 0x" << std::setw(4) << dx_acc->get_value() << std::endl;
        log_file_ << "EX: 0x" << std::setw(4) << ex_acc->get_value() << std::endl;
        
        auto accessor = instruction_unit_->get_accessor(MemAccessMode::READ_ONLY);
        log_file_ << "IR: 0x" << std::setw(4) << accessor->get_IR() << std::endl;
        log_file_ << "SP: 0x" << std::setw(4) << stack_->get_sp() << std::endl;
        
        log_file_ << "Flags: ";
        log_file_ << "Z=" << (flags->is_set(Flag::ZERO) ? "1" : "0") << " ";
        log_file_ << "C=" << (flags->is_set(Flag::CARRY) ? "1" : "0") << " ";
        log_file_ << "S=" << (flags->is_set(Flag::SIGN) ? "1" : "0") << " ";
        log_file_ << "O=" << (flags->is_set(Flag::OVERFLOW) ? "1" : "0");
        log_file_ << std::dec << std::endl;
    }

} // namespace lvm