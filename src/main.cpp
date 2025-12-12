#include <iostream>
#include "lvm.h"

int main(int argc, char** argv) {
    if(argc < 3){
        std::cerr << "Usage: " << argv[0] << " <program file>" << " <load address>" << std::endl;
        return 1;
    }
    try {
        lvm::vm virtual_machine(1024, 65536, 32768); // 1KB stack, 64KB code space, 32KB data space
        virtual_machine.load_program(argv[1], argv[2] ? static_cast<lvm::addr_t>(std::stoi(argv[2])) : 0x0000);
        virtual_machine.run();
    } catch (const lvm::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}