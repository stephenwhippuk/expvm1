#include "stack.h"
#include "memunit.h"
#include "mem_access.h"
#include "errors.h"
#include "accessMode.h"
using namespace lvm;

Stack::Stack(std::shared_ptr<Memory> memory, page_t page, addr_t base_address, memsize_t stack_size)
    : stack_size(stack_size), sp(stack_size) // Initialize SP to stack size (empty stack)
{
    // Reserve space in the memory for the stack
    mem_accessor = memory->reserve_space(page, base_address, stack_size, MemAccessMode::READ_WRITE);
    // Initialize frame pointer and base pointer registers
    FP = std::make_shared<Register>();
    BP = std::make_shared<Register>();
    FP->set_value(stack_size);
    BP->set_value(stack_size);
    bottom_address = base_address;
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
    if (sp > stack_size - 2) {
        throw lvm::runtime_error("Stack underflow on peek_word");
    }
    word_t low = mem_accessor->read_byte(sp);
    word_t high = mem_accessor->read_byte(sp + 1);
    return (high << 8) | low;
}

byte_t Stack::peek_byte_from_base(addr_t offset) const {
    if (offset >= stack_size) {
        throw lvm::runtime_error("Peek byte from base offset out of bounds");
    }
    return mem_accessor->read_byte(stack_size - 1 - offset);
}

word_t Stack::peek_word_from_base(addr_t offset) const {
    if (offset + 1 >= stack_size) {
        throw lvm::runtime_error("Peek word from base offset out of bounds");
    }
    word_t low = mem_accessor->read_byte(stack_size - 1 - offset);
    word_t high = mem_accessor->read_byte(stack_size - 1 - (offset + 1));
    return (high << 8) | low;
}

byte_t Stack::peek_byte_from_frame(addr_t offset) const {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set for peek_byte_from_frame");
    }
    addr_t frame_base = FP->get_value();
    if (offset >= stack_size || frame_base + offset >= stack_size) {
        throw lvm::runtime_error("Peek byte from frame offset out of bounds");
    }
    return mem_accessor->read_byte(frame_base + offset);
}

word_t Stack::peek_word_from_frame(addr_t offset) const {
    if (!FP) {
        throw lvm::runtime_error("Frame pointer not set for peek_word_from_frame");
    }
    addr_t frame_base = FP->get_value();
    if (offset + 1 >= stack_size || frame_base + offset + 1 >= stack_size) {
        throw lvm::runtime_error("Peek word from frame offset out of bounds");
    }
    word_t low = mem_accessor->read_byte(frame_base + offset);
    word_t high = mem_accessor->read_byte(frame_base + offset + 1);
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
    stack_size = BP->get_value() - get_frame_register();
    sp = FP->get_value();
}

std::unique_ptr<Stack_Accessor> Stack::get_accessor(MemAccessMode mode) {
    return std::unique_ptr<Stack_Accessor>(new Stack_Accessor(this, mode));
}


