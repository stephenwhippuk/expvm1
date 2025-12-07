# VMemUnit - Virtual Memory Unit

## Purpose

The Virtual Memory Unit (VMemUnit) provides the foundation for all memory operations in the Pendragon VM. It manages isolated memory contexts, enforces protection modes, and serves as the central authority for memory allocation and access control.

## Interface Design

### IVMemUnit Interface

VMemUnit implements the `IVMemUnit` pure virtual interface, enabling dependency injection and loose coupling:

```cpp
class IVMemUnit {
public:
    enum class Mode { UNPROTECTED, PROTECTED };
    
    virtual void set_mode(Mode mode) = 0;
    virtual bool is_protected() const = 0;
    
    virtual context_id_t create_context(addr32_t capacity) = 0;
    virtual void destroy_context(context_id_t id) = 0;
    
    virtual std::shared_ptr<Context> get_context(context_id_t id) const = 0;
    virtual std::shared_ptr<Context> find_context_for_address(vaddr_t address) const = 0;
    
    virtual ~IVMemUnit() = default;
};
```

**Key Points:**
- Mode enum is defined at interface level (`IVMemUnit::Mode`)
- All public methods are pure virtual
- Enables testing with mock implementations
- CPU depends only on this interface, not concrete VMemUnit

### Context Class

The `Context` class is in the `lvm` namespace and enforces accessor-only access:

```cpp
namespace lvm {
    class Context {
        // All getters are private
        // Access only through PagedMemoryAccessor in PROTECTED mode
        friend class VMemUnit;
        friend class PagedMemoryAccessor;
    };
}
```

**Access Pattern:**
- No public getters for Context internals
- All access must go through `PagedMemoryAccessor`
- Accessors can only be created in PROTECTED mode
- This enforces proper encapsulation and security

## Architecture Overview

### Core Concepts

- **Contexts**: Isolated memory spaces with configurable capacity
- **Protection Modes**: UNPROTECTED (setup/teardown) and PROTECTED (runtime execution)
- **40-bit Virtual Address Space**: Supports very large memory configurations
- **Access Control**: Friend-based accessor creation pattern for security

### Memory Model

```
VMemUnit (implements IVMemUnit)
├── Mode (IVMemUnit::Mode: UNPROTECTED/PROTECTED)
└── Contexts (multiple isolated spaces)
    ├── Context 0: Stack (1KB typical)
    ├── Context 1: Code Space (64KB typical)
    ├── Context 2: Data Space (64KB typical)
    └── Context N: Additional spaces as needed
```

### Protection Mode State Machine

```
UNPROTECTED → can create/destroy contexts, cannot create accessors
     ↓
PROTECTED → can create accessors, cannot create/destroy contexts
```

## Interface

### VMemUnit Public API (implements IVMemUnit)

```cpp
class VMemUnit : public IVMemUnit {
public:
    // Mode management
    void set_mode(IVMemUnit::Mode mode) override;
    bool is_protected() const override;

    // Context lifecycle (UNPROTECTED mode only)
    context_id_t create_context(addr32_t capacity) override;
    void destroy_context(context_id_t id) override;
    
    // Context access (any mode)
    std::shared_ptr<Context> get_context(context_id_t id) const override;
    std::shared_ptr<Context> find_context_for_address(vaddr_t address) const override;
};
```

### Context Class

```cpp
class Context {
    // Accessor creation (PROTECTED mode only)
    std::unique_ptr<PagedMemoryAccessor> create_paged_accessor(
        VMemUnit& vmem_unit, 
        MemAccessMode mode
    ) const;
    
    std::unique_ptr<StackMemoryAccessor> create_stack_accessor(
        VMemUnit& vmem_unit
    ) const;
    
    // Address validation
    bool contains(vaddr_t address) const;
    
    // Properties
    vaddr_t get_start() const;
    vaddr_t get_end() const;
    addr32_t get_capacity() const;
};
```

## Usage Examples

### Example 1: Basic Setup and Access

