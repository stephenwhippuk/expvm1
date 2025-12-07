#pragma once
#include "memsize.h"
#include "accessMode.h"
#include <memory>
#include <vector>

namespace lvm {

    // Forward declaration
    class InstructionUnit_Accessor;

    /**
     * IInstructionUnit - Pure virtual interface for instruction unit implementations
     * 
     * This interface allows other subsystems (like CPU) to interact with the
     * instruction unit without depending on concrete implementation details.
     * Accessors are created ephemerally as needed.
     */
    class IInstructionUnit {
    public:
        virtual ~IInstructionUnit() = default;
        
        // Accessor creation (ephemeral)
        virtual std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode) = 0;
    };

} // namespace lvm
