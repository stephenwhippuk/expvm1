#include <iostream>
#include "errors.h"
#include "vm.h"
int main(int argc, char** argv) {
    if(argc < 3){
        std::cerr << "Usage: " << argv[0] << " <program file>" << " <load address>" << std::endl;
        return 1;
    }
    try {
        lvm::vm virtual_machine(16, 63356, 512); // 16 pages, 256 bytes per page, 512 bytes stack
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