```cpp
#include "vmemunit.h"

// Create the virtual memory unit
using namespace lvm;
VMemUnit vmem_unit;

// Create contexts in UNPROTECTED mode (default)
context_id_t stack_ctx = vmem_unit.create_context(1024);    // 1KB stack
context_id_t code_ctx = vmem_unit.create_context(65536);    // 64KB code
context_id_t data_ctx = vmem_unit.create_context(65536);    // 64KB data

// Switch to PROTECTED mode for runtime
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);

// Create accessors for memory operations
auto code_context = vmem_unit.get_context(code_ctx);
auto code_accessor = code_context->create_paged_accessor(
    vmem_unit, 
    MemAccessMode::READ_WRITE
);

// Write program to code space
code_accessor->set_page(0);
code_accessor->write_byte(0, 0x01);  // HALT instruction

// Read back
byte_t opcode = code_accessor->read_byte(0);
```

### Example 2: Stack Context with Direct Addressing

```cpp
#include "vmemunit.h"
#include "stack.h"

using namespace lvm;
VMemUnit vmem_unit;

// Create stack in UNPROTECTED mode
Stack stack(vmem_unit, 1024);

// Switch to PROTECTED for operations
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);

// Get stack accessor
auto stack_accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Use the stack
stack_accessor->push_word(0x1234);
word_t value = stack_accessor->pop_word();
```

### Example 3: Multi-Context Data Access

```cpp
// Setup phase
using namespace lvm;
VMemUnit vmem_unit;
context_id_t ctx1 = vmem_unit.create_context(4096);  // 4KB
context_id_t ctx2 = vmem_unit.create_context(4096);  // 4KB
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);

// Access first context
const Context* c1 = vmem_unit.get_context(ctx1);
auto accessor1 = c1->create_paged_accessor(MemAccessMode::READ_WRITE);
accessor1->set_page(0);
accessor1->write_word(0, 0xABCD);

// Access second context (isolated from first)
const Context* c2 = vmem_unit.get_context(ctx2);
auto accessor2 = c2->create_paged_accessor(MemAccessMode::READ_WRITE);
accessor2->set_page(0);
accessor2->write_word(0, 0x1234);  // Separate memory space

// Contexts are isolated - changes don't affect each other
assert(accessor1->read_word(0) == 0xABCD);
assert(accessor2->read_word(0) == 0x1234);
```

### Example 4: Safe Teardown

```cpp
using namespace lvm;
VMemUnit vmem_unit;
context_id_t ctx = vmem_unit.create_context(1024);

vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);
// ... do work ...
vmem_unit.set_mode(IVMemUnit::Mode::UNPROTECTED);

// Clean up
vmem_unit.destroy_context(ctx);
```

## Key Design Patterns

### Friend-Based Access Control

VMemUnit uses C++ friend relationships to enforce that only specific classes can create accessors:
- `PagedMemoryAccessor` - for paged memory access (256-byte pages)
- `StackMemoryAccessor` - for direct 32-bit stack addressing with pre-allocation

This prevents unauthorized memory access while maintaining performance.

### Ephemeral Accessors

Accessors are typically created on-demand and destroyed after use:

```cpp
{
    auto accessor = context->create_paged_accessor(mode);
    accessor->write_byte(0, value);
    // accessor destroyed here
}
```

This pattern ensures proper resource management and prevents dangling references.

## Error Handling

Common exceptions:
- `runtime_error("Cannot create context in PROTECTED mode")` - Attempted context creation while protected
- `runtime_error("Cannot destroy context in PROTECTED mode")` - Attempted context destruction while protected
- `runtime_error("Context not found")` - Invalid context ID
- `runtime_error("Accessor can only be created in PROTECTED mode")` - Attempted accessor creation while unprotected

## Performance Considerations

- **Context Creation**: O(1) - Fast allocation from virtual address space
- **Context Lookup**: O(n) where n is number of contexts (typically small)
- **Mode Switching**: O(1) - Simple state flag change
- **Accessor Creation**: O(1) - No allocation, returns unique_ptr with private constructor

## Thread Safety

VMemUnit is **not thread-safe**. The VM is designed as a single-threaded execution environment. If multi-threading is required, external synchronization must be provided.
