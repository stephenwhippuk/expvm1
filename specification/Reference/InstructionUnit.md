# InstructionUnit - Program Counter and Instruction Management

## Purpose

The InstructionUnit manages program execution flow, including instruction pointer (IR) management, program loading, jump operations, and subroutine call/return mechanisms. It bridges the CPU execution logic with code memory and the stack.

## Interface Design

### IInstructionUnit Interface

InstructionUnit implements the `IInstructionUnit` pure virtual interface:

```cpp
class IInstructionUnit {
public:
    virtual std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode) = 0;
    virtual ~IInstructionUnit() = default;
};
```

**Key Points:**
- All operations go through ephemeral `InstructionUnit_Accessor` objects
- CPU depends on IInstructionUnit interface, not concrete InstructionUnit
- InstructionUnit itself depends on IStack interface, not concrete Stack
- Enables testing with mock implementations

## Architecture Overview

### Core Responsibilities

- **Instruction Register (IR)**: Tracks current execution address
- **Program Loading**: Writes bytecode into code memory context
- **Control Flow**: Handles jumps, conditional branches, calls, and returns
- **Return Stack**: Maintains return addresses and frame pointers for nested calls
- **Stack Integration**: Manages subroutine calling conventions

### Execution Model

```
┌─────────────────────────────────────────┐
│       InstructionUnit                   │
├─────────────────────────────────────────┤
│  ┌─────────────────────────────────┐   │
│  │  IR Register (16-bit)           │   │  ← Current instruction address
│  │  Current Value: 0x0042          │   │
│  └─────────────────────────────────┘   │
│                                         │
│  ┌─────────────────────────────────┐   │
│  │  Return Stack                   │   │  ← Call history
│  │  ┌──────────┬──────────┐        │   │
│  │  │ RetAddr  │   FP     │        │   │
│  │  ├──────────┼──────────┤        │   │
│  │  │ 0x0008   │   -1     │  ←Top  │   │
│  │  │ 0x0020   │    5     │        │   │
│  │  └──────────┴──────────┘        │   │
│  └─────────────────────────────────┘   │
│                                         │
│  ┌─────────────────────────────────┐   │
│  │  Code Context Access            │   │  ← Program memory
│  │  (PagedMemoryAccessor)          │   │
│  └─────────────────────────────────┘   │
│                                         │
│  ┌─────────────────────────────────┐   │
│  │  Stack Accessor                 │   │  ← Subroutine frames
│  │  (StackAccessor)                │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

### Return Stack Structure

```cpp
struct ReturnStackItem {
    addr_t return_address;    // Where to return
    int32_t frame_pointer;    // Caller's frame pointer (signed, -1 for no frame)
};
```

## Interface

### InstructionUnit Class

```cpp
class InstructionUnit : public IInstructionUnit {
public:
    // Construction (UNPROTECTED mode only)
    // Note: Takes IVMemUnit&, IStack&, Flags, and BasicIO dependencies
    InstructionUnit(std::shared_ptr<IVMemUnit> vmem_unit, 
                   context_id_t code_context_id,
                   IStack& stack,
                   std::shared_ptr<Flags> flags_ptr,
                   std::shared_ptr<BasicIO> basic_io);
    
    // IInstructionUnit interface implementation
    std::unique_ptr<InstructionUnit_Accessor> get_accessor(MemAccessMode mode) override;
};
```

### InstructionUnit_Accessor Class

```cpp
class InstructionUnit_Accessor {
    // Read-only operations
    word_t get_IR() const;
    word_t readByte_At_IR() const;
    word_t readWord_At_IR() const;
    
    // Program control
    void set_IR(word_t value);
    void advance_IR(word_t offset);
    void Jump_To_Address(addr_t address);
    void Jump_To_Address_Conditional(addr_t address, Flag flag, bool condition);
    
    // Program management
    void Load_Program(const std::vector<byte_t>& program);
    
    // Subroutine support
    void call_subroutine(addr_t address, bool with_return_value = false);
    void return_from_subroutine();
    
    // System call support
    void system_call(word_t syscall_number);
};
```

## Usage Examples

### Example 1: Program Loading and Basic Execution

```cpp
#include "instruction_unit.h"
#include "vmemunit.h"
#include "stack.h"

using namespace lvm;

// Setup VM
VMemUnit vmem_unit;

// Create stack
Stack stack(vmem_unit, 4096);

