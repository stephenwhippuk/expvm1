#include "helpers.h"

namespace lvm {
    addr_t combine_bytes_to_address(byte_t high, byte_t low) {
        return static_cast<addr_t>((static_cast<word_t>(high) << 8) | static_cast<word_t>(low));
    }

    word_t combine_bytes_to_word(byte_t high, byte_t low) {
        return static_cast<word_t>((static_cast<word_t>(high) << 8) | static_cast<word_t>(low));
    }
}