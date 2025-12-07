# Stack2 - Upward-Growing Stack with Frame Support

## Purpose

Stack2 provides a modern upward-growing stack implementation with frame pointer support for subroutine calls. It enables clean separation between stack frames, local variables, and supports both absolute and frame-relative addressing.

## Architecture Overview

### Core Concepts

- **Upward Growth**: Stack grows from address 0 upward (opposite of traditional stacks)
- **Frame Pointer**: Signed 32-bit pointer supporting -1 initial state
- **Stack Pointer**: Points to next available position
- **Frame-Relative Addressing**: Access data relative to frame pointer
- **Frame Protection**: Cannot pop below current frame pointer

### Stack Layout

```
High Address
    │
    ├── [Local Variable 3]    ← SP (next push goes here)
    ├── [Local Variable 2]    
    ├── [Local Variable 1]    
    ├── [Flag Byte]           ← FP (frame start, protected)
    ├── [Caller Data 2]       
    ├── [Caller Data 1]       
    └── [Base]                ← Address 0
Low Address
```

### Frame Pointer Semantics

- **Initial State**: FP = -1 (no frame set)
- **After `set_frame_to_top()`**: FP = SP - 1 (points to last pushed item)
- **Frame Offset 0**: Accesses data at address FP (the protected item)
- **Frame Offset N**: Accesses data at address FP + N

## Interface

### Stack2 Class

```cpp
class Stack2 {
    // Construction (UNPROTECTED mode only)
    Stack2(VMemUnit& vmem_unit, addr32_t capacity);
    
    // Accessor creation (PROTECTED mode only)
    std::unique_ptr<StackAccessor2> get_accessor(MemAccessMode mode);
};
```

### StackAccessor2 Class

```cpp
class StackAccessor2 {
    // Basic stack operations
    void push_byte(byte_t value);
    void push_word(word_t value);
    byte_t pop_byte();
    word_t pop_word();
    
    // Peek operations (absolute addressing)
    byte_t peek_byte_from_base(addr32_t offset);
    word_t peek_word_from_base(addr32_t offset);
    
    // Frame operations
    int32_t get_fp() const;
    addr32_t get_sp() const;
    void set_frame_pointer(int32_t value);
    void set_frame_to_top();
    
    // Frame-relative operations
    byte_t peek_byte_from_frame(addr32_t offset);
    word_t peek_word_from_frame(addr32_t offset);
    
    // Frame management
    void flush();  // Clear back to FP+1
    bool is_empty() const;  // SP == FP+1
};
```

## Usage Examples

### Example 1: Basic Stack Operations

```cpp
#include "stack_new.h"
#include "vmemunit.h"

// Create VM and stack
VMemUnit vmem_unit;
Stack2 stack(vmem_unit, 1024);  // 1KB capacity

// Switch to protected mode
vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);

// Get accessor
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Push data
accessor->push_byte(0x42);
accessor->push_word(0x1234);

// Pop data (LIFO order)
word_t value = accessor->pop_word();  // 0x1234
byte_t byte = accessor->pop_byte();   // 0x42
```

### Example 2: Subroutine Call Setup

```cpp
// Caller's code
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Push arguments for subroutine
accessor->push_word(0x1234);  // arg1
accessor->push_word(0x5678);  // arg2

// Save current frame pointer
int32_t caller_fp = accessor->get_fp();  // -1 initially

// Push flag indicating return value expected
accessor->push_byte(1);  // 1 = has return value

// Create new frame (flag is now protected)
accessor->set_frame_to_top();  

int32_t callee_fp = accessor->get_fp();  // Now points to flag
// Frame offset 0 will access the flag
// Stack grows upward from here for local variables
```

### Example 3: Subroutine with Local Variables

```cpp
// Inside subroutine after frame setup
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Check if return value expected
byte_t has_return = accessor->peek_byte_from_frame(0);  // Read flag

// Access arguments from caller's frame (via absolute addressing)
word_t arg1 = accessor->peek_word_from_base(0);  // First argument
word_t arg2 = accessor->peek_word_from_base(2);  // Second argument

// Allocate local variables
accessor->push_byte(0xAA);  // local1 at frame offset 1
accessor->push_byte(0xBB);  // local2 at frame offset 2
accessor->push_word(0xCCDD);  // local3 at frame offset 3-4

// Access local variables
byte_t local1 = accessor->peek_byte_from_frame(1);  // 0xAA
word_t local3 = accessor->peek_word_from_frame(3);  // 0xCCDD
```

### Example 4: Subroutine Return

