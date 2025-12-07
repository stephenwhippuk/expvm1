#include "opcodes.h"
#include "cpu.h"
#include "helpers.h"
using namespace lvm;

void Cpu::execute_add_operation(byte_t opcode, const std::vector<byte_t>& params) {
        switch(opcode) {
            case OPCODE_ADD_REG_W:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu_->add(reg->get_value());
                }
                break;
            case OPCODE_ADD_IMM_W:
                {
                    word_t value = combine_bytes_to_word(params[0], params[1]);
                    alu_->add(value);
                }
                break;
            case OPCODE_ADB_IMM_B:
                {
                    alu_->add_byte(params[0]);
                }
                break;
            case OPCODE_ADH_REG_B:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu_->add_byte(reg->get_high_byte());
                }
                break;
            case OPCODE_ADL_REG_B:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu_->add_byte(reg->get_low_byte());
                }
                break;
            default:
                throw lvm::runtime_error("Invalid add opcode");
        }
    }

void Cpu::execute_sub_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_SUB_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->sub(value);
            }
            break;
        case OPCODE_SUB_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->sub(reg->get_value());
            }
            break;
        case OPCODE_SBB_IMM_B:
            {
                alu_->sub_byte(params[0]);
            }
            break;
        case OPCODE_SBH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->sub_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_SBL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->sub_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid sub opcode");
    }
}

void Cpu::execute_mul_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_MUL_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->mul(value);
            }
            break;
        case OPCODE_MUL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->mul(reg->get_value());
            }
            break;
        case OPCODE_MLB_IMM_B:
            {
                alu_->mul_byte(params[0]);
            }
            break;
        case OPCODE_MLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->mul_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_MLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->mul_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid mul opcode");
    }
}

void Cpu::execute_div_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_DIV_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->div(value);
            }
            break;
        case OPCODE_DIV_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->div(reg->get_value());
            }
            break;
        case OPCODE_DVB_IMM_B:
            {
                alu_->div_byte(params[0]);
            }
            break;
        case OPCODE_DVH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->div_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_DVL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->div_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid div opcode");
    }
}

void Cpu::execute_rem_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_REM_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->rem(value);
            }
            break;
        case OPCODE_REM_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rem(reg->get_value());
            }
            break;
        case OPCODE_RMB_IMM_B:
            {
                alu_->rem_byte(params[0]);
            }
            break;
        case OPCODE_RMH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rem_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_RML_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rem_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid rem opcode");
    }
}

void Cpu::execute_and_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_AND_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->bit_and(value);
            }
            break;
        case OPCODE_AND_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_and(reg->get_value());
            }
            break;
        case OPCODE_ANB_IMM_B:
            {
                alu_->bit_and_byte(params[0]);
            }
            break;
        case OPCODE_ANH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_and_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_ANL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_and_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid and opcode");
    }
}

void Cpu::execute_or_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_OR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->bit_or(value);
            }
            break;
        case OPCODE_OR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_or(reg->get_value());
            }
            break;
        case OPCODE_ORB_IMM_B:
            {
                alu_->bit_or_byte(params[0]);
            }
            break;
        case OPCODE_ORH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_or_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_ORL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_or_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid or opcode");
    }
}

void Cpu::execute_xor_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_XOR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->bit_xor(value);
            }
            break;
        case OPCODE_XOR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_xor(reg->get_value());
            }
            break;
        case OPCODE_XOB_IMM_B:
            {
                alu_->bit_xor_byte(params[0]);
            }
            break;
        case OPCODE_XOH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_xor_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_XOL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->bit_xor_byte(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid xor opcode");
    }
}

void Cpu::execute_shift_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_SHL_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->shl(value);
            }
            break;
        case OPCODE_SHL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shl(reg->get_value());
            }
            break;
        case OPCODE_SLB_IMM_B:
            {
                alu_->shl(params[0]);
            }
            break;
        case OPCODE_SLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shl(reg->get_high_byte());
            }
            break;
        case OPCODE_SLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shl(reg->get_low_byte());
            }
            break;
        case OPCODE_SHR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->shr(value);
            }
            break;
        case OPCODE_SHR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shr(reg->get_value());
            }
            break;
        case OPCODE_SHRB_IMM_B:
            {
                alu_->shr(params[0]);
            }
            break;
        case OPCODE_SHRH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shr(reg->get_high_byte());
            }
            break;
        case OPCODE_SHRL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->shr(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid shift opcode");
    }
}

void Cpu::execute_rotate_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_ROL_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->rol(value);
            }
            break;
        case OPCODE_ROL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rol(reg->get_value());
            }
            break;
        case OPCODE_ROLB_IMM_B:
            {
                alu_->rol(params[0]);
            }
            break;
        case OPCODE_ROLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rol(reg->get_high_byte());
            }
            break;
        case OPCODE_ROLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->rol(reg->get_low_byte());
            }
            break;
        case OPCODE_ROR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu_->ror(value);
            }
            break;
        case OPCODE_ROR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->ror(reg->get_value());
            }
            break;
        case OPCODE_RORB_IMM_B:
            {
                alu_->ror(params[0]);
            }
            break;
        case OPCODE_RORH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->ror(reg->get_high_byte());
            }
            break;
        case OPCODE_RORL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu_->ror(reg->get_low_byte());
            }
            break;
        default:
            throw lvm::runtime_error("Invalid rotate opcode");
    }
}

// comparisons

void Cpu::execute_cmp_operation(byte_t opcode, const std::vector<byte_t>& params) {
    switch(opcode) {
        case OPCODE_CMP_REG_REG:
            {
                // Compare two registers (full word)
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                AX->set_value(reg1->get_value());
                alu_->cmp(reg2->get_value());
            }
            break;
        case OPCODE_CMP_REG_IMM_W:
            {
                // Compare register with immediate word
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_value());
                alu_->cmp(combine_bytes_to_word(params[1], params[2]));
            }
            break;
        case OPCODE_CPH_REG_REG:
            {
                // Compare high bytes of two registers
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                AX->set_value(reg1->get_high_byte());
                alu_->cmp_byte(reg2->get_high_byte());
            }
            break;
        case OPCODE_CPH_REG_IMM_B:
            {
                // Compare high byte of register with immediate byte
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_high_byte());
                alu_->cmp_byte(params[1]);
            }
            break;
        case OPCODE_CPL_REG_REG:
            {
                // Compare low bytes of two registers
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                AX->set_value(reg1->get_low_byte());
                alu_->cmp_byte(reg2->get_low_byte());
            }
            break;
        case OPCODE_CPL_REG_IMM_B:
            {
                // Compare low byte of register with immediate byte
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_low_byte());
                alu_->cmp_byte(params[1]);
            }
            break;
        default:
            throw lvm::runtime_error("Invalid cmp opcode");
    }
}

