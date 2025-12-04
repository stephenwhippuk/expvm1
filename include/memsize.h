/* This defines a set of size types specific for our code
    this is to allow us control the sizes used in various places
    and to make it easier to change them later if needed.

*/

#pragma once

#include <cstdint>

namespace lvm {

    using addr_t = uint16_t;      // Type for memory addresses
    using page_offset_t = int16_t; // Type for offsets within a page may be positive or negative
    using page_t = uint16_t;      // Type for page numbers
    using offset_t = uint16_t;    // Type for offsets within a page
    using memsize_t = uint16_t;   // Type for sizes of memory blocks

    typedef uint8_t byte_t;        // Type for a byte of memory
    typedef uint16_t word_t;       // Type for a word (2 bytes)
    typedef uint32_t dword_t;      // Type for a double word (4 bytes)

} // namespace lvm