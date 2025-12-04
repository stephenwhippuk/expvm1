#include "flags.h"
using namespace lvm;

Flags::Flags() : flags(0) {}
Flags::~Flags() {}
void Flags::set(Flag flag) {
    flags |= static_cast<byte_t>(flag);
}
void Flags::clear(Flag flag) {
    flags &= ~static_cast<byte_t>(flag);
}
bool Flags::is_set(Flag flag) const {
    return (flags & static_cast<byte_t>(flag)) != 0;
}
void Flags::clear_all() {
    flags = 0;
}   
