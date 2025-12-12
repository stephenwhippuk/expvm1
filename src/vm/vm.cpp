#include "vm.h"
#include "binary_loader.h"

using namespace lvm;

vm::vm(addr32_t stack_capacity, addr32_t code_capacity, addr32_t data_capacity)
    : vmem_unit(std::make_shared<VMemUnit>())
{
    // Create CPU first (creates contexts and flags)
    cpu_instance = std::make_shared<Cpu>(vmem_unit, stack_capacity, code_capacity);
    
    // Get flags from CPU to share with instruction unit
    flags = cpu_instance->get_flags();
    
    // Create stack in UNPROTECTED mode
    stack = std::make_shared<Stack>(vmem_unit, stack_capacity);
    
    // Create BasicIO with stack and memory
    basic_io = std::make_shared<BasicIO>(vmem_unit, stack);
    
    // Create code context for instruction unit
    code_context_id_ = vmem_unit->create_context(code_capacity);
    
    // Create data context for program data
    data_context_id_ = vmem_unit->create_context(data_capacity);
    
    // Create instruction unit in UNPROTECTED mode with stack interface
    // Use CPU's flags to ensure registers and instruction unit share state
    instruction_unit = std::make_shared<InstructionUnit>(vmem_unit, code_context_id_, *stack, flags, basic_io);
    
    // Inject dependencies into CPU
    cpu_instance->set_stack(stack);
    cpu_instance->set_instruction_unit(instruction_unit);
    
    // Initialize CPU
    cpu_instance->initialize();
}

vm::~vm() {
    // Destructor
}

void vm::load_program(char* fileName, addr_t load_address) {
    // Load program from binary file
    BinaryLoader loader;
    
    try {
        // Parse binary file
        BinaryProgram program = loader.load_file(fileName);
        
        // Load data segment into data context if present
        if (!program.data_segment.empty()) {
            auto data_ctx = vmem_unit->get_context(data_context_id_);
            auto data_accessor = data_ctx->create_paged_accessor(MemAccessMode::READ_WRITE);
            
            // Write data segment starting at load_address
            addr32_t current_addr = load_address;
            for (byte_t byte : program.data_segment) {
                page_t page = current_addr / 256;
                addr_t offset = current_addr % 256;
                data_accessor->set_page(page);
                data_accessor->write_byte(offset, byte);
                current_addr++;
            }
        }

        // Load code segment into CPU
        cpu_instance->load_program(program.code_segment);
        
    } catch (const runtime_error& e) {
        // Re-throw with context
        throw runtime_error("Failed to load program '" + std::string(fileName) + "': " + e.what());
    }
}

void vm::run() {
    cpu_instance->run();
}

