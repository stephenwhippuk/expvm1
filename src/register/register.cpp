#include "register.h"

using namespace lvm;

Register::Register() : 
    value(0), 
    flags(*(new Flags())), 
    flags_provided(false) {}
    
Register::Register(std::shared_ptr<Flags> flags_ptr) : flags(*flags_ptr), value(0), flags_provided(true)  {
    if (!flags_ptr) {
        throw std::runtime_error("Register constructor received null flags pointer");
    }
}

Register::Register(const Register& other) 
    : flags(other.flags), value(other.value), flags_provided(other.flags_provided) {}

Register::~Register() {}    
void Register::set_value(word_t val) {
    value = val;
}
word_t Register::get_value() const {

    return value;
}
void Register::clear() {
    value = 0;
}
void Register::set_high_byte(byte_t high) {
    value = (value & 0x00FF) | (static_cast<word_t>(high) << 8);
}
void Register::set_low_byte(byte_t low) {   
    value = (value & 0xFF00) | static_cast<word_t>(low);
}
byte_t Register::get_high_byte() const {
    return static_cast<byte_t>((value >> 8) & 0xFF);
}
byte_t Register::get_low_byte() const {
    return static_cast<byte_t>(value & 0xFF);
}       
bool Register::has_flags() const {
    return flags_provided;
}
bool Register::set_flag(Flag flag) {
    flags.set(flag);
    return true;
}
bool Register::clear_flag(Flag flag) {
    flags.clear(flag);
    return true;
}
bool Register::is_flag_set(Flag flag) const {
    return flags.is_set(flag);
}

void Register::inc() {
    word_t old_value = value;
    value++;
    
    if (flags_provided) {
        // Clear flags
        flags.clear(Flag::ZERO);
        flags.clear(Flag::CARRY);
        flags.clear(Flag::SIGN);
        flags.clear(Flag::OVERFLOW);
        
        // Set ZERO flag
        if (value == 0) {
            flags.set(Flag::ZERO);
        }
        
        // Set SIGN flag
        if (value & 0x8000) {
            flags.set(Flag::SIGN);
        }
        
        // Set CARRY flag (increment wrapped from 0xFFFF to 0x0000)
        if (value < old_value) {
            flags.set(Flag::CARRY);
        }
        
        // Set OVERFLOW flag (signed overflow: 0x7FFF + 1 = 0x8000)
        if (old_value == 0x7FFF) {
            flags.set(Flag::OVERFLOW);
        }
    }
}

void Register::dec() {
    word_t old_value = value;
    value--;
    
    if (flags_provided) {
        // Clear flags
        flags.clear(Flag::ZERO);
        flags.clear(Flag::CARRY);
        flags.clear(Flag::SIGN);
        flags.clear(Flag::OVERFLOW);
        
        // Set ZERO flag
        if (value == 0) {
            flags.set(Flag::ZERO);
        }
        
        // Set SIGN flag
        if (value & 0x8000) {
            flags.set(Flag::SIGN);
        }
        
        // Set CARRY flag (decrement wrapped from 0x0000 to 0xFFFF)
        if (value > old_value) {
            flags.set(Flag::CARRY);
        }
        
        // Set OVERFLOW flag (signed underflow: 0x8000 - 1 = 0x7FFF)
        if (old_value == 0x8000) {
            flags.set(Flag::OVERFLOW);
        }
    }
}   
