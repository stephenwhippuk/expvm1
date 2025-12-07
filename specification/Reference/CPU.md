# CPU - Central Processing Unit

## Purpose

The CPU is the primary execution engine of the Pendragon VM, responsible for fetching instructions, decoding opcodes, dispatching operations to appropriate handlers, and managing the overall program execution lifecycle. It uses an interface-based architecture with dependency injection, integrating subsystems through their abstract interfaces (IVMemUnit, IStack, IInstructionUnit, IALU) to enable loose coupling and testability.

## Interface Design

### Dependency Injection Architecture

The CPU follows a dependency injection pattern where subsystems are created externally and injected via interfaces:

```cpp
// CPU header depends ONLY on interfaces
#include "ivmemunit.h"
#include "istack.h"
#include "iinstruction_unit.h"
#include "alu.h"  // Created internally, not injected

class Cpu {
private:
    IVMemUnit& vmem_unit_;           // Injected via constructor
    IStack* stack_;                  // Injected via set_stack()
    IInstructionUnit* instr_unit_;   // Injected via set_instruction_unit()
    std::unique_ptr<Alu> alu_;       // Created internally (needs AX register)
};
```

**Benefits:**
- **Decoupled Compilation**: CPU header includes only interfaces, not concrete implementations
- **Testability**: Can inject mock implementations for unit testing
- **Flexibility**: Different subsystem implementations can be used
- **Clear Contracts**: Interface defines exactly what CPU needs from each subsystem

**Why ALU is Different:**
The ALU is created internally by the CPU because it requires direct access to the AX register for flag management. This tight coupling makes dependency injection unnecessary and potentially harmful to encapsulation.

### Construction and Initialization Sequence

1. **VM creates subsystems** in UNPROTECTED mode:
   - VMemUnit (created first)
   - Stack (needs VMemUnit)
   - InstructionUnit (needs VMemUnit, Stack accessor, Flags)

2. **Switch to PROTECTED mode** for runtime operations

3. **Create CPU** with IVMemUnit reference:
   - CPU stores reference to IVMemUnit&
   - CPU creates ALU internally with AX register

4. **Inject subsystems** via setter methods:
   - `cpu.set_stack(*stack)` - Injects IStack interface
   - `cpu.set_instruction_unit(*instruction_unit)` - Injects IInstructionUnit interface

5. **Ready for execution** - All subsystems connected via interfaces

## Architecture Overview

### Core Components

The CPU integrates the following subsystems through dependency injection:
- **IVMemUnit&**: Virtual memory management interface (injected via constructor)
- **IStack***: Call stack and local variables interface (injected via set_stack())
- **IInstructionUnit***: Program counter and instruction fetching interface (injected via set_instruction_unit())
- **Alu**: Arithmetic and logical operations (created internally, owns unique_ptr<Alu>)
- **Register File**: 8 general-purpose 16-bit registers (AX-HX)
- **Flags**: Zero, Sign, Carry, Overflow status bits

### Execution Model

```
┌─────────────────────────────────────┐
│           CPU Core                  │
├─────────────────────────────────────┤
│  ┌──────┐  ┌──────┐  ┌──────┐     │
│  │  AX  │  │  BX  │  │  CX  │ ... │  ← Registers
│  └──────┘  └──────┘  └──────┘     │
│                                     │
│  ┌─────────────────────────────┐   │
│  │     Instruction Unit         │   │  ← Program Counter
│  │     (IR, Code Context)       │   │
│  └─────────────────────────────┘   │
│                                     │
│  ┌─────────────────────────────┐   │
│  │          ALU                 │   │  ← Arithmetic/Logic
│  └─────────────────────────────┘   │
│                                     │
│  ┌─────────────────────────────┐   │
│  │         Stack               │   │  ← Call Stack
│  └─────────────────────────────┘   │
└─────────────────────────────────────┘
         │            │
         ▼            ▼
   Code Context   Data Context
   (Programs)     (Variables)
```

### Operation Dispatch

The CPU uses range-based dispatch to route opcodes to specialized handlers:

