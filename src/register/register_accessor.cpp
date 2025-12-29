#include "register.h" 

using namespace lvm;

RegisterAccessor::RegisterAccessor(Register& reg) : register_ref(reg) {}
RegisterAccessor::~RegisterAccessor() {}
word_t RegisterAccessor::get_value() const {
    return register_ref.get_value();
}
void RegisterAccessor::set_value(word_t value) {
    register_ref.set_value(value);
}
void RegisterAccessor::clear() {
    register_ref.clear();
}       
void RegisterAccessor::set_high_byte(byte_t high) {
    register_ref.set_high_byte(high);
}
void RegisterAccessor::set_low_byte(byte_t low) {
    register_ref.set_low_byte(low);
}
byte_t RegisterAccessor::get_high_byte() const {
    return register_ref.get_high_byte();
}
byte_t RegisterAccessor::get_low_byte() const {
    return register_ref.get_low_byte();
}
bool RegisterAccessor::has_flags() const {
    return register_ref.has_flags();
}
bool RegisterAccessor::set_flag(Flag flag) {    
    return register_ref.set_flag(flag);
}
bool RegisterAccessor::clear_flag(Flag flag) {
    return register_ref.clear_flag(flag);
}
bool RegisterAccessor::is_flag_set(Flag flag) const {
    return register_ref.is_flag_set(flag);
}
void RegisterAccessor::inc() {
    register_ref.inc();
}
void RegisterAccessor::dec() {
    register_ref.dec();
}   



