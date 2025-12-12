#include "basic_io.h"
#include <iostream>
#include "basic_io_accessor.h"
#include "stack.h"
using namespace lvm;

BasicIO::BasicIO(std::shared_ptr<IVMemUnit> memUnit, std::shared_ptr<IStack> stack)   
    : memUnit(std::move(memUnit)), stack(std::move(stack))
{
    // TODO: Initialize I/O subsystem
}

BasicIO::~BasicIO() = default;

// TODO: Implement methods

void BasicIO::write_string_from_stack() {
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    uint16_t count = accessor->pop_word();
    std::string output;
    for (uint16_t i = 0; i < count; ++i) {
        byte_t ch = accessor->pop_byte();
        output += static_cast<char>(ch);
    }
    // Output the string
    std::cout << output;
}

void BasicIO::write_line_from_stack() {
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    uint16_t count = accessor->pop_word();
    std::string output;
    for (uint16_t i = 0; i < count; ++i) {
        byte_t ch = accessor->pop_byte();
        output += static_cast<char>(ch);
            
    }
    // Output the string with newline
    std::cout << output << std::endl;
}


void BasicIO::read_line_onto_stack() {
    auto accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    auto maxLength = accessor->pop_word();
    std::string input;
    std::getline(std::cin, input);
    uint16_t count = static_cast<uint16_t>(input.length());
    if (count > maxLength) {
        count = maxLength; // Truncate if input exceeds max length
    }
    // Push characters onto stack in reverse order
    for (int i = count - 1; i >= 0; --i) {
        accessor->push_byte(static_cast<byte_t>(input[i]));
    }
    accessor->push_word(count);
}

std::unique_ptr<BasicIOAccessor> BasicIO::get_accessor() {
    return std::unique_ptr<BasicIOAccessor>(new BasicIOAccessor(*this));
}


BasicIOAccessor::BasicIOAccessor(BasicIO& basic_io)
    : basic_io_ref(basic_io) {}


BasicIOAccessor::~BasicIOAccessor() = default;

void BasicIOAccessor::write_string_from_stack() {
    basic_io_ref.write_string_from_stack();
}

void BasicIOAccessor::write_line_from_stack() {
    basic_io_ref.write_line_from_stack();
}

void BasicIOAccessor::read_line_onto_stack() {
    basic_io_ref.read_line_onto_stack();
}   