```cpp
// Return from subroutine
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Check if caller expects return value
byte_t has_return = accessor->peek_byte_from_frame(0);

if (has_return) {
    // Push return value
    accessor->push_word(0xABCD);
    
    // Save return value temporarily
    word_t return_value = accessor->pop_word();
    
    // Flush local variables (clears back to FP+1)
    accessor->flush();
    
    // Restore caller's frame
    accessor->set_frame_pointer(caller_fp);
    
    // Remove flag byte
    accessor->pop_byte();
    
    // Push return value for caller
    accessor->push_word(return_value);
} else {
    // No return value needed
    accessor->flush();
    accessor->set_frame_pointer(caller_fp);
    accessor->pop_byte();  // Remove flag
}
```

### Example 5: Nested Subroutine Calls

```cpp
VMemUnit vmem_unit;
Stack2 stack(vmem_unit, 1024);
vmem_unit.set_mode(VMemUnit::Mode::PROTECTED);
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// First call
int32_t main_fp = accessor->get_fp();  // -1
accessor->push_byte(0);  // No return value
accessor->set_frame_to_top();
int32_t sub1_fp = accessor->get_fp();  // 0

// Push local
accessor->push_byte(0x11);

// Nested call
accessor->push_byte(1);  // Expects return value
accessor->set_frame_to_top();
int32_t sub2_fp = accessor->get_fp();  // 2

// Push local in nested call
accessor->push_byte(0x22);

// Return from nested (sub2)
accessor->push_word(0x9999);  // return value
word_t ret = accessor->pop_word();
accessor->flush();
accessor->set_frame_pointer(sub1_fp);
accessor->pop_byte();
accessor->push_word(ret);

// Process return value in sub1
word_t result = accessor->pop_word();  // 0x9999

// Return from sub1
accessor->flush();
accessor->set_frame_pointer(main_fp);
accessor->pop_byte();
```

### Example 6: Frame Protection

```cpp
auto accessor = stack.get_accessor(MemAccessMode::READ_WRITE);

// Set up frame
accessor->push_byte(0xAA);
accessor->push_byte(0xBB);
accessor->set_frame_to_top();  // FP now protects 0xBB

// Push data in new frame
accessor->push_byte(0xCC);

// Can pop data from current frame
byte_t cc = accessor->pop_byte();  // OK: 0xCC

// Frame is now empty relative to FP
assert(accessor->is_empty());  // SP == FP+1

// Cannot pop below frame
try {
    accessor->pop_byte();  // THROWS: can't pop past frame
} catch (runtime_error& e) {
    // Error: "Cannot pop below frame pointer"
}

// Can still access protected data via frame offset
byte_t protected = accessor->peek_byte_from_frame(0);  // OK: 0xBB
```

## Key Design Patterns

### Subroutine Calling Convention

1. **Caller**:
   - Push arguments
   - Save current FP
   - Push flag byte (0=no return, 1=has return)
   - Call `set_frame_to_top()`
   - Jump to subroutine

2. **Callee**:
   - Access flag at frame offset 0
   - Access arguments via `peek_from_base()`
   - Push local variables
   - Compute result

3. **Return**:
   - Check flag at frame offset 0
   - If returning: push value, pop it, flush, restore FP, pop flag, push value
   - If not: flush, restore FP, pop flag

### Frame Offset Semantics

```cpp
// After set_frame_to_top(), FP points to last pushed item
accessor->push_byte(0x10);
accessor->push_byte(0x20);
accessor->push_byte(0x30);
accessor->set_frame_to_top();  // FP = 2 (points to 0x30)

// Frame offset 0 accesses where FP points
byte_t at_fp = accessor->peek_byte_from_frame(0);  // 0x30

// Push more data
accessor->push_byte(0x40);  // Now at address 3 (frame offset 1)

// Access via frame offset
byte_t local = accessor->peek_byte_from_frame(1);  // 0x40
```

## Error Handling

Common exceptions:
- `runtime_error("Stack overflow")` - Exceeded capacity
- `runtime_error("Stack underflow")` - Pop from empty stack
- `runtime_error("Cannot pop below frame pointer")` - Attempted pop past frame
- `runtime_error("Frame pointer exceeds stack capacity")` - Invalid FP value
- `runtime_error("Frame pointer cannot be less than -1")` - Invalid FP value
- `runtime_error("Peek offset out of bounds")` - Invalid peek address

## Performance Considerations

- **Push/Pop**: O(1) - Direct memory write
- **Peek**: O(1) - Direct memory read
- **Flush**: O(1) - SP adjustment only
- **Frame Operations**: O(1) - Simple pointer manipulation

## Memory Layout

Stack uses direct 32-bit addressing within its own context:
- Each address maps directly to a byte
- No page calculation overhead
- Cache-friendly sequential access pattern
- Grows naturally upward in address space