// Create code context
context_id_t code_ctx = vmem_unit.create_context(8192);

// Create flags
auto flags = std::make_shared<Flags>();

// Get stack accessor
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);
auto stack_accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Create instruction unit (must be in UNPROTECTED mode)
vmem_unit.set_mode(IVMemUnit::Mode::UNPROTECTED);
InstructionUnit instr_unit(vmem_unit, code_ctx, std::move(stack_accessor), flags);

// Switch back to PROTECTED mode for operations
vmem_unit.set_mode(IVMemUnit::Mode::PROTECTED);

// Get accessor
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Load program
std::vector<byte_t> program = {
    0x00, 0x01, 0x34, 0x12,  // LD AX, 0x1234
    0xFF                      // HALT
};
accessor->Load_Program(program);

// Fetch first instruction
byte_t opcode = accessor->readByte_At_IR();  // 0x00
accessor->advance_IR(1);
```

### Example 2: Instruction Pointer Management

```cpp
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Set IR to start of program
accessor->set_IR(0x0000);

// Read opcode at current IR
byte_t opcode = accessor->readByte_At_IR();

// Advance past opcode
accessor->advance_IR(1);

// Read parameter byte
byte_t param1 = accessor->readByte_At_IR();
accessor->advance_IR(1);

// Read parameter word (little-endian)
word_t param2 = accessor->readWord_At_IR();
accessor->advance_IR(2);

// Get current position
word_t current_ir = accessor->get_IR();
```

### Example 3: Unconditional Jump

```cpp
// Program with jump
std::vector<byte_t> program = {
    0x00, 0x01, 0x01, 0x00,  // LD AX, 1      (address 0x0000)
    0x36, 0x08, 0x00,        // JMP 0x0008    (address 0x0004)
    0x00, 0x01, 0x02, 0x00,  // LD AX, 2      (address 0x0007, skipped)
    0x00, 0x01, 0x03, 0x00,  // LD AX, 3      (address 0x000B, executed)
    0xFF                      // HALT
};

auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);
accessor->Load_Program(program);

// Execute jump
accessor->set_IR(0x0004);  // At JMP instruction
accessor->advance_IR(1);   // Past opcode
word_t target = accessor->readWord_At_IR();  // 0x0008
accessor->Jump_To_Address(target);

// IR now = 0x0008, skipped LD AX, 2
```

### Example 4: Conditional Jump

```cpp
#include "flags.h"

auto flags = std::make_shared<Flags>();
// ... create instruction unit with flags ...

auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Set zero flag
flags->set(Flag::ZERO, true);

// Conditional jump if zero flag is set
addr_t jump_target = 0x0020;
accessor->Jump_To_Address_Conditional(jump_target, Flag::ZERO, true);
// IR = 0x0020 (jump taken)

// Conditional jump if zero flag is NOT set
flags->set(Flag::ZERO, false);
accessor->set_IR(0x0010);
accessor->Jump_To_Address_Conditional(0x0030, Flag::ZERO, true);
// IR = 0x0010 (jump not taken, zero flag is false)

// Jump if carry flag is NOT set
flags->set(Flag::CARRY, false);
accessor->Jump_To_Address_Conditional(0x0040, Flag::CARRY, false);
// IR = 0x0040 (jump taken, condition matches)
```

### Example 5: Subroutine Call Without Return Value

```cpp
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Load program with subroutine
std::vector<byte_t> program = {
    // Main
    0x00, 0x01, 0x05, 0x00,  // LD AX, 5      (0x0000)
    0x47, 0x08, 0x00,        // CALL 0x0008   (0x0004)
    0xFF,                     // HALT          (0x0007)
    
    // Subroutine at 0x0008
    0x34, 0x01,              // INC AX        (0x0008)
    0x4B                      // RET           (0x000A)
};

accessor->Load_Program(program);
accessor->set_IR(0x0004);  // At CALL instruction

// Execute call (no return value)
addr_t subroutine_addr = 0x0008;
accessor->call_subroutine(subroutine_addr, false);

// After call_subroutine():
// - IR = 0x0008 (at subroutine)
// - Flag byte (0) pushed to stack
// - Frame set to top (flag is protected)
// - Return address (0x0004) saved in return stack
// - Caller's FP saved in return stack

// Execute subroutine...

// Return from subroutine
accessor->return_from_subroutine();

