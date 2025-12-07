#pragma once
#include <memory>
namespace lvm {

    class BasicIOAccessor;

    /**
     * IBasicIO - Pure virtual interface for basic I/O operations
     * 
     * Provides abstraction for basic input/output functionality.
     */
    class IBasicIO {
    public:
        virtual ~IBasicIO() = default;
        
        // TODO: Add interface methods
        virtual std::unique_ptr<BasicIOAccessor> get_accessor() = 0;
    };

} // namespace lvm
