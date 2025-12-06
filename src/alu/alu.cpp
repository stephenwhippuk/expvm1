#include "alu.h"
#include "errors.h"

using namespace lvm;

Alu::Alu(std::shared_ptr<Register> acc)
    : accumulator(acc) {
        if(!accumulator->has_flags()) {
            throw lvm::runtime_error("Accumulator register must have associated flags for ALU operations");
        }
    }


Alu::Alu(const Alu& other)
    : accumulator(other.accumulator) {}
Alu::~Alu() {}

void Alu::calculate_flags(word_t result, word_t a, word_t b, char operation) {
    // Clear all relevant flags first
    accumulator->clear_flag(Flag::ZERO);
    accumulator->clear_flag(Flag::CARRY);
    accumulator->clear_flag(Flag::SIGN);
    accumulator->clear_flag(Flag::OVERFLOW);

    // Set ZERO flag
    if (result == 0) {
        accumulator->set_flag(Flag::ZERO);
    }

    // Set SIGN flag
    if (result & 0x8000) { // Check if the highest bit is set
        accumulator->set_flag(Flag::SIGN);
    }

    // Set CARRY and OVERFLOW flags based on operation
    switch (operation) {
        case '+':
            if (result < a || result < b) {
                accumulator->set_flag(Flag::CARRY);
            }
            if (((a ^ result) & (b ^ result) & 0x8000) != 0) {
                accumulator->set_flag(Flag::OVERFLOW);
            }
            break;
        case '-':
            if (a < b) {
                accumulator->set_flag(Flag::CARRY);
            }
            if (((a ^ b) & (a ^ result) & 0x8000) != 0) {
                accumulator->set_flag(Flag::OVERFLOW);
            }
            break;
        // Additional operations can be added here
        default:
            break;
    }

}

void Alu::add(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = 0;
    word_t carry = 0;
    
    // Bitwise addition using XOR and AND
    for (int i = 0; i < 16; i++) {
        word_t bit_a = (a >> i) & 1;
        word_t bit_b = (b >> i) & 1;
        word_t sum_bit = bit_a ^ bit_b ^ carry;
        carry = (bit_a & bit_b) | (bit_a & carry) | (bit_b & carry);
        result |= (sum_bit << i);
    }
    
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
    
}       

void Alu::add_byte(byte_t value) {
    add(static_cast<word_t>(value));
}

void Alu::sub(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = 0;
    word_t borrow = 0;

    // Bitwise subtraction using XOR and AND
    for (int i = 0; i < 16; i++) {
        word_t bit_a = (a >> i) & 1;
        word_t bit_b = (b >> i) & 1;
        word_t diff_bit = bit_a ^ bit_b ^ borrow;
        borrow = (~bit_a & bit_b) | ((~bit_a | bit_b) & borrow);
        result |= (diff_bit << i);
    }

    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '-');
}

void Alu::sub_byte(byte_t value) {
    sub(static_cast<word_t>(value));
}

void Alu::mul(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    dword_t result = static_cast<dword_t>(a) * static_cast<dword_t>(b);
    accumulator->set_value(static_cast<word_t>(result & 0xFFFF));
    // Set flags
    calculate_flags(static_cast<word_t>(result & 0xFFFF), a, b, '+');
    if (result > 0xFFFF) {
        accumulator->set_flag(Flag::CARRY);
    }
}

void Alu::mul_byte(byte_t value) {
    mul(static_cast<word_t>(value));
}

void Alu::div(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    if (b == 0) {
        throw lvm::runtime_error("Division by zero");
    }
    word_t result = a / b;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
}

void Alu::div_byte(byte_t value) {
    div(static_cast<word_t>(value));
}

void Alu::rem(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    if (b == 0) {
        throw lvm::runtime_error("Division by zero");
    }
    word_t result = a % b;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
}

void Alu::rem_byte(byte_t value) {
    rem(static_cast<word_t>(value));
}

void Alu::bit_and(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = a & b;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
}

void Alu::bit_and_byte(byte_t value) {
    bit_and(static_cast<word_t>(value));
}   
void Alu::bit_or(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = a | b;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
}

void Alu::bit_or_byte(byte_t value) {
    bit_or(static_cast<word_t>(value));
}

void Alu::bit_xor(word_t value) {
    word_t a = accumulator->get_value();
    word_t b = value;
    word_t result = a ^ b;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, b, '+');
}

void Alu::bit_xor_byte(byte_t value) {
    bit_xor(static_cast<word_t>(value));
}

void Alu::bit_not() {
    word_t a = accumulator->get_value();
    word_t result = ~a;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, 0, '+');
}
void Alu::shl(word_t count) {
    word_t a = accumulator->get_value();
    word_t result = a << count;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, 0, '+');
}
void Alu::shr(word_t count) {
    word_t a = accumulator->get_value();
    word_t result = a >> count;
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, 0, '+');
}
void Alu::rol(word_t count) {
    word_t a = accumulator->get_value();
    word_t result = (a << count) | (a >> (16 - count));
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, 0, '+');
}
void Alu::ror(word_t count) {
    word_t a = accumulator->get_value();
    word_t result = (a >> count) | (a << (16 - count));
    accumulator->set_value(result);
    // Set flags
    calculate_flags(result, a, 0, '+');
}

void Alu::cmp(word_t value) {
    word_t acc_value = accumulator->get_value();
    word_t result;
    
    if (acc_value < value) {
        result = 0xFFFF; // -1 in two's complement
    } else if (acc_value == value) {
        result = 0x0000; // 0
    } else {
        result = 0x0001; // 1
    }
    
    accumulator->set_value(result);
    calculate_flags(result, acc_value, value, 'c');
}

void Alu::cmp_byte(byte_t value) {
    byte_t acc_value = accumulator->get_low_byte();
    word_t result;
    
    if (acc_value < value) {
        result = 0xFFFF; // -1 in two's complement
    } else if (acc_value == value) {
        result = 0x0000; // 0
    } else {
        result = 0x0001; // 1
    }
    
    accumulator->set_value(result);
    calculate_flags(result, acc_value, value, 'c');
}