1. **Memory Operations** (0x02-0x0F, 0x10-0x1D, 0x72-0x76): Load, store, push, pop
   - Includes PUSHW (0x75) and PUSHB (0x76) for immediate value stack operations
2. **Jump Operations** (0x1E-0x26): Conditional and unconditional branching
3. **Subroutine Operations** (0x27-0x28): Call, return, frame management
4. **Arithmetic Operations** (0x29-0x3D): Add, subtract, multiply, divide, remainder
5. **Logical Operations** (0x42-0x55): AND, OR, XOR, NOT
6. **Shift/Rotate Operations** (0x56-0x69): Bit manipulation
7. **Increment/Decrement** (0x6A-0x6B): Register arithmetic
8. **Compare Operations** (0x6C-0x71): Set flags based on comparison
9. **Register-Indirect Load** (0x72-0x74): Load from address in register
10. **System Calls** (0x7F): Delegated to InstructionUnit for I/O and OS services

## Interface

### CPU Class

```cpp
class Cpu {
    // Construction with interface-based dependency injection
    Cpu(std::shared_ptr<IVMemUnit> vmem_unit);
    ~Cpu();
    
    // Dependency injection
    void set_stack(IStack& stack);
    void set_instruction_unit(IInstructionUnit& instruction_unit);
    
    // Lifecycle
    void initialize();
    void load_program(const std::vector<byte_t>& program);
    void run();
    
private:
    // Execution
    void step();
    
    // Operation handlers
    void execute_memory_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_jump(byte_t opcode, addr_t address);
    void execute_add_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_sub_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_mul_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_div_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_rem_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_and_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_or_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_xor_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_shift_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_rotate_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_cmp_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_inc_dec_operation(byte_t opcode, const std::vector<byte_t>& params);
    void execute_subroutine_operation(byte_t opcode, const std::vector<byte_t>& params);
    
    // Note: System calls (0x7F) are delegated to InstructionUnit::system_call()
    // which handles I/O and OS services via the BasicIO subsystem
    
    // Register management
    std::shared_ptr<Register> get_register_by_code(byte_t code);
};
```

### Register Codes

```cpp
enum register_codes {
    REG_AX = 0x01,
    REG_BX = 0x02,
    REG_CX = 0x03,
    REG_DX = 0x04,
    REG_EX = 0x05,
    REG_IR = 0x06,  // Instruction Register (program counter)
    REG_SP = 0x07,  // Stack Pointer
    REG_SI = 0x08,  // Source Index
};
```

## Usage Examples

### Example 1: Basic CPU Setup via VM (Recommended)

```cpp
#include "vm.h"
#include <vector>

using namespace lvm;

// VM handles all subsystem creation and injection
VM vm(4096, 8192);  // 4KB stack, 8KB code

// Create a simple program
std::vector<byte_t> program = {
    0x00, 0x01, 0x34, 0x12,  // LD AX, 0x1234
    0x00, 0x02, 0x78, 0x56,  // LD BX, 0x5678
    0x4C, 0x02,              // ADD BX (AX += BX)
    0xFF                     // HALT
};

// Load program and run
vm.load_program(program);
vm.run();
```

### Example 1b: Manual CPU Construction with Dependency Injection

```cpp
#include "cpu.h"
#include "vmemunit.h"
#include "stack.h"
#include "instruction_unit.h"

using namespace lvm;

// Create virtual memory unit
VMemUnit vmem_unit;

// Create subsystems in UNPROTECTED mode
auto stack = std::make_unique<Stack>(vmem_unit, 4096);  // 4KB stack
context_id_t code_ctx = vmem_unit.create_context(8192);  // 8KB code

// Get stack accessor for InstructionUnit
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);
auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
vmem_unit.set_mode(IVMemUnit::Mode::UNPROTECTED);

auto flags = std::make_shared<Flags>();
auto instruction_unit = std::make_unique<InstructionUnit>(
    vmem_unit, code_ctx, std::move(stack_accessor), flags);

// Switch to protected mode
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);

// Create CPU with interface reference
Cpu cpu(vmem_unit);

// Inject subsystems via interfaces
cpu.set_stack(*stack);
cpu.set_instruction_unit(*instruction_unit);

// Now ready for program execution
std::vector<byte_t> program = {
    0x00, 0x01, 0x34, 0x12,  // LD AX, 0x1234
    0xFF                     // HALT
};
cpu.load_program(program);
cpu.run();
```