// After return:
// - IR = 0x0004 (return address)
// - Frame restored to caller's FP
// - Flag byte removed from stack
```

### Example 6: System Call Execution

```cpp
#include "instruction_unit.h"
#include "basic_io.h"
#include "systemcalls.h"

using namespace lvm;

// Setup (assumes vmem_unit, stack, flags already created)
auto basic_io = std::make_shared<BasicIO>(vmem_unit, stack);

InstructionUnit instr_unit(vmem_unit, code_ctx, *stack, flags, basic_io);
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Example: Print string system call
// Stack should have: [count] [char_n] ... [char_1]
// Push "Hello" onto stack (reverse order)
auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
stack_accessor->push_byte('o');
stack_accessor->push_byte('l');
stack_accessor->push_byte('l');
stack_accessor->push_byte('e');
stack_accessor->push_byte('H');
stack_accessor->push_word(5);  // Count

// Execute system call
accessor->system_call(SYSCALL_PRINT_STRING_FROM_STACK);
// Output: "Hello" to console
```

### Example 7: Subroutine Call With Return Value

```cpp
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Program with return value
std::vector<byte_t> program = {
    // Main
    0x00, 0x01, 0x05, 0x00,  // LD AX, 5      (0x0000)
    0x47, 0x08, 0x00,        // CALL 0x0008   (0x0004)
    0x1C, 0x02,              // POP BX        (0x0007, get return value)
    0xFF,                     // HALT          (0x0009)
    
    // Subroutine: double AX
    0x4C, 0x01,              // ADD AX        (0x000A, AX *= 2)
    0x18, 0x01,              // PUSH AX       (0x000C, return value)
    0x4B                      // RET           (0x000E)
};

accessor->Load_Program(program);
accessor->set_IR(0x0004);

// Call with return value expected
accessor->call_subroutine(0x000A, true);

// After call:
// - Flag byte (1) pushed and protected
// - IR = 0x000A

// In subroutine, can check flag at frame offset 0
// to confirm return value expected

// After return_from_subroutine():
// - Return value left on stack
// - Caller can pop it into register
```

### Example 7: Nested Subroutine Calls

```cpp
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Main calls Sub1, Sub1 calls Sub2
std::vector<byte_t> program = {
    // Main
    0x47, 0x06, 0x00,        // CALL Sub1     (0x0000)
    0xFF,                     // HALT          (0x0003)
    
    // Padding
    0x90, 0x90,
    
    // Sub1 at 0x0006
    0x47, 0x0C, 0x00,        // CALL Sub2     (0x0006)
    0x4B,                     // RET           (0x0009)
    
    // Padding
    0x90, 0x90,
    
    // Sub2 at 0x000C
    0x00, 0x01, 0x42, 0x00,  // LD AX, 0x42   (0x000C)
    0x4B                      // RET           (0x0010)
};

accessor->Load_Program(program);

// Call Sub1
accessor->set_IR(0x0000);
accessor->call_subroutine(0x0006, false);
// Return stack: [{ret: 0x0000, fp: -1}]
// IR = 0x0006

// Sub1 calls Sub2
accessor->call_subroutine(0x000C, false);
// Return stack: [{ret: 0x0000, fp: -1}, {ret: 0x0006, fp: 0}]
// IR = 0x000C

// Return from Sub2
accessor->return_from_subroutine();
// Return stack: [{ret: 0x0000, fp: -1}]
// IR = 0x0006, FP = 0

// Return from Sub1
accessor->return_from_subroutine();
// Return stack: []
// IR = 0x0000, FP = -1
```

### Example 8: Program Counter Arithmetic

```cpp
auto accessor = instr_unit.get_accessor(MemAccessMode::READ_WRITE);

// Load multi-byte instruction
std::vector<byte_t> program = {
    0x00, 0x01, 0x34, 0x12,  // LD AX, 0x1234 (4 bytes)
    0x00, 0x02, 0x78, 0x56,  // LD BX, 0x5678 (4 bytes)
};

accessor->Load_Program(program);
accessor->set_IR(0x0000);

// Read opcode
byte_t opcode = accessor->readByte_At_IR();  // 0x00
accessor->advance_IR(1);  // IR = 0x0001

// Read register code
byte_t reg = accessor->readByte_At_IR();  // 0x01
accessor->advance_IR(1);  // IR = 0x0002

// Read immediate word
word_t value = accessor->readWord_At_IR();  // 0x1234
accessor->advance_IR(2);  // IR = 0x0004

