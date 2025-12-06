#pragma once

#include "memsize.h"

namespace lvm {
    addr_t combine_bytes_to_address(byte_t high, byte_t low); 
    word_t combine_bytes_to_word(byte_t high, byte_t low) ;
}