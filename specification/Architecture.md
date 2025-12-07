# Pendragon Virtual Machine - Architecture & Solution Structure

**Version:** 1.0  
**Project:** Pendragon (LVM - Language Virtual Machine)  
**Language:** C++20  
**Build System:** CMake 3.20+  
**Testing Framework:** Google Test 1.14.0

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architectural Principles](#architectural-principles)
3. [Core Subsystems](#core-subsystems)
4. [Solution Structure](#solution-structure)
5. [Memory Architecture](#memory-architecture)
6. [Execution Model](#execution-model)
7. [Interface Contracts](#interface-contracts)
8. [Build and Test Infrastructure](#build-and-test-infrastructure)
9. [Dependencies and Data Flow](#dependencies-and-data-flow)

---

## System Overview

### Purpose

Pendragon is a **16-bit register-based virtual machine** designed as a flexible, extensible platform for language implementation research and systems programming experimentation. It serves as the foundation of the Arthurian VM family, providing a sandboxed environment with well-defined interfaces and modular architecture.

### Key Characteristics

- **16-bit Architecture**: All registers and primary data paths operate on 16-bit words
- **Register-Based Execution**: 5 general-purpose registers (AX-EX) with dedicated accumulator
- **Protected Memory Model**: Page-based virtual memory with context isolation
- **Stack-Based Calling Convention**: Separate call stack and return stack for subroutines
- **System Call Interface**: Extensible I/O through system call mechanism
- **Little-Endian**: Multi-byte values stored with LSB first

### Version 1.0 Feature Set

**Instruction Set:**
- 119 opcodes (0x00-0x76, 0x7F)
- Memory operations (load, store, addressing modes)
- Arithmetic and logical operations (add, subtract, multiply, divide, bitwise)
- Control flow (jumps, conditional branches, subroutines)
- Stack operations (push, pop, peek, frame management)
- System calls (I/O operations)

**Memory Management:**
- Paged memory (256-byte pages)
- Multiple isolated contexts
- Protected/unprotected mode discipline
- Context-based address translation

**I/O System:**
- Console input/output
- Stack-based parameter passing
- 3 system calls (print string, print line, read line)

---

## Architectural Principles

### 1. Interface-Based Design

All major subsystems communicate through **pure virtual interfaces**, enabling:
- **Loose coupling**: Components depend on contracts, not implementations
- **Testability**: Mock implementations for unit testing
- **Flexibility**: Swap implementations without breaking contracts
- **Clear boundaries**: Explicit dependencies and data flow

**Example Pattern:**
```
IVMemUnit (interface) ← implemented by → VMemUnit (concrete)
     ↑
     └─ CPU depends on IVMemUnit, not VMemUnit
```

### 2. Unit → Handler/Accessor Pattern

All components follow a **protection pattern** where direct access is controlled through ephemeral accessor objects:

```
Unit (private state)
  ↓
  get_accessor(mode) → Accessor (ephemeral, mode-aware)
  ↓
Client (uses accessor, cannot access unit directly)
```

**Benefits:**
- **Access control**: Read-only vs read-write modes
- **Mode enforcement**: Protected vs unprotected operations
- **Encapsulation**: Internal state hidden from clients
- **Contract protection**: Interface changes don't break clients

### 3. Dependency Injection

The VM uses **constructor and setter injection** to wire subsystems:

```cpp
// Subsystems created independently
auto vmem_unit = std::make_shared<VMemUnit>();
auto stack = std::make_shared<Stack>(vmem_unit, capacity);
auto basic_io = std::make_shared<BasicIO>(vmem_unit, stack);

// Dependencies injected into CPU
Cpu cpu(vmem_unit);
cpu.set_stack(stack);
cpu.set_instruction_unit(instruction_unit);
```

This enables:
- Controlled initialization order
- Clear dependency graph
- Easy testing with mock objects
- Runtime subsystem replacement

### 4. Mode-Based Protection

The memory system enforces **PROTECTED vs UNPROTECTED mode**:

- **UNPROTECTED**: Context creation, unit construction, system setup
- **PROTECTED**: Normal execution, accessor creation, runtime operations

Mode violations throw exceptions, ensuring proper initialization discipline.

### 5. Memory Safety

All raw pointers are avoided in favor of:
- `std::shared_ptr<T>` for shared ownership (VMemUnit, Stack, etc.)
- `std::unique_ptr<T>` for exclusive ownership (Accessors, ALU)
- References (`T&`) for non-owning access (interfaces)

No manual `new`/`delete`, preventing memory leaks and double-frees.

---

## Core Subsystems

### 1. Virtual Memory Unit (VMemUnit)

**Location:** `src/memunit/`  
**Interface:** `IVMemUnit`  
**Purpose:** Page-based virtual memory management with context isolation

**Responsibilities:**
- Context lifecycle management (create, destroy, get)
- Page allocation and deallocation
- Address validation and translation
- Mode enforcement (PROTECTED/UNPROTECTED)

**Key Concepts:**
- **Context**: Isolated memory region with unique ID
- **Page**: 256-byte memory block
- **Virtual Address**: Page number + offset within page

**Accessors:**
- `PagedMemoryAccessor`: Access memory through page-based addressing
- `StackMemoryAccessor`: Contiguous pre-allocated stack access

### 2. Stack

**Location:** `src/stack/`  
**Interface:** `IStack`  
**Purpose:** Upward-growing stack with frame pointer support

**Responsibilities:**
- Push/pop bytes and words (little-endian)
- Stack frame management (base, frame pointer, top)
- Peek operations (relative to base or frame)
- Subroutine calling convention support

**Key Operations:**
- `push_byte/push_word`: Add data to top of stack
- `pop_byte/pop_word`: Remove data from top (respects frame boundary)
- `peek_word_from_base/frame`: Read without popping
- `set_frame_pointer`: Mark subroutine frame boundary
- `flush`: Reset stack to frame pointer

**Memory Model:**
```
Stack grows upward from address 0:
[base=0] → [frame pointer] → [top]
```

### 3. Instruction Unit

**Location:** `src/instruction_unit/`  
**Interface:** `IInstructionUnit`  
**Purpose:** Program counter and control flow management

**Responsibilities:**
- Instruction register (IR) management
- Program loading into code context
- Jump and branch operations
- Subroutine call/return mechanism
- System call dispatching

**Key Components:**
- **Instruction Register (IR)**: 16-bit program counter
- **Return Stack**: Stores return addresses and frame pointers
- **Code Context**: Read-only program memory

**Subroutine Support:**
```cpp
struct ReturnStackItem {
    addr_t return_address;
    int32_t frame_pointer;  // -1 = no frame
};
```

### 4. CPU (Central Processing Unit)

**Location:** `src/cpu/`  
**Purpose:** Instruction fetch-decode-execute engine

**Responsibilities:**
- Opcode dispatch to specialized handlers
- Register file management (AX-EX)
- Program execution lifecycle
- Integration of all subsystems

**Components:**
- **Registers**: 5 general-purpose 16-bit registers
- **Flags**: Zero, Sign, Carry, Overflow
- **ALU**: Arithmetic and logical operations
- **Opcode Handlers**: Specialized methods for each operation category

**Execution Cycle:**
```
1. Fetch opcode at IR
2. Advance IR past opcode
3. Fetch parameters (0-4 bytes)
4. Dispatch to handler:
   - execute_memory_operation()
   - execute_jump()
   - execute_add_operation()
   - execute_subroutine_operation()
   - etc.
5. Handler updates registers/flags
6. Repeat until HALT
```

### 5. ALU (Arithmetic Logic Unit)

**Location:** `src/alu/`  
**Purpose:** Arithmetic and logical operations with flag management

**Responsibilities:**
- Binary operations (add, subtract, multiply, divide)
- Bitwise operations (AND, OR, XOR, NOT)
- Shift and rotate operations
- Flag computation (zero, sign, carry, overflow)
- Comparison operations

**Integration:**
- Tightly coupled to AX register
- Direct flag register access
- Created internally by CPU (not injected)

### 6. Basic I/O

**Location:** `src/basic_io/`  
**Interface:** `IBasicIO`  
**Purpose:** Console input/output via system calls

**Responsibilities:**
- Stack-based I/O parameter handling
- String output (with/without newline)
- Line input with length limiting
- System call implementation

**System Calls:**
- `PRINT_STRING_FROM_STACK (0x0010)`: Output string
- `PRINT_LINE_FROM_STACK (0x0011)`: Output string + newline
- `READ_LINE_ONTO_STACK (0x0012)`: Read input line

**Architecture:**
```
CPU → InstructionUnit.system_call()
       ↓
       BasicIO.get_accessor()
       ↓
       BasicIOAccessor.write_string_from_stack()
       ↓
       Stack (read parameters)
       ↓
       std::cout (output)
```

### 7. VM (Virtual Machine)

**Location:** `src/vm/`  
**Purpose:** High-level API and subsystem orchestration

**Responsibilities:**
- Subsystem creation and initialization
- Dependency injection coordination
- Program loading convenience methods
- Execution management

**Usage:**
```cpp
VM vm(4096, 8192);  // 4KB stack, 8KB code
vm.load_program(program);
vm.run();
```

### 8. Support Libraries

**Helpers** (`src/helpers/`):
- Error handling types
- Utility functions
- Common definitions

**Register** (`src/register/`):
- 16-bit register implementation
- High/low byte access
- Flag integration

---

## Solution Structure

```
expvm1/
├── CMakeLists.txt                    # Root build configuration
├── specification/                    # Documentation
│   ├── Architecture.md              # This document
│   ├── PendragonVirtualMachine.md   # High-level overview
│   ├── PendragonOperations.md       # Complete instruction set
│   ├── SystemCalls.md               # System call reference
│   └── Reference/                   # Component documentation
│       ├── ALU.md
│       ├── CPU.md
│       ├── Helpers.md
│       ├── InstructionUnit.md
│       ├── Register.md
│       ├── Stack.md
│       └── VMemUnit.md
│
├── src/                             # Source code
│   ├── main.cpp                     # Executable entry point
│   │
│   ├── memunit/                     # Virtual memory subsystem
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── ivmemunit.h          # Interface
│   │   │   ├── vmemunit.h           # Implementation
│   │   │   ├── context.h
│   │   │   ├── paged_memory_accessor.h
│   │   │   └── stack_memory_accessor.h
│   │   ├── vmemunit.cpp
│   │   ├── context.cpp
│   │   ├── paged_memory_accessor.cpp
│   │   ├── stack_memory_accessor.cpp
│   │   └── tests/
│   │       └── memunit_tests.cpp
│   │
│   ├── stack/                       # Stack subsystem
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── istack.h             # Interface
│   │   │   ├── stack.h              # Implementation
│   │   │   └── stack_accessor.h
│   │   ├── stack.cpp
│   │   ├── stack_accessor.cpp
│   │   └── tests/
│   │       ├── stack_tests.cpp
│   │       └── stack_subroutine_tests.cpp
│   │
│   ├── instruction_unit/            # Instruction unit subsystem
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── iinstruction_unit.h  # Interface
│   │   │   ├── instruction_unit.h   # Implementation
│   │   │   ├── opcodes.h            # Opcode definitions
│   │   │   └── systemcalls.h        # System call IDs
│   │   ├── instruction_unit.cpp
│   │   └── tests/
│   │       └── instruction_unit_tests.cpp
│   │
│   ├── basic_io/                    # I/O subsystem
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── ibasic_io.h          # Interface
│   │   │   ├── basic_io.h           # Implementation
│   │   │   └── basic_io_accessor.h
│   │   ├── basic_io.cpp
│   │   └── tests/
│   │       └── basic_io_tests.cpp
│   │
│   ├── cpu/                         # CPU subsystem
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   └── cpu.h
│   │   ├── cpu.cpp                  # Main execution logic
│   │   └── cpu_alu_ops.cpp          # ALU operation handlers
│   │
│   ├── alu/                         # Arithmetic logic unit
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   └── alu.h
│   │   ├── alu.cpp
│   │   └── tests/
│   │       └── alu_tests.cpp
│   │
│   ├── register/                    # Register implementation
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── register.h
│   │   │   └── flags.h
│   │   ├── register.cpp
│   │   ├── flags.cpp
│   │   └── tests/
│   │       └── register_tests.cpp
│   │
│   ├── helpers/                     # Common utilities
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   ├── errors.h
│   │   │   ├── memsize.h            # Type definitions
│   │   │   ├── accessMode.h
│   │   │   └── vaddr.h              # Virtual address
│   │   ├── errors.cpp
│   │   └── vaddr.cpp
│   │
│   └── vm/                          # High-level VM API
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── vm.h
│       └── vm.cpp
│
├── include/                         # Legacy includes (deprecated)
│   ├── memory.h
│   ├── memsize.h
│   └── stack.h
│
└── build/                           # CMake build output
    ├── compile_commands.json        # For IDE integration
    └── (generated files)
```

---

## Memory Architecture

### Address Space Organization

```
┌─────────────────────────────────────────┐
│         Virtual Memory Space             │
├─────────────────────────────────────────┤
│                                          │
│  Context 0: Code Memory (8192 bytes)    │
│  ┌────────────────────────────────────┐ │
│  │ Page 0  │ Page 1  │ ... │ Page 31 │ │
│  │ [0-255] │[256-511]│     │[8191]   │ │
│  └────────────────────────────────────┘ │
│                                          │
│  Context 1: Data Memory (65536 bytes)   │
│  ┌────────────────────────────────────┐ │
│  │ Page 0  │ Page 1  │ ... │ Page255│ │
│  └────────────────────────────────────┘ │
│                                          │
│  Context 2: Stack (4096 bytes)          │
│  ┌────────────────────────────────────┐ │
│  │ Pre-allocated contiguous memory     │ │
│  │ [Base=0] ───→ [Frame] ───→ [Top]   │ │
│  └────────────────────────────────────┘ │
│                                          │
└─────────────────────────────────────────┘
```

### Context Isolation

Each context is **completely isolated**:
- Separate page allocations
- Independent address spaces
- No cross-context access
- Mode-protected boundaries

### Page Structure

```
Page (256 bytes):
┌──────────────────────────────────┐
│ Byte 0   │ Byte 1   │ ... │255  │
├──────────┼──────────┼─────┼─────┤
│ data[0]  │ data[1]  │ ... │data │
└──────────┴──────────┴─────┴─────┘

Virtual Address = (Page Number × 256) + Offset
Example: Address 0x0142 = Page 1, Offset 0x42
```

### Memory Access Patterns

1. **Paged Access** (Code/Data):
   - Set page number
   - Access via offset within page
   - Page switch for cross-page operations

2. **Stack Access**:
   - Pre-allocated contiguous buffer
   - Direct offset addressing
   - Fast push/pop operations

---

## Execution Model

### Program Lifecycle

```
1. VM Construction
   ├─> VMemUnit created
   ├─> Stack created (context 2)
   ├─> InstructionUnit created (code context 0)
   ├─> BasicIO created
   └─> CPU created with dependencies

2. Program Loading
   ├─> vm.load_program(bytecode)
   ├─> InstructionUnit writes to code context
   └─> IR set to 0x0000

3. Execution (vm.run())
   ├─> Set PROTECTED mode
   ├─> Loop until halted:
   │   ├─> cpu.step()
   │   │   ├─> Fetch opcode at IR
   │   │   ├─> Fetch parameters
   │   │   ├─> Dispatch to handler
   │   │   ├─> Execute operation
   │   │   └─> Update IR
   │   └─> Check for HALT (0x01)
   └─> Set UNPROTECTED mode

4. Shutdown
   └─> Automatic cleanup (RAII)
```

### Instruction Execution

```
Step 1: Fetch
┌──────────────────────┐
│ IR → Code Memory     │ → Read opcode byte
└──────────────────────┘

Step 2: Decode
┌──────────────────────┐
│ getArgumentSize()    │ → Determine parameter count
└──────────────────────┘

Step 3: Fetch Parameters
┌──────────────────────┐
│ IR+1 → IR+N          │ → Read 0-4 parameter bytes
└──────────────────────┘

Step 4: Dispatch
┌──────────────────────┐
│ Opcode Range Check   │ → Route to handler
│ if (0x02-0x76)       │    execute_memory_operation()
│ if (0x1E-0x26)       │    execute_jump()
│ if (0x29-0x2D)       │    execute_add_operation()
│ if (0x7F)            │    instruction_unit->system_call()
└──────────────────────┘

Step 5: Execute
┌──────────────────────┐
│ Handler Logic        │ → Perform operation
│ - Access registers   │    Update state
│ - Access memory      │    Set flags
│ - Update flags       │
└──────────────────────┘

Step 6: Advance
┌──────────────────────┐
│ IR += instruction_sz │ → Next instruction
└──────────────────────┘
```

### Subroutine Calling Convention

```
CALL Sequence:
1. Push has_return_value flag (0 or 1) to stack
2. Set frame pointer to current stack top
3. Save return address and frame pointer to return stack
4. Jump IR to subroutine address

Subroutine Body:
1. Push local variables
2. Execute operations
3. If has_return_value, push return value

RET Sequence:
1. If has_return_value:
   - Pop return value from stack
   - Flush stack to frame
   - Push return value back
2. Else:
   - Flush stack to frame
3. Pop flag byte
4. Restore frame pointer from return stack
5. Pop return address from return stack
6. Jump IR to return address
```

---

## Interface Contracts

### IVMemUnit

**Invariants:**
- Context IDs are unique and non-reusable
- Contexts can only be created in UNPROTECTED mode
- Accessors can only be created in PROTECTED mode
- Address validation prevents out-of-bounds access

**Contract:**
```cpp
class IVMemUnit {
    virtual Mode get_mode() const = 0;
    virtual void set_mode(Mode mode) = 0;
    virtual context_id_t create_context(addr32_t capacity) = 0;
    virtual void destroy_context(context_id_t id) = 0;
    virtual std::shared_ptr<Context> get_context(context_id_t id) = 0;
};
```

### IStack

**Invariants:**
- Stack grows upward from address 0
- Cannot pop past frame pointer
- Base ≤ Frame Pointer ≤ Top
- Capacity must not be exceeded

**Contract:**
```cpp
class IStack {
    virtual std::unique_ptr<StackAccessor> get_accessor(MemAccessMode mode) = 0;
    virtual ~IStack() = default;
};
```

### IInstructionUnit

**Invariants:**
- IR must point to valid code address
- Return stack must be balanced (call/return pairs)
- Code context is read-only during execution

**Contract:**
```cpp
class IInstructionUnit {
    virtual std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode) = 0;
    virtual ~IInstructionUnit() = default;
};
```

### IBasicIO

**Invariants:**
- Stack must contain required parameters
- String length must not exceed max_length
- Character encoding is single-byte

**Contract:**
```cpp
class IBasicIO {
    virtual std::unique_ptr<BasicIOAccessor> get_accessor() = 0;
    virtual ~IBasicIO() = default;
};
```

---

## Build and Test Infrastructure

### Build System

**Technology:** CMake 3.20+  
**Compiler:** C++20 compliant (GCC 10+, Clang 12+, MSVC 2019+)

**Build Targets:**
```
lvm                          # Main executable
lvm_memunit                  # VMemUnit library
lvm_stack                    # Stack library
lvm_instruction_unit         # InstructionUnit library
lvm_basic_io                 # BasicIO library
lvm_cpu                      # CPU library
lvm_alu                      # ALU library
lvm_register                 # Register library
lvm_helpers                  # Helpers library
lvm_vm                       # VM library

# Test executables
lvm_memunit_tests
lvm_stack_tests
lvm_instruction_unit_tests
lvm_basic_io_tests
lvm_alu_tests
lvm_register_tests
```

### Test Organization

**Framework:** Google Test 1.14.0  
**Total Tests:** 93 tests  
**Coverage:** All core subsystems

**Test Categories:**
- Unit Tests: Individual component testing
- Integration Tests: Subsystem interaction
- Subroutine Tests: Call/return mechanisms
- Memory Tests: Context isolation and protection

**Test Execution:**
```bash
cmake --build build
cd build
ctest --output-on-failure
```

### Compilation

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run specific test
./build/src/memunit/lvm_memunit_tests

# Run all tests
cd build && ctest
```

---

## Dependencies and Data Flow

### Dependency Graph

```
┌─────────────┐
│   VM        │ (orchestrates all)
└─────┬───────┘
      │
      ├──────────────────┬──────────────────┬─────────────┐
      │                  │                  │             │
      v                  v                  v             v
┌───────────┐     ┌────────────┐    ┌─────────┐   ┌──────────┐
│ VMemUnit  │     │ Stack      │    │BasicIO  │   │   CPU    │
└─────┬─────┘     └──────┬─────┘    └────┬────┘   └────┬─────┘
      │                  │               │             │
      │                  │               │             │
      └──────────────────┴───────────────┴─────────────┤
                                                        │
                         ┌──────────────────────────────┘
                         │
                         v
                  ┌──────────────┐
                  │InstructionUnit│
                  └──────┬────────┘
                         │
                  ┌──────┴──────┬──────────────┐
                  │             │              │
                  v             v              v
            ┌─────────┐   ┌────────┐    ┌─────────┐
            │  Stack  │   │BasicIO │    │VMemUnit │
            └─────────┘   └────────┘    └─────────┘

Additional Components:
┌─────────┐
│   ALU   │ (created by CPU, uses AX)
└─────────┘

┌──────────┐
│ Register │ (used by CPU, ALU)
└──────────┘

┌─────────┐
│  Flags  │ (shared by CPU, ALU, Register, InstructionUnit)
└─────────┘

┌─────────┐
│ Helpers │ (used by all)
└─────────┘
```

### Module Dependencies (CMake)

```
lvm_vm
  └─> lvm_cpu
        ├─> lvm_instruction_unit
        │     ├─> lvm_stack
        │     │     ├─> lvm_memunit
        │     │     │     └─> lvm_helpers
        │     │     └─> lvm_helpers
        │     ├─> lvm_basic_io
        │     │     ├─> lvm_stack
        │     │     └─> lvm_memunit
        │     └─> lvm_helpers
        ├─> lvm_alu
        │     ├─> lvm_register
        │     │     └─> lvm_helpers
        │     └─> lvm_helpers
        ├─> lvm_stack
        └─> lvm_memunit
```

### Data Flow Examples

**Memory Write Operation:**
```
CPU → Register (read value)
    ↓
    InstructionUnit.get_accessor() → PagedMemoryAccessor
    ↓
    VMemUnit.get_context() → Context
    ↓
    Context.allocate_page_if_needed()
    ↓
    Page.write_byte()
```

**Stack Push Operation:**
```
CPU → Register (read value)
    ↓
    Stack.get_accessor() → StackAccessor
    ↓
    StackAccessor.push_word()
    ↓
    StackMemoryAccessor.write_word()
    ↓
    VMemUnit (stack context) → Write bytes
```

**System Call Operation:**
```
CPU (opcode 0x7F) → InstructionUnit.get_accessor()
    ↓
    InstructionUnit_Accessor.system_call(number)
    ↓
    BasicIO.get_accessor() → BasicIOAccessor
    ↓
    BasicIOAccessor.write_string_from_stack()
    ↓
    Stack.get_accessor() → Read parameters
    ↓
    std::cout (output)
```

---

## Summary

Pendragon VM 1.0 represents a complete, production-ready virtual machine with:

✅ **119 opcodes** covering all essential operations  
✅ **Modular architecture** with 9 independent subsystems  
✅ **Interface-based design** enabling testing and flexibility  
✅ **Protected memory model** with context isolation  
✅ **Subroutine support** with proper calling conventions  
✅ **System call interface** for I/O operations  
✅ **93 passing tests** ensuring correctness  
✅ **Complete documentation** for all components  

The system is ready for:
- Language backend implementation
- Instruction set experimentation
- Systems programming research
- Virtual machine education

**Next Steps (Future Versions):**
- Extended instruction set
- File I/O system calls
- Virtual device support
- Debugger and profiler
- 32-bit architecture (next family member)
