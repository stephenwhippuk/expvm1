#pragma once
#include "memsize.h"
#include <vector>
#include "register.h"
#include <memory>
namespace lvm
{
    class Alu
    {
        public:
        // Alu requires access to the accumulator register
        // the flags are managed via the register
            Alu(std::shared_ptr<Register> acc);
            Alu(const Alu&);
            ~Alu();
            
            // ALU operation methods - operate on accumulator with provided value
            void add(word_t value);
            void add_byte(byte_t value);
            void sub(word_t value);
            void sub_byte(byte_t value);
            void mul(word_t value);
            void mul_byte(byte_t value);
            void div(word_t value);
            void div_byte(byte_t value);
            void rem(word_t value);
            void rem_byte(byte_t value);
            void bit_and(word_t value);
            void bit_and_byte(byte_t value);
            void bit_or(word_t value);
            void bit_or_byte(byte_t value);
            void bit_xor(word_t value);
            void bit_xor_byte(byte_t value);
            void bit_not();
            void shl(word_t count);
            void shr(word_t count);
            void rol(word_t count);
            void ror(word_t count);
            void cmp(word_t value);
            void cmp_byte(byte_t value);
            
        private:
            std::shared_ptr<Register> accumulator;
            void calculate_flags(word_t result, word_t a, word_t b, char operation);
    };
}