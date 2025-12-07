#include "stack.h"
#include "errors.h"
#include "helpers.h"
#include <stdexcept>

using namespace lvm;

// Stack implementation

Stack::Stack(std::shared_ptr<IVMemUnit> vmem_unit, addr32_t capacity)
    : vmem_unit_(std::move(vmem_unit)),
      capacity_(capacity),
      sp_(0),
      fp_(-1) {
    
    if (vmem_unit_->is_protected()) {
        throw lvm::runtime_error("Stack can only be created in UNPROTECTED mode");
    }
    
    // Create a dedicated context for the stack
    context_id_ = vmem_unit_->create_context(capacity);
}

std::unique_ptr<StackAccessor> Stack::get_accessor(MemAccessMode mode) {
    if (!vmem_unit_->is_protected()) {
        throw lvm::runtime_error("Stack accessor can only be created in PROTECTED mode");
    }
    
    return std::unique_ptr<StackAccessor>(new StackAccessor(this, mode));
}

void Stack::push_byte(byte_t value) {
    if (is_full()) {
        throw lvm::runtime_error("Stack overflow");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    accessor->write_byte(sp_, value);
    sp_++;
}

byte_t Stack::pop_byte() {
    if (is_empty()) {
        throw lvm::runtime_error("Stack underflow");
    }
    
    sp_--;
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_byte(sp_);
}

byte_t Stack::peek_byte() const {
    if (is_empty()) {
        throw lvm::runtime_error("Stack is empty");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_byte(sp_ - 1);
}

void Stack::push_word(word_t value) {
    if (sp_ + sizeof(word_t) > capacity_) {
        throw lvm::runtime_error("Stack overflow");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    accessor->write_word(sp_, value);
    sp_ += 2;
}

word_t Stack::pop_word() {
    if (sp_ < static_cast<addr32_t>(fp_ + 1 + 2)) {
        throw lvm::runtime_error("Stack underflow");
    }
    
    sp_ -= 2;
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_word(sp_);
}

word_t Stack::peek_word() const {
    if (sp_ < static_cast<addr32_t>(fp_ + 1 + 2)) {
        throw lvm::runtime_error("Stack is empty");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_word(sp_ - 2);
}

byte_t Stack::peek_byte_from_base(addr32_t offset) const {
    if (offset >= sp_) {
        throw lvm::runtime_error("Offset beyond stack pointer");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_byte(offset);
}

word_t Stack::peek_word_from_base(addr32_t offset) const {
    if (offset + 2 > sp_) {
        throw lvm::runtime_error("Offset exceeds stack size");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_word(offset);
}

byte_t Stack::peek_byte_from_frame(addr32_t offset) const {
    // FP sits at -1 relative to frame means FP points one position before frame data
    // But frame offset 0 should access the first item after FP, which is at FP+1
    // Wait - after set_frame_to_top, if we pushed flag at pos 0, sp=1, fp=0
    // The flag IS the first frame item, at position 0 where FP points
    // So frame offset 0 = FP + 0, not FP + 1
    addr32_t absolute_offset = static_cast<addr32_t>(fp_) + offset;
    
    if (absolute_offset >= sp_ || fp_ < 0) {
        throw lvm::runtime_error("Offset exceeds stack size");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_byte(absolute_offset);
}

word_t Stack::peek_word_from_frame(addr32_t offset) const {
    // Frame offset 0 accesses where FP points (first protected item)
    addr32_t absolute_offset = static_cast<addr32_t>(fp_) + offset;
    
    if (absolute_offset + 2 > sp_ || fp_ < 0) {
        throw lvm::runtime_error("Offset exceeds stack size");
    }
    
    auto ctx = vmem_unit_->get_context(context_id_);
    auto accessor = ctx->create_stack_accessor();
    return accessor->read_word(absolute_offset);
}

bool Stack::is_empty() const {
    // fp_ sits at -1 relative to frame, so fp_+1 is the first frame position
    // Stack is empty when sp_ is at the first frame position
    return sp_ == static_cast<addr32_t>(fp_ + 1);
}

bool Stack::is_full() const {
    return sp_ >= capacity_;
}

void Stack::set_frame_pointer(int32_t value) {
    if (value >= static_cast<int32_t>(capacity_)) {
        throw lvm::runtime_error("Frame pointer exceeds stack capacity");
    }
    if (value < -1) {
        throw lvm::runtime_error("Frame pointer cannot be less than -1");
    }
    fp_ = value;
}

void Stack::set_frame_to_top() {
    // FP sits at -1 relative to the frame, so if SP is at position N,
    // setting FP to SP-1 means the frame starts at position SP
    // Since SP points to the next free position, we set FP = SP - 1
    // so that FP+1 (first frame position) equals SP
    fp_ = static_cast<int32_t>(sp_) - 1;
}

void Stack::flush() {
    // Flush only the current frame - reset sp_ to the start of the frame
    // Frame starts at fp_ + 1, so we set sp_ to that position
    sp_ = static_cast<addr32_t>(fp_ + 1);
}

// Stack_Accessor implementation

StackAccessor::StackAccessor(Stack* stack, MemAccessMode access_mode)
    : stack_ref(stack), mode(access_mode) {
}

byte_t StackAccessor::peek_byte() const {
    return stack_ref->peek_byte();
}

word_t StackAccessor::peek_word() const {
    return stack_ref->peek_word();
}

byte_t StackAccessor::peek_byte_from_base(addr32_t offset) const {
    return stack_ref->peek_byte_from_base(offset);
}

word_t StackAccessor::peek_word_from_base(addr32_t offset) const {
    return stack_ref->peek_word_from_base(offset);
}

byte_t StackAccessor::peek_byte_from_frame(addr32_t offset) const {
    return stack_ref->peek_byte_from_frame(offset);
}

word_t StackAccessor::peek_word_from_frame(addr32_t offset) const {
    return stack_ref->peek_word_from_frame(offset);
}

bool StackAccessor::is_empty() const {
    return stack_ref->is_empty();
}

bool StackAccessor::is_full() const {
    return stack_ref->is_full();
}

addr32_t StackAccessor::get_size() const {
    return stack_ref->get_size();
}

addr32_t StackAccessor::get_capacity() const {
    return stack_ref->get_capacity();
}

addr32_t StackAccessor::get_sp() const {
    return stack_ref->get_sp();
}

int32_t StackAccessor::get_fp() const {
    return stack_ref->get_fp();
}

void StackAccessor::push_byte(byte_t value) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to push to READ_ONLY stack");
    }
    stack_ref->push_byte(value);
}

byte_t StackAccessor::pop_byte() {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to pop from READ_ONLY stack");
    }
    return stack_ref->pop_byte();
}

void StackAccessor::push_word(word_t value) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to push to READ_ONLY stack");
    }
    stack_ref->push_word(value);
}

word_t StackAccessor::pop_word() {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to pop from READ_ONLY stack");
    }
    return stack_ref->pop_word();
}

void StackAccessor::set_frame_pointer(int32_t value) {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to set frame pointer on READ_ONLY stack");
    }
    stack_ref->set_frame_pointer(value);
}

void StackAccessor::set_frame_to_top() {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to set frame pointer on READ_ONLY stack");
    }
    stack_ref->set_frame_to_top();
}

void StackAccessor::flush() {
    if (mode != MemAccessMode::READ_WRITE) {
        throw lvm::runtime_error("Attempt to flush READ_ONLY stack");
    }
    stack_ref->flush();
}
