#pragma once
#include "memsize.h"
#include "accessMode.h"
#include "stack_accessor.h"
#include <memory>

namespace lvm {

    // Forward declaration
    class StackAccessor;

    /**
     * IStack - Pure virtual interface for stack implementations
     * 
     * This interface allows other subsystems to interact with the stack
     * without depending on concrete Stack implementation details.
     * Accessors are created ephemerally as needed.
     */
    class IStack {
    public:
        virtual ~IStack() = default;
        
        // Accessor creation (ephemeral)
        virtual std::unique_ptr<StackAccessor> get_accessor(MemAccessMode mode) = 0;
        
        // Stack state queries
        virtual addr32_t get_sp() const = 0;
        virtual int32_t get_fp() const = 0;
        virtual addr32_t get_capacity() const = 0;
    };

} // namespace lvm
