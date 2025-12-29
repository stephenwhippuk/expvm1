#include "helpers.h"

namespace lvm {
    // Little-endian: low byte first, high byte second
    addr_t combine_bytes_to_address(byte_t low, byte_t high) {
        return static_cast<addr_t>(static_cast<word_t>(low) | (static_cast<word_t>(high) << 8));
    }

    word_t combine_bytes_to_word(byte_t low, byte_t high) {
        return static_cast<word_t>(static_cast<word_t>(low) | (static_cast<word_t>(high) << 8));
    }
}