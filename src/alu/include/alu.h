#pragma once
#include "memsize.h"
#include <vector>
#include "register.h"
#include "ialu.h"
#include <memory>
namespace lvm
{
    class Alu : public IALU
    {
        public:
        // Alu requires access to the accumulator register
        // the flags are managed via the register
            Alu(std::shared_ptr<Register> acc);
            Alu(const Alu&);
            ~Alu();
            
            // IALU interface implementation
            void add(word_t value) override;
            void add_byte(byte_t value) override;
            void sub(word_t value) override;
            void sub_byte(byte_t value) override;
            void mul(word_t value) override;
            void mul_byte(byte_t value) override;
            void div(word_t value) override;
            void div_byte(byte_t value) override;
            void rem(word_t value) override;
            void rem_byte(byte_t value) override;
            void bit_and(word_t value) override;
            void bit_and_byte(byte_t value) override;
            void bit_or(word_t value) override;
            void bit_or_byte(byte_t value) override;
            void bit_xor(word_t value) override;
            void bit_xor_byte(byte_t value) override;
            void bit_not() override;
            void shl(word_t count) override;
            void shr(word_t count) override;
            void rol(word_t count) override;
            void ror(word_t count) override;
            void cmp(word_t value) override;
            void cmp_byte(byte_t value) override;
            
        private:
            std::shared_ptr<Register> accumulator;
            void calculate_flags(word_t result, word_t a, word_t b, char operation);
    };
}