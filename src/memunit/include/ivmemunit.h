#pragma once
#include "memsize.h"
#include "vaddr.h"
#include <memory>

namespace lvm {

    // Forward declarations
    class Context;
    class PagedMemoryAccessor;
    class StackMemoryAccessor;

    /**
     * IVMemUnit - Pure virtual interface for virtual memory unit implementations
     * 
     * This interface allows subsystems to interact with virtual memory
     * without depending on concrete VMemUnit implementation details.
     * Provides context management and mode switching capabilities.
     */
    class IVMemUnit {
    public:
        virtual ~IVMemUnit() = default;

        enum class Mode {
            PROTECTED,
            UNPROTECTED
        };

        // Mode management
        virtual void set_mode(Mode mode) = 0;
        virtual bool is_protected() const = 0;

        // Context management
        virtual context_id_t create_context(addr32_t size) = 0;
        virtual void destroy_context(context_id_t id) = 0;
        virtual std::shared_ptr<Context> get_context(context_id_t id) const = 0;
        virtual std::shared_ptr<Context> find_context_for_address(vaddr_t address) const = 0;
    };

} // namespace lvm
