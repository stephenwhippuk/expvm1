#pragma once

namespace lvm {
    class BasicIO;

    class BasicIOAccessor{
    public:
        ~BasicIOAccessor(); 

        void write_string_from_stack();
        void write_line_from_stack();
        void read_line_onto_stack();
        void debug_print_word();

    private:
        friend class BasicIO;
        BasicIOAccessor(BasicIO& basic_io); 
        BasicIO& basic_io_ref;

};

} // namespace lvm