### Example 2: Memory Operation Execution

```cpp
// Program using memory operations
std::vector<byte_t> program = {
    // Load immediate value into AX
    0x00, 0x01, 0x00, 0x10,  // LD AX, 0x1000
    
    // Store AX to data memory at address 0x2000
    0x0E, 0x00, 0x20, 0x01,  // STA 0x2000, AX
    
    // Load from memory into BX
    0x06, 0x02, 0x00, 0x20,  // LDA BX, 0x2000
    
    0xFF                     // HALT
};

cpu.load_program(program);
cpu.run();
```

### Example 3: Conditional Branching

```cpp
// Program with conditional jump
std::vector<byte_t> program = {
    // Load values
    0x00, 0x01, 0x0A, 0x00,  // LD AX, 10
    0x00, 0x02, 0x14, 0x00,  // LD BX, 20
    
    // Compare AX and BX
    0x73, 0x02,              // CMP BX
    
    // Jump if less (AX < BX) to address 0x0014
    0x3F, 0x14, 0x00,        // JL 0x0014
    
    // Not less path
    0x00, 0x03, 0x01, 0x00,  // LD CX, 1
    0x36, 0x18, 0x00,        // JMP 0x0018
    
    // Less path (address 0x0014)
    0x00, 0x03, 0x02, 0x00,  // LD CX, 2
    
    // Continue (address 0x0018)
    0xFF                     // HALT
};

cpu.load_program(program);
cpu.run();
// Result: CX = 2 (since 10 < 20)
```

### Example 4: Subroutine Call

```cpp
// Program with subroutine
std::vector<byte_t> program = {
    // Main routine
    0x00, 0x01, 0x05, 0x00,  // LD AX, 5      (address 0x0000)
    0x00, 0x02, 0x03, 0x00,  // LD BX, 3      (address 0x0004)
    
    // Call subroutine at 0x0014
    0x47, 0x14, 0x00,        // CALL 0x0014   (address 0x0008)
    
    // After return, result in AX
    0xFF,                    // HALT          (address 0x000B)
    
    // Padding to address 0x0014
    0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90,
    
    // Subroutine: Add AX + BX (address 0x0014)
    0x4C, 0x02,              // ADD BX        (AX += BX)
    0x4B                     // RET           (address 0x0016)
};

cpu.load_program(program);
cpu.run();
// Result: AX = 8 (5 + 3)
```

### Example 5: Stack Operations

```cpp
// Program using stack for temporary storage
std::vector<byte_t> program = {
    // Save registers
    0x00, 0x01, 0xAA, 0xBB,  // LD AX, 0xBBAA
    0x00, 0x02, 0xCC, 0xDD,  // LD BX, 0xDDCC
    
    // Push to stack
    0x18, 0x01,              // PUSH AX
    0x18, 0x02,              // PUSH BX
    
    // Modify registers
    0x00, 0x01, 0x00, 0x00,  // LD AX, 0
    0x00, 0x02, 0x00, 0x00,  // LD BX, 0
    
    // Restore from stack (LIFO order)
    0x1C, 0x02,              // POP BX
    0x1C, 0x01,              // POP AX
    
    0xFF                     // HALT
};

cpu.load_program(program);
cpu.run();
// Result: AX = 0xBBAA, BX = 0xDDCC (restored)
```

### Example 6: Register-Indirect Load

```cpp
// Load address from register (new in 0x72-0x74)
std::vector<byte_t> program = {
    // Load memory address into BX
    0x00, 0x02, 0x00, 0x10,  // LD BX, 0x1000
    
    // Store data at 0x1000
    0x00, 0x01, 0x42, 0x00,  // LD AX, 0x42
    0x0E, 0x00, 0x10, 0x01,  // STA 0x1000, AX
    
    // Load from address stored in BX
    0x72, 0x01, 0x02,        // LDA AX, [BX]  (register-indirect)
    
    0xFF                     // HALT
};

cpu.load_program(program);
cpu.run();
// Result: AX = 0x42 (loaded from address in BX)
```

### Example 7: ALU Operations

```cpp
// Arithmetic and logical operations
std::vector<byte_t> program = {
    // Setup
    0x00, 0x01, 0x0F, 0x00,  // LD AX, 15
    0x00, 0x02, 0x03, 0x00,  // LD BX, 3
    
    // Multiply
    0x5A, 0x02,              // MUL BX  (AX = 15 * 3 = 45)
    
    // Divide
    0x00, 0x02, 0x05, 0x00,  // LD BX, 5
    0x62, 0x02,              // DIV BX  (AX = 45 / 5 = 9)
    
    // Remainder
    0x00, 0x02, 0x04, 0x00,  // LD BX, 4
    0x6A, 0x02,              // REM BX  (AX = 9 % 4 = 1)
    
    // Logical AND
    0x00, 0x02, 0xFF, 0x00,  // LD BX, 0xFF
    0x72, 0x02,              // AND BX  (AX = 1 & 0xFF = 1)
    
    0xFF                     // HALT
};

cpu.load_program(program);
cpu.run();
// Result: AX = 1
```

## Key Design Patterns

### Fetch-Decode-Execute Cycle

```cpp
void Cpu::step() {
    // Fetch opcode
    byte_t opcode = accessor->readByte_At_IR();
    accessor->advance_IR(1);
    
    // Check for special opcodes
    if (opcode == OPCODE_HALT) {
        halted = true;
        return;
    }
    
    // Fetch parameters
    std::vector<byte_t> params;
    int param_count = get_additional_bytes(opcode);
    for (int i = 0; i < param_count; ++i) {
        params.push_back(accessor->readByte_At_IR() + 1 + i);
        accessor->advance_IR(1);
    }
    
    // Dispatch to handler
    if (opcode >= OPCODE_LD_REG_IMM_W && opcode <= OPCODE_STAL_ADDR_REG_B) {
        execute_memory_operation(opcode, params);
    }
    // ... more dispatch ranges
}
```

### Range-Based Opcode Dispatch

Efficient opcode routing using contiguous ranges:

```cpp
// Memory operations: 0x00-0x33
if (opcode >= OPCODE_LD_REG_IMM_W && opcode <= OPCODE_STAL_ADDR_REG_B) {
    execute_memory_operation(opcode, params);
    return;
}

// Register-indirect loads: 0x72-0x74
if (opcode >= OPCODE_LDA_REG_REGADDR_W && opcode <= OPCODE_LDAL_REG_REGADDR_B) {
    execute_memory_operation(opcode, params);
    return;
}

// Jumps: 0x36-0x46
if (opcode >= OPCODE_JMP_ADDR && opcode <= OPCODE_JPNO_ADDR) {
    execute_jump(opcode, combine_bytes_to_address(params[0], params[1]));
    return;
}
```

### Register Code Mapping

```cpp
std::shared_ptr<Register> Cpu::get_register_by_code(byte_t code) {
    switch (code) {
        case REG_AX: return AX;
        case REG_BX: return BX;
        case REG_CX: return CX;
        case REG_DX: return DX;
        case REG_EX: return EX;
        default:
            throw std::runtime_error("Invalid register code");
    }
}
```

### Memory Context Isolation

- **Code Context**: Read-only during execution, stores program
- **Data Context**: Read-write, stores variables and heap
- **Stack Context**: Managed by Stack, isolated from code/data

## Initialization Sequence

