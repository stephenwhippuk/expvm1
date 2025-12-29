#pragma once
#include "memsize.h"
#include "flags.h"
#include <memory>
namespace lvm { 

    class Register;
    
    class RegisterAccessor{
    public:
        ~RegisterAccessor();
        word_t get_value() const ;
        void set_value(word_t value);
        void clear();
        void set_high_byte(byte_t high);
        void set_low_byte(byte_t low);
        byte_t get_high_byte() const;
        byte_t get_low_byte() const;

        bool has_flags() const;
        bool set_flag(Flag flag);
        bool clear_flag(Flag flag);
        bool is_flag_set(Flag flag) const;
        
        void inc();
        void dec();
        private:
            friend class Register;
            RegisterAccessor(Register& reg);
            Register& register_ref;
    };

    class Register {
    public:
        Register();
        Register(std::shared_ptr<Flags> flags_ptr);
        Register(const Register& other);
        ~Register();
        std::unique_ptr<RegisterAccessor> get_accessor();

    private:
        friend class RegisterAccessor;
        Flags& flags;
        word_t value;
        bool flags_provided = false;

        void set_value(word_t value);
        word_t get_value() const;
        void clear();
        void set_high_byte(byte_t high);
        void set_low_byte(byte_t low);
        byte_t get_high_byte() const;
        byte_t get_low_byte() const;

        bool has_flags() const;
        bool set_flag(Flag flag);
        bool clear_flag(Flag flag);
        bool is_flag_set(Flag flag) const;
        
        void inc();
        void dec();
    };
}