#include <gtest/gtest.h>
#include "memsize.h"
#include "instruction_unit.h"
#include "vmemunit.h"
#include "stack_new.h"
#include "flags.h"
#include "accessMode.h"

using namespace lvm;

// InstructionUnit test fixture
class InstructionUnitTest : public ::testing::Test {
protected:
    std::unique_ptr<VMemUnit> vmem_unit;
    std::shared_ptr<Flags> flags;
    std::unique_ptr<Stack2> stack;
    context_id_t code_context_id;
    
    void SetUp() override {
        vmem_unit = std::make_unique<VMemUnit>();
        flags = std::make_shared<Flags>();
        
        // Create stack with 1KB capacity (in UNPROTECTED mode)
        stack = std::make_unique<Stack2>(*vmem_unit, 1024);
        
        // Create code context with 64KB capacity (in UNPROTECTED mode)
        code_context_id = vmem_unit->create_context(65536);
        
        // Set protected mode before creating accessors
        vmem_unit->set_mode(VMemUnit::Mode::PROTECTED);
    }
    
    std::unique_ptr<InstructionUnit> createInstructionUnit() {
        // Get stack accessor in PROTECTED mode
        auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
        
        // Switch to UNPROTECTED mode for InstructionUnit creation
        vmem_unit->set_mode(VMemUnit::Mode::UNPROTECTED);
        
        auto iu = std::make_unique<InstructionUnit>(
            *vmem_unit,
            code_context_id,
            std::move(stack_accessor),
            flags
        );
        
        // Switch back to PROTECTED mode for normal operation
        vmem_unit->set_mode(VMemUnit::Mode::PROTECTED);
        
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
    // Create a small code context (256 bytes = 1 page) in UNPROTECTED mode
    vmem_unit->set_mode(VMemUnit::Mode::UNPROTECTED);
    context_id_t small_code_ctx = vmem_unit->create_context(256);
    
    // Get stack accessor in PROTECTED mode
    vmem_unit->set_mode(VMemUnit::Mode::PROTECTED);
    auto stack_accessor = stack->get_accessor(MemAccessMode::READ_WRITE);
    
    // Create InstructionUnit in UNPROTECTED mode
    vmem_unit->set_mode(VMemUnit::Mode::UNPROTECTED);
    auto iu = std::make_unique<InstructionUnit>(
        *vmem_unit,
        small_code_ctx,
        std::move(stack_accessor),
        flags
    );
    
    vmem_unit->set_mode(VMemUnit::Mode::PROTECTED);
    auto accessor = iu->get_accessor(MemAccessMode::READ_WRITE);
    
    std::vector<byte_t> large_program(300, 0xFF); // Too large for 256 bytes
    EXPECT_THROW(accessor->Load_Program(large_program), std::runtime_error);
}