// Now at second instruction
byte_t next_opcode = accessor->readByte_At_IR();  // 0x00
```

## Key Design Patterns

### Calling Convention

**Call sequence:**
```cpp
// 1. Save return context
ReturnStackItem item;
item.return_address = ir_register->get_value();
item.frame_pointer = stack_accessor->get_fp();
return_stack.push_back(item);

// 2. Jump to subroutine
ir_register->set_value(address);

// 3. Push flag byte and create frame
stack_accessor->push_byte(with_return_value ? 1 : 0);
stack_accessor->set_frame_to_top();
```

**Return sequence:**
```cpp
// 1. Restore context
ReturnStackItem item = return_stack.back();
return_stack.pop_back();
ir_register->set_value(item.return_address);

// 2. Check return value flag
byte_t has_return_value = stack_accessor->peek_byte_from_frame(0);

// 3. Handle return value if present
if (has_return_value) {
    // Leave value on stack for caller
}

// 4. Clean up frame
stack_accessor->flush();
stack_accessor->set_frame_pointer(item.frame_pointer);
stack_accessor->pop_byte();  // Remove flag
```

### Program Loading with Paging

```cpp
void InstructionUnit::load_program(const std::vector<byte_t>& program) {
    auto code_ctx = vmem_unit_->get_context(code_context_id_);
    auto code_accessor = code_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
    
    addr32_t addr = 0;
    for (byte_t byte : program) {
        page_t page = addr / 256;      // Page number
        addr_t offset = addr % 256;     // Offset within page
        code_accessor->set_page(page);
        code_accessor->write_byte(offset, byte);
        addr++;
    }
}
```

## System Call Support

The InstructionUnit handles system calls (opcode 0x7F) by delegating to the BasicIO subsystem. System calls provide access to I/O and operating system services.

### System Call Mechanism

```cpp
void InstructionUnit_Accessor::system_call(word_t syscall_number) {
    // Delegates to BasicIO through accessor
    auto io_accessor = basic_io_->get_accessor();
    
    switch (syscall_number) {
        case SYSCALL_PRINT_STRING_FROM_STACK:   // 0x0010
        case SYSCALL_PRINT_LINE_FROM_STACK:     // 0x0011
        case SYSCALL_READ_LINE_ONTO_STACK:      // 0x0012
            // Handled by BasicIO
            break;
        default:
            throw runtime_error("Invalid system call number");
    }
}
```

### Dependencies

- **BasicIO**: Required dependency injected during InstructionUnit construction
- **Stack**: Used by BasicIO to read/write I/O data
- **System Call Table**: Defined in `systemcalls.h`

### Supported System Calls

| Index | Hex    | Name                       | Description |
|-------|--------|----------------------------|-------------|
| 16    | 0x0010 | PRINT_STRING_FROM_STACK    | Output string to console |
| 17    | 0x0011 | PRINT_LINE_FROM_STACK      | Output string with newline |
| 18    | 0x0012 | READ_LINE_ONTO_STACK       | Read line from console input |

See `specification/SystemCalls.md` for detailed documentation of each system call.

## Error Handling

Common exceptions:
- `runtime_error("InstructionUnit requires a valid StackAccessor")` - Null stack pointer
- `runtime_error("InstructionUnit must be created in unprotected mode")` - Wrong mode
- `runtime_error("Return stack underflow on return from subroutine")` - Return without call
- `runtime_error("Page out of bounds")` - Invalid program counter address
- `runtime_error("Program too large for code context")` - Program exceeds capacity
- `runtime_error("Invalid system call number: <number>")` - Unsupported system call

## Performance Considerations

- **IR Access**: O(1) - Direct register read/write
- **Program Loading**: O(n) where n is program size - Sequential page writes
- **Jump Operations**: O(1) - Direct IR assignment
- **Subroutine Call**: O(1) - Push to return stack, stack operations
- **Return**: O(1) - Pop from return stack, stack cleanup

## Memory Protection

The InstructionUnit enforces mode discipline:
- **Construction**: Must occur in UNPROTECTED mode
- **Accessor Creation**: Must occur in PROTECTED mode
- **Operations**: Accessor methods work in PROTECTED mode

## Return Stack vs Call Stack

- **Return Stack**: Internal to InstructionUnit, stores return addresses and frame pointers
- **Call Stack**: Stack managed by StackAccessor, stores local variables and parameters
- Both work together to support nested subroutines
- Return stack ensures proper unwinding even with multiple nested calls
