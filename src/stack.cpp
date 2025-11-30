#include "stack.h"
#include "memunit.h"
#include "mem_access.h"
#include "errors.h"
#include "accessMode.h"
using namespace lvm;

Stack::Stack(Memory& memory, page_t page, addr_t base_address, memsize_t stack_size)
    : stack_size(stack_size), sp(stack_size) // Initialize SP to stack size (empty stack)
{
    // Reserve space in the memory for the stack
    mem_accessor = memory.reserve_space(page, base_address, stack_size, MemAccessMode::READ_WRITE);
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
}

byte_t Stack::pop() {
    if (is_empty()) {
        throw lvm::runtime_error("Stack underflow on pop");
    }
    byte_t value = mem_accessor->read_byte(sp);
    sp++;
    return value;
}

byte_t Stack::peek() const {
    if (is_empty()) {
        throw lvm::runtime_error("Stack underflow on peek");
    }
    return mem_accessor->read_byte(sp);
}

void Stack::push_word(word_t value) {
    if (sp < 2) {
        throw lvm::runtime_error("Stack overflow on push_word");
    }
    sp -= 2;
    mem_accessor->write_byte(sp, static_cast<byte_t>(value & 0xFF));         // Low byte
    mem_accessor->write_byte(sp + 1, static_cast<byte_t>((value >> 8) & 0xFF)); // High byte
}

word_t Stack::pop_word() {
    if (sp > stack_size - 2) {
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

byte_t Stack::peek_at_offset(addr_t offset) const {
    if (offset >= stack_size) {
        throw lvm::runtime_error("Peek offset out of bounds");
    }
    // Stack grows downward, so conceptual offset from base maps to negative offset from end
    addr_t actual_address = stack_size - 1 - offset;
    return mem_accessor->read_byte(actual_address);
}

word_t Stack::peek_word_at_offset(addr_t offset) const {
    if (offset + 1 >= stack_size) {
        throw lvm::runtime_error("Peek word offset out of bounds");
    }
    // Stack grows downward, so conceptual offset from base maps to negative offset from end
    addr_t actual_address = stack_size - 1 - offset;
    word_t low = mem_accessor->read_byte(actual_address);
    word_t high = mem_accessor->read_byte(actual_address - 1);
    return (high << 8) | low;
}

bool Stack::is_empty() const {
    return sp == stack_size;
}

bool Stack::is_full() const {
    return sp == 0;
}

memsize_t Stack::get_size() const {
    return stack_size;
}