```cpp
Cpu::Cpu(VMemUnit& vmem_unit, addr32_t stack_capacity, addr32_t code_capacity)
    : vmem_unit_(vmem_unit),
      flags(std::make_shared<Flags>()),
      AX(std::make_shared<Register>(flags)),
      BX(std::make_shared<Register>(flags)),
      CX(std::make_shared<Register>(flags)),
      DX(std::make_shared<Register>(flags)),
      EX(std::make_shared<Register>(flags)),
      alu(std::make_unique<Alu>(AX))
{
    // Create stack context
    stack = std::make_unique<Stack>(vmem_unit_, stack_capacity);
    
    // Create code context
    code_context_id_ = vmem_unit_.create_context(code_capacity);
    
    // Create data context
    data_context_id_ = vmem_unit_.create_context(65536);  // 64KB
    
    // Create instruction unit
    vmem_unit_.set_mode(VMemUnit::Mode::PROTECTED);
    auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    vmem_unit_.set_mode(VMemUnit::Mode::UNPROTECTED);
    
    instruction_unit = std::make_unique<InstructionUnit>(
        vmem_unit_, code_context_id_, std::move(stack_accessor), flags);
}
```

## New Instructions

### PUSHW (0x75) - Push Immediate Word

Pushes a 16-bit immediate value directly onto the stack.

**Opcode**: 0x75  
**Arguments**: WORD (2 bytes, little-endian)  
**Total Size**: 3 bytes  

**Usage**:
```cpp
0x75, 0x34, 0x12  // PUSHW 0x1234
```

**Operation**:
- Reads 2-byte immediate value from instruction stream
- Pushes word onto stack (low byte first, then high byte)

### PUSHB (0x76) - Push Immediate Byte

Pushes an 8-bit immediate value directly onto the stack.

**Opcode**: 0x76  
**Arguments**: BYTE (1 byte)  
**Total Size**: 2 bytes  

**Usage**:
```cpp
0x76, 0xAA  // PUSHB 0xAA
```

**Operation**:
- Reads 1-byte immediate value from instruction stream
- Pushes byte onto stack

**Benefits**:
- Eliminates need to load value into register first
- More compact code for pushing constants
- Ideal for system call parameters and string literals

## System Call Integration

The CPU dispatches system call opcodes (0x7F) to the InstructionUnit, which delegates to the BasicIO subsystem. System calls enable:

- Console I/O (print, read)
- Future: File operations, network I/O, process control

**System Call Execution Flow**:
1. CPU fetches SYS opcode (0x7F) and system call number
2. CPU delegates to InstructionUnit accessor
3. InstructionUnit calls `system_call(syscall_number)`
4. InstructionUnit delegates to BasicIO accessor
5. BasicIO reads/writes stack data and performs I/O operation

See `specification/SystemCalls.md` for complete system call reference.

## Error Handling

Common exceptions:
- `runtime_error("Invalid register code")` - Unknown register code in instruction
- `runtime_error("Division by zero")` - Attempted division/remainder by zero
- `runtime_error("Invalid opcode")` - Unrecognized instruction
- `runtime_error("Program too large")` - Program exceeds code context size
- `runtime_error("Stack overflow")` - Stack capacity exceeded
- `runtime_error("Invalid jump address")` - Jump to invalid code location
- `runtime_error("Invalid system call number")` - Unsupported system call (delegated to InstructionUnit)

## Performance Considerations

- **Opcode Dispatch**: O(1) using range checks
- **Register Access**: O(1) direct pointer access
- **Parameter Fetching**: O(n) where n is parameter count (typically 0-4 bytes)
- **Memory Protection**: Context switching uses mode flags (minimal overhead)
- **Stack Operations**: O(1) via Stack

## Thread Safety

The CPU is **not thread-safe**. Each CPU instance should be used by a single thread. Multiple CPUs can operate on separate VMemUnit instances concurrently.

## Execution Flow

```
1. load_program()
   └─> InstructionUnit loads bytes into code context
   
2. run()
   └─> while (!halted)
       └─> step()
           ├─> Fetch opcode at IR
           ├─> Fetch parameters
           ├─> Dispatch to handler
           │   ├─> execute_memory_operation()
           │   ├─> execute_jump()
           │   ├─> execute_add_operation()
           │   └─> ... other handlers
           └─> Advance IR
```
