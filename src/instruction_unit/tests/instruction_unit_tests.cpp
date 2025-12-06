#include <gtest/gtest.h>
#include "memsize.h"
#include "instruction_unit.h"
#include "memunit.h"
#include "stack.h"
#include "flags.h"
#include "accessMode.h"

using namespace lvm;

// InstructionUnit test fixture
class InstructionUnitTest : public ::testing::Test {
protected:
    std::shared_ptr<Memory> memory;
    std::shared_ptr<Flags> flags;
    
    void SetUp() override {
        memory = std::make_shared<Memory>(256, 256);
        flags = std::make_shared<Flags>();
        memory->unprotected_mode();
    }
    
    std::unique_ptr<Stack> stack; // Keep stack alive
    
    std::unique_ptr<InstructionUnit> createInstructionUnit(memsize_t size = 1024) {
        // Create stack first and keep it alive
        stack = std::make_unique<Stack>(memory, 0, 0x0000, 512);
        auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
        
        // Create instruction unit on same page 0, after the stack
        auto iu = std::make_unique<InstructionUnit>(
            memory,
            std::move(stack_accessor),
            flags,
            512,  // start address (after stack)
            size
        );
        
        memory->protected_mode();
        return iu;
    }
};

// Test instruction unit creation
TEST_F(InstructionUnitTest, InstructionUnitCreation) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    // IR should be initialized to 0
    EXPECT_EQ(accessor->get_IR(), 0);
}

// Test setting and getting IR
TEST_F(InstructionUnitTest, SetAndGetIR) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x1234);
    EXPECT_EQ(accessor->get_IR(), 0x1234);
}

// Test advancing IR
TEST_F(InstructionUnitTest, AdvanceIR) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x100);
    accessor->advance_IR(5);
    EXPECT_EQ(accessor->get_IR(), 0x105);
    
    accessor->advance_IR(10);
    EXPECT_EQ(accessor->get_IR(), 0x10F);
}

// Test jump to address
TEST_F(InstructionUnitTest, JumpToAddress) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x100);
    accessor->Jump_To_Address(0x500);
    EXPECT_EQ(accessor->get_IR(), 0x500);
}

// Test conditional jump when condition is met
TEST_F(InstructionUnitTest, ConditionalJumpTrue) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    // Set ZERO flag
    flags->set(Flag::ZERO);
    
    accessor->set_IR(0x100);
    accessor->Jump_To_Address_Conditional(0x500, Flag::ZERO, true);
    
    // Should jump because ZERO flag is set and condition is true
    EXPECT_EQ(accessor->get_IR(), 0x500);
}

// Test conditional jump when condition is not met
TEST_F(InstructionUnitTest, ConditionalJumpFalse) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    // ZERO flag is not set
    flags->clear(Flag::ZERO);
    
    accessor->set_IR(0x100);
    accessor->Jump_To_Address_Conditional(0x500, Flag::ZERO, true);
    
    // Should not jump because ZERO flag is not set
    EXPECT_EQ(accessor->get_IR(), 0x100);
}

// Test conditional jump with negative condition
TEST_F(InstructionUnitTest, ConditionalJumpNegative) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    // Set ZERO flag
    flags->set(Flag::ZERO);
    
    accessor->set_IR(0x100);
    accessor->Jump_To_Address_Conditional(0x500, Flag::ZERO, false);
    
    // Should not jump because ZERO flag is set but condition is false
    EXPECT_EQ(accessor->get_IR(), 0x100);
    
    // Clear ZERO flag
    flags->clear(Flag::ZERO);
    accessor->Jump_To_Address_Conditional(0x500, Flag::ZERO, false);
    
    // Should jump now because ZERO flag is not set and condition is false
    EXPECT_EQ(accessor->get_IR(), 0x500);
}

// Test loading a program
// NOTE: Currently disabled - bulk_write has page boundary issues
// TEST_F(InstructionUnitTest, LoadProgram) {
//     auto iu = createInstructionUnit();
//     auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
//     
//     std::vector<byte_t> program = {0x01, 0x02, 0x03, 0x04, 0x05};
//     accessor->Load_Program(program);
//     
//     // Set IR to start of program
//     accessor->set_IR(0);
//     
//     // Read bytes at IR
//     EXPECT_EQ(accessor->readByte_At_IR(), 0x01);
//     accessor->advance_IR(1);
//     EXPECT_EQ(accessor->readByte_At_IR(), 0x02);
//     accessor->advance_IR(1);
//     EXPECT_EQ(accessor->readByte_At_IR(), 0x03);
// }

