#pragma once

#include "ibasic_io.h"
#include <memory>
#include "ivmemunit.h"
#include "stack.h"

namespace lvm {

    class BasicIOAccessor;
    /**
     * BasicIO - Basic I/O implementation
     * 
     * Implements basic input/output operations for the virtual machine.
     */
    class BasicIO : public IBasicIO {
    public:
        BasicIO(std::shared_ptr<IVMemUnit> memUnit, std::shared_ptr<IStack> stack);
        ~BasicIO() override;
        
        // Delete copy operations
        BasicIO(const BasicIO&) = delete;
        BasicIO& operator=(const BasicIO&) = delete;
        
        // Allow move operations
        BasicIO(BasicIO&&) noexcept = default;
        BasicIO& operator=(BasicIO&&) noexcept = default;
        
        // TODO: Add implementation methods
        std::unique_ptr<BasicIOAccessor> get_accessor() override;
    private:
        friend class BasicIOAccessor;
        
        // TODO: Add private members
        std::shared_ptr<IVMemUnit> memUnit;
        std::shared_ptr<IStack> stack;

        void write_string_from_stack();
        void write_line_from_stack();
        void read_line_onto_stack();
        
    };

} // namespace lvm
