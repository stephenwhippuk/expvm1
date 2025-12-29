#pragma once

#include "memsize.h"

namespace lvm {
    // Little-endian: low byte first, high byte second
    addr_t combine_bytes_to_address(byte_t low, byte_t high); 
    word_t combine_bytes_to_word(byte_t low, byte_t high) ;
}