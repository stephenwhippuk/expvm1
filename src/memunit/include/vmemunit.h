#ifndef VMEMUNIT_H
#define VMEMUNIT_H

#include "context.h"
#include "vaddr.h"
#include "accessMode.h"
#include "memsize.h"
#include "ivmemunit.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace lvm {
    class PagedMemoryAccessor;  // Forward declaration
}

namespace lvm {

// VMemUnit: Virtual Memory Unit
// - Maps a 40-bit virtual address space
// - Manages allocation/deallocation of physical memory to virtual regions
// - Operates in two modes: PROTECTED and UNPROTECTED
// - Contexts can only be created in UNPROTECTED mode
class VMemUnit : public IVMemUnit {
public:
    VMemUnit();
    ~VMemUnit() = default;
    
    // Delete copy operations
    VMemUnit(const VMemUnit&) = delete;
    VMemUnit& operator=(const VMemUnit&) = delete;
    
    // Allow move operations
    VMemUnit(VMemUnit&&) = default;
    VMemUnit& operator=(VMemUnit&&) = default;
    
    // IVMemUnit interface implementation
    void set_mode(IVMemUnit::Mode mode) override;
    bool is_protected() const override { return mode_ == IVMemUnit::Mode::PROTECTED; }
    context_id_t create_context(uint32_t size) override;
    void destroy_context(context_id_t id) override;
    std::shared_ptr<Context> get_context(context_id_t id) const override;
    std::shared_ptr<Context> find_context_for_address(vaddr_t addr) const override;
    
    // Additional methods not in interface
    IVMemUnit::Mode get_mode() const { return mode_; }
    bool is_unprotected() const { return mode_ == IVMemUnit::Mode::UNPROTECTED; }
    
    // Memory operations (used by FreeStoreAccessor and StackAccessor)
    // These are public so Context can create accessors in PROTECTED mode
    byte_t read_byte(context_id_t context_id, uint32_t address) const;
    void write_byte(context_id_t context_id, addr32_t address, byte_t value);
    
    // Physical memory management - public for StackMemoryAccessor pre-allocation
    void ensure_physical_memory(context_id_t context_id, addr32_t address);
    
    // Block size for memory allocation
    static constexpr size_t BLOCK_SIZE = 4096;

private:
    friend class lvm::PagedMemoryAccessor;
    friend class lvm::StackMemoryAccessor;
    friend class Context;
    IVMemUnit::Mode mode_;
    context_id_t next_context_id_;
    vaddr_t next_free_address_;  // Next available address in virtual space
    std::unordered_map<context_id_t, std::shared_ptr<Context>> contexts_;
    
    // Physical memory management
    // Physical memory blocks: context_id -> (block_index -> data)
    std::unordered_map<context_id_t, std::unordered_map<uint32_t, std::vector<byte_t>>> physical_memory_;
    
    // Allocate a region of virtual address space
    vaddr_t allocate_virtual_space(uint32_t size);
    
    // Get block index for an address
    uint32_t get_block_index(uint32_t address) const { return address / BLOCK_SIZE; }
    uint32_t get_block_offset(uint32_t address) const { return address % BLOCK_SIZE; }
};

}  // namespace lvm

#endif // VMEMUNIT_H
