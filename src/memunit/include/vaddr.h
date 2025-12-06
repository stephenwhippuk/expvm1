#ifndef VADDR_H
#define VADDR_H

#include <cstdint>

// Virtual address type: 40-bit addressing (only lower 40 bits used)
using vaddr_t = uint64_t;

// Context ID type (16-bit allows up to 65536 contexts)
using context_id_t = uint16_t;

// Mask for valid 40-bit addresses
constexpr vaddr_t VADDR_MASK = 0xFFFFFFFFFFULL;

// Maximum size for a single context (32-bit addressable space = 4GB)
constexpr uint32_t MAX_CONTEXT_SIZE = 0xFFFFFFFFU;

// Validate that a virtual address is within 40-bit range
inline bool is_valid_vaddr(vaddr_t addr) {
    return (addr & ~VADDR_MASK) == 0;
}

#endif // VADDR_H
