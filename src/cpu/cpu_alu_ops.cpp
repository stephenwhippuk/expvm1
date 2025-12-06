#include "opcodes.h"
#include "cpu.h"
#include "helpers.h"
using namespace lvm;

void Cpu::execute_add_operation(byte_t opcode, const std::vector<byte_t>& params) {
        switch(opcode) {
            case OPCODE_ADD_REG_W:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu->add(reg->get_value());
                }
                break;
            case OPCODE_ADD_IMM_W:
                {
                    word_t value = combine_bytes_to_word(params[0], params[1]);
                    alu->add(value);
                }
                break;
            case OPCODE_ADB_IMM_B:
                {
                    alu->add_byte(params[0]);
                }
                break;
            case OPCODE_ADH_REG_B:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu->add_byte(reg->get_high_byte());
                }
                break;
            case OPCODE_ADL_REG_B:
                {
                    auto reg = get_register_by_code(params[0]);
                    alu->add_byte(reg->get_low_byte());
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
                alu->sub(value);
            }
            break;
        case OPCODE_SUB_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->sub(reg->get_value());
            }
            break;
        case OPCODE_SBB_IMM_B:
            {
                alu->sub_byte(params[0]);
            }
            break;
        case OPCODE_SBH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->sub_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_SBL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->sub_byte(reg->get_low_byte());
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
                alu->mul(value);
            }
            break;
        case OPCODE_MUL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->mul(reg->get_value());
            }
            break;
        case OPCODE_MLB_IMM_B:
            {
                alu->mul_byte(params[0]);
            }
            break;
        case OPCODE_MLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->mul_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_MLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->mul_byte(reg->get_low_byte());
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
                alu->div(value);
            }
            break;
        case OPCODE_DIV_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->div(reg->get_value());
            }
            break;
        case OPCODE_DVB_IMM_B:
            {
                alu->div_byte(params[0]);
            }
            break;
        case OPCODE_DVH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->div_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_DVL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->div_byte(reg->get_low_byte());
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
                alu->rem(value);
            }
            break;
        case OPCODE_REM_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rem(reg->get_value());
            }
            break;
        case OPCODE_RMB_IMM_B:
            {
                alu->rem_byte(params[0]);
            }
            break;
        case OPCODE_RMH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rem_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_RML_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rem_byte(reg->get_low_byte());
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
                alu->bit_and(value);
            }
            break;
        case OPCODE_AND_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_and(reg->get_value());
            }
            break;
        case OPCODE_ANB_IMM_B:
            {
                alu->bit_and_byte(params[0]);
            }
            break;
        case OPCODE_ANH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_and_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_ANL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_and_byte(reg->get_low_byte());
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
                alu->bit_or(value);
            }
            break;
        case OPCODE_OR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_or(reg->get_value());
            }
            break;
        case OPCODE_ORB_IMM_B:
            {
                alu->bit_or_byte(params[0]);
            }
            break;
        case OPCODE_ORH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_or_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_ORL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_or_byte(reg->get_low_byte());
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
                alu->bit_xor(value);
            }
            break;
        case OPCODE_XOR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_xor(reg->get_value());
            }
            break;
        case OPCODE_XOB_IMM_B:
            {
                alu->bit_xor_byte(params[0]);
            }
            break;
        case OPCODE_XOH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_xor_byte(reg->get_high_byte());
            }
            break;
        case OPCODE_XOL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->bit_xor_byte(reg->get_low_byte());
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
                alu->shl(value);
            }
            break;
        case OPCODE_SHL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shl(reg->get_value());
            }
            break;
        case OPCODE_SLB_IMM_B:
            {
                alu->shl(params[0]);
            }
            break;
        case OPCODE_SLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shl(reg->get_high_byte());
            }
            break;
        case OPCODE_SLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shl(reg->get_low_byte());
            }
            break;
        case OPCODE_SHR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu->shr(value);
            }
            break;
        case OPCODE_SHR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shr(reg->get_value());
            }
            break;
        case OPCODE_SHRB_IMM_B:
            {
                alu->shr(params[0]);
            }
            break;
        case OPCODE_SHRH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shr(reg->get_high_byte());
            }
            break;
        case OPCODE_SHRL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->shr(reg->get_low_byte());
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
                alu->rol(value);
            }
            break;
        case OPCODE_ROL_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rol(reg->get_value());
            }
            break;
        case OPCODE_ROLB_IMM_B:
            {
                alu->rol(params[0]);
            }
            break;
        case OPCODE_ROLH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rol(reg->get_high_byte());
            }
            break;
        case OPCODE_ROLL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->rol(reg->get_low_byte());
            }
            break;
        case OPCODE_ROR_IMM_W:
            {
                word_t value = combine_bytes_to_word(params[0], params[1]);
                alu->ror(value);
            }
            break;
        case OPCODE_ROR_REG_W:
            {
                auto reg = get_register_by_code(params[0]);
                alu->ror(reg->get_value());
            }
            break;
        case OPCODE_RORB_IMM_B:
            {
                alu->ror(params[0]);
            }
            break;
        case OPCODE_RORH_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->ror(reg->get_high_byte());
            }
            break;
        case OPCODE_RORL_REG_B:
            {
                auto reg = get_register_by_code(params[0]);
                alu->ror(reg->get_low_byte());
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
                alu->cmp(reg2->get_value());
            }
            break;
        case OPCODE_CMP_REG_IMM_W:
            {
                // Compare register with immediate word
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_value());
                alu->cmp(combine_bytes_to_word(params[1], params[2]));
            }
            break;
        case OPCODE_CPH_REG_REG:
            {
                // Compare high bytes of two registers
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                AX->set_value(reg1->get_high_byte());
                alu->cmp_byte(reg2->get_high_byte());
            }
            break;
        case OPCODE_CPH_REG_IMM_B:
            {
                // Compare high byte of register with immediate byte
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_high_byte());
                alu->cmp_byte(params[1]);
            }
            break;
        case OPCODE_CPL_REG_REG:
            {
                // Compare low bytes of two registers
                auto reg1 = get_register_by_code(params[0]);
                auto reg2 = get_register_by_code(params[1]);
                AX->set_value(reg1->get_low_byte());
                alu->cmp_byte(reg2->get_low_byte());
            }
            break;
        case OPCODE_CPL_REG_IMM_B:
            {
                // Compare low byte of register with immediate byte
                auto reg = get_register_by_code(params[0]);
                AX->set_value(reg->get_low_byte());
                alu->cmp_byte(params[1]);
            }
            break;
        default:
            throw lvm::runtime_error("Invalid cmp opcode");
    }
}

