#pragma once
#include "memsize.h"

namespace lvm { 

    enum class Flag {
        ZERO = 0x01,
        CARRY = 0x02,
        SIGN = 0x04,
        OVERFLOW = 0x08
    };

    class Flags {
    public:
        Flags();
        ~Flags();  
        void set(Flag flag);
        void clear(Flag flag);
        bool is_set(Flag flag) const;
        void clear_all();
    private:
        byte_t flags; // Using a byte to store flags as bits
    };
}