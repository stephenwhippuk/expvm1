#pragma once
#include "memsize.h"

namespace lvm {

    /**
     * IALU - Pure virtual interface for Arithmetic Logic Unit implementations
     * 
     * This interface allows subsystems to perform arithmetic and logical
     * operations without depending on concrete ALU implementation details.
     * Operations work on an accumulator register managed by the ALU.
     */
    class IALU {
    public:
        virtual ~IALU() = default;

        // Arithmetic operations (16-bit)
        virtual void add(word_t value) = 0;
        virtual void sub(word_t value) = 0;
        virtual void mul(word_t value) = 0;
        virtual void div(word_t value) = 0;
        virtual void rem(word_t value) = 0;

        // Arithmetic operations (8-bit)
        virtual void add_byte(byte_t value) = 0;
        virtual void sub_byte(byte_t value) = 0;
        virtual void mul_byte(byte_t value) = 0;
        virtual void div_byte(byte_t value) = 0;
        virtual void rem_byte(byte_t value) = 0;

        // Logical operations (16-bit)
        virtual void bit_and(word_t value) = 0;
        virtual void bit_or(word_t value) = 0;
        virtual void bit_xor(word_t value) = 0;
        virtual void bit_not() = 0;

        // Logical operations (8-bit)
        virtual void bit_and_byte(byte_t value) = 0;
        virtual void bit_or_byte(byte_t value) = 0;
        virtual void bit_xor_byte(byte_t value) = 0;

        // Shift and rotate (16-bit only)
        virtual void shl(word_t count) = 0;
        virtual void shr(word_t count) = 0;
        virtual void rol(word_t count) = 0;
        virtual void ror(word_t count) = 0;

        // Comparison (sets flags, doesn't modify accumulator)
        virtual void cmp(word_t value) = 0;
        virtual void cmp_byte(byte_t value) = 0;
    };

} // namespace lvm
