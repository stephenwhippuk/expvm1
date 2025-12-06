#include "stack.h"
#include "memunit.h"
#include "mem_access.h"
#include "errors.h"
#include "accessMode.h"
using namespace lvm;

Stack::Stack(std::shared_ptr<Memory> memory, page_t page, addr_t base_address, memsize_t size)
    : stack_size(size), sp(size) // Initialize SP to stack size (empty stack)
{
    // Reserve space in the memory for the stack
    mem_accessor = memory->reserve_space(page, base_address, size, MemAccessMode::READ_WRITE);
    // Initialize frame pointer and base pointer registers
    FP = std::make_shared<Register>();
    BP = std::make_shared<Register>();
    // BP points to base (highest address in stack = stack_size - 1)
    // FP and SP start at stack_size (empty stack, one past the top)
    FP->set_value(size);
    BP->set_value(size - 1);
    bottom_address = base_address;
    stack_size = 0; // Initially stack is empty
}

Stack::~Stack() {
    // MemoryAccessor will automatically release memory when destroyed
}

void Stack::push(byte_t value) {
    if (is_full()) {
        throw lvm::runtime_error("Stack overflow on push");
    }
    sp--;
    mem_accessor->write_byte(sp, value);
    stack_size++;
}

byte_t Stack::pop() {
    if (is_empty()) {
        throw lvm::runtime_error("Stack underflow on pop");
    }
    byte_t value = mem_accessor->read_byte(sp);
    sp++;
    stack_size--;
    return value;
}

byte_t Stack::peek() const {
    if (is_empty()) {
        throw lvm::runtime_error("Stack underflow on peek");
    }
    return mem_accessor->read_byte(sp);
}

void Stack::push_word(word_t value) {
    if (sp < bottom_address + 2) {
        throw lvm::runtime_error("Stack overflow on push_word");
    }
    sp -= 2;
    stack_size += 2;
    mem_accessor->write_byte(sp, static_cast<byte_t>(value & 0xFF));         // Low byte
    mem_accessor->write_byte(sp + 1, static_cast<byte_t>((value >> 8) & 0xFF)); // High byte
}

word_t Stack::pop_word() {
    if (sp > FP->get_value() - 2) {
        throw lvm::runtime_error("Stack underflow on pop_word");
    }
    word_t low = mem_accessor->read_byte(sp);
    word_t high = mem_accessor->read_byte(sp + 1);
    sp += 2;
    return (high << 8) | low;
}

word_t Stack::peek_word() const {
    if (sp > FP->get_value() - 2) {
        throw lvm::runtime_error("Stack underflow on peek_word");
    }
    word_t low = mem_accessor->read_byte(sp);
    word_t high = mem_accessor->read_byte(sp + 1);
    return (high << 8) | low;
}

byte_t Stack::peek_byte_from_base(page_offset_t offset) const {
    addr_t base = BP->get_value();
    // Positive offset moves toward stack top (lower addresses)
    // Negative offset moves deeper into stack (higher addresses)
    int32_t address = base - offset;
    if (address < 0 || address >= bottom_address) {
        throw lvm::runtime_error("Peek byte from base offset out of bounds");
    }
    return mem_accessor->read_byte(static_cast<addr_t>(address));
}

word_t Stack::peek_word_from_base(page_offset_t offset) const {
    addr_t base = BP->get_value();
    // Offset 0 should give us the first word pushed
    // First word: low byte at BP-1, high byte at BP
    // Second word: low byte at BP-3, high byte at BP-2
    // So for offset N: low byte at BP - (2*N + 1), high byte at BP - 2*N
    int32_t high_address = base - (2 * offset);
    int32_t low_address = high_address - 1;
    if (low_address < 0 || low_address >= stack_size || high_address < 0 || high_address >= stack_size) {
        throw lvm::runtime_error("Peek word from base offset out of bounds");
    }
    word_t low = mem_accessor->read_byte(static_cast<addr_t>(low_address));
    word_t high = mem_accessor->read_byte(static_cast<addr_t>(high_address));
    return (high << 8) | low;
}

byte_t Stack::peek_byte_from_frame(page_offset_t offset) const {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set for peek_byte_from_frame");
    }
    addr_t frame_base = FP->get_value();
    // Positive offset moves toward stack top (lower addresses)
    // Negative offset moves deeper into stack (higher addresses)
    page_offset_t address = frame_base - offset;
    if (address < 0 || address >= stack_size) {
        throw lvm::runtime_error("Peek byte from frame offset out of bounds");
    }
    return mem_accessor->read_byte(static_cast<addr_t>(address));
}

word_t Stack::peek_word_from_frame(page_offset_t offset) const {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set for peek_word_from_frame");
    }
    addr_t frame_base = FP->get_value();
    int32_t address = frame_base - offset;
    if (address < 1 || address >= stack_size) {
        throw lvm::runtime_error("Peek word from frame offset out of bounds");
    }
    // Word is stored with low byte at address, high byte at address-1
    word_t low = mem_accessor->read_byte(static_cast<addr_t>(address));
    word_t high = mem_accessor->read_byte(static_cast<addr_t>(address - 1));
    return (high << 8) | low;
}

bool Stack::is_empty() const {
    return sp == get_frame_register();
}

bool Stack::is_full() const {
    return sp == bottom_address;
}

memsize_t Stack::get_size() const {
    return stack_size;
}

addr_t Stack::get_frame_register() const {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set");
    }
    return FP->get_value();
}

void Stack::set_frame_register(word_t value) {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set");
    }
    FP->set_value(value);
}

void Stack::set_frame_to_top() {
    set_frame_register(sp);
}   
void Stack::flush() {
    sp = get_frame_register();
    stack_size = BP->get_value() - get_frame_register();
}

std::unique_ptr<Stack_Accessor> Stack::get_accessor(MemAccessMode mode) {
    return std::unique_ptr<Stack_Accessor>(new Stack_Accessor(this, mode));
}