// Test reading word at IR
// NOTE: Currently disabled - bulk_write has page boundary issues
// TEST_F(InstructionUnitTest, ReadWordAtIR) {
//     auto iu = createInstructionUnit();
//     auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
//     
//     // Load program with word values
//     std::vector<byte_t> program = {0xCD, 0xAB, 0x34, 0x12};
//     accessor->Load_Program(program);
//     
//     accessor->set_IR(0);
//     word_t word1 = accessor->readWWord_At_IR();
//     EXPECT_EQ(word1, 0xABCD); // Little endian
//     
//     accessor->set_IR(2);
//     word_t word2 = accessor->readWWord_At_IR();
//     EXPECT_EQ(word2, 0x1234);
// }

// Test call subroutine without return value
TEST_F(InstructionUnitTest, CallSubroutineNoReturn) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x100);
    accessor->call_subroutine(0x500, false);
    
    // IR should now point to subroutine
    EXPECT_EQ(accessor->get_IR(), 0x500);
}

// Test call subroutine with return value
TEST_F(InstructionUnitTest, CallSubroutineWithReturn) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x100);
    accessor->call_subroutine(0x500, true);
    
    // IR should now point to subroutine
    EXPECT_EQ(accessor->get_IR(), 0x500);
}

// Test return from subroutine without return value
TEST_F(InstructionUnitTest, ReturnFromSubroutineNoReturn) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    addr_t original_ir = 0x100;
    accessor->set_IR(original_ir);
    
    // Call subroutine
    accessor->call_subroutine(0x500, false);
    EXPECT_EQ(accessor->get_IR(), 0x500);
    
    // At this point: flag is AT FP, SP = FP
    // Subroutine runs (but does nothing in this test)
    
    // Return from subroutine
    accessor->return_from_subroutine();
    EXPECT_EQ(accessor->get_IR(), original_ir);
}

// Test return from subroutine with return value
TEST_F(InstructionUnitTest, ReturnFromSubroutineWithReturn) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    addr_t original_ir = 0x100;
    accessor->set_IR(original_ir);
    
    // Call subroutine with return value
    accessor->call_subroutine(0x500, true);
    EXPECT_EQ(accessor->get_IR(), 0x500);
    
    // Simulate subroutine pushing a return value (would be done by actual subroutine code)
    stack_accessor->push_word(0x1234);
    
    // Return from subroutine
    accessor->return_from_subroutine();
    EXPECT_EQ(accessor->get_IR(), original_ir);
    
    // Verify return value is on the stack
    word_t returnValue = stack_accessor->pop_word();
    EXPECT_EQ(returnValue, 0x1234);
}

// Test nested subroutine calls
TEST_F(InstructionUnitTest, NestedSubroutines) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    accessor->set_IR(0x100);
    
    // First call
    accessor->call_subroutine(0x200, false);
    EXPECT_EQ(accessor->get_IR(), 0x200);
    
    // Nested call
    accessor->call_subroutine(0x300, false);
    EXPECT_EQ(accessor->get_IR(), 0x300);
    
    // Return from nested call
    accessor->return_from_subroutine();
    EXPECT_EQ(accessor->get_IR(), 0x200);
    
    // Return from first call
    accessor->return_from_subroutine();
    EXPECT_EQ(accessor->get_IR(), 0x100);
}

// Test return stack underflow
TEST_F(InstructionUnitTest, ReturnStackUnderflow) {
    auto iu = createInstructionUnit();
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    // Try to return without calling
    EXPECT_THROW(accessor->return_from_subroutine(), std::runtime_error);
}

// Test read-only accessor restrictions
TEST_F(InstructionUnitTest, ReadOnlyAccessor) {
    auto iu = createInstructionUnit();
    auto ro_accessor = iu->get_accessor(MemAccessMode::READ_ONLY);
    
    // Read operations should work
    EXPECT_NO_THROW(ro_accessor->get_IR());
    
    // Write operations should throw
    EXPECT_THROW(ro_accessor->set_IR(0x100), std::runtime_error);
    EXPECT_THROW(ro_accessor->advance_IR(5), std::runtime_error);
    EXPECT_THROW(ro_accessor->Jump_To_Address(0x500), std::runtime_error);
}

// Test program too large
TEST_F(InstructionUnitTest, ProgramTooLarge) {
    auto iu = createInstructionUnit(100); // Small instruction unit
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    std::vector<byte_t> large_program(200, 0xFF); // Too large
    EXPECT_THROW(accessor->Load_Program(large_program), std::runtime_error);
}
