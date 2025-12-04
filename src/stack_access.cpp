#include "stack.h"
#include "memunit.h"
#include "mem_access.h"
#include "errors.h"
#include "accessMode.h"
using namespace lvm;

Stack_Accessor::Stack_Accessor(Stack* stack, MemAccessMode access_mode)
    : stack_ref(stack), mode(access_mode) {

    }

Stack_Accessor::~Stack_Accessor() {}
// READ Access Methods
byte_t Stack_Accessor::peek_byte() const {
    return stack_ref->peek();
}   

word_t Stack_Accessor::peek_word() const {
    return stack_ref->peek_word();
}

byte_t Stack_Accessor::peek_byte_from_base(page_offset_t offset) const {
    return stack_ref->peek_byte_from_base(offset);
}

word_t Stack_Accessor::peek_word_from_base(page_offset_t offset) const {
    return stack_ref->peek_word_from_base(offset);
}

byte_t Stack_Accessor::peek_byte_from_frame(page_offset_t offset) const {
    return stack_ref->peek_byte_from_frame(offset);
}

word_t Stack_Accessor::peek_word_from_frame(page_offset_t offset) const {
    return stack_ref->peek_word_from_frame(offset);
}

bool Stack_Accessor::is_empty() const {
    return stack_ref->is_empty();
}

bool Stack_Accessor::is_full() const {
    return stack_ref->is_full();
}

memsize_t Stack_Accessor::get_size() const {
    return stack_ref->get_size();
}

void Stack_Accessor::flush() {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to flush stack in READ_ONLY mode");
    }
    stack_ref->flush();
}

void Stack_Accessor::set_frame_to_top() {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to set frame to top in READ_ONLY mode");
    }
    stack_ref->set_frame_to_top();
}

void Stack_Accessor::set_frame_register(word_t value) {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to set frame register in READ_ONLY mode");
    }
    stack_ref->set_frame_register(value);
}

addr_t Stack_Accessor::get_frame_register() const {
    return stack_ref->get_frame_register();
}

void Stack_Accessor::push(byte_t value) {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to push to stack in READ_ONLY mode");
    }
    stack_ref->push(value);
}

byte_t Stack_Accessor::pop() {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to pop from stack in READ_ONLY mode");
    }
    return stack_ref->pop();
}

void Stack_Accessor::push_word(word_t value) {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to push_word to stack in READ_ONLY mode");
    }
    stack_ref->push_word(value);
}

word_t Stack_Accessor::pop_word() {
    if(mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to pop_word from stack in READ_ONLY mode");
    }
    return stack_ref->pop_word();
}

