#include <gtest/gtest.h>
#include "memsize.h"
#include "register.h"
#include "flags.h"
#include "alu.h"

using namespace lvm;

// Basic sanity test
TEST(SanityTest, BasicAssertion) {
    EXPECT_EQ(1, 1);
}

// Test register operations
TEST(RegisterTest, SetAndGetValue) {
    Register reg;
    reg.set_value(0x1234);
    EXPECT_EQ(reg.get_value(), 0x1234);
}

TEST(RegisterTest, HighAndLowBytes) {
    Register reg;
    reg.set_value(0xABCD);
    EXPECT_EQ(reg.get_high_byte(), 0xAB);
    EXPECT_EQ(reg.get_low_byte(), 0xCD);
}

TEST(RegisterTest, SetHighByte) {
    Register reg;
    reg.set_value(0x1234);
    reg.set_high_byte(0xAB);
    EXPECT_EQ(reg.get_value(), 0xAB34);
}

TEST(RegisterTest, SetLowByte) {
    Register reg;
    reg.set_value(0x1234);
    reg.set_low_byte(0xCD);
    EXPECT_EQ(reg.get_value(), 0x12CD);
}

// Test flags
TEST(FlagsTest, SetAndCheckFlags) {
    Flags flags;
    flags.set(Flag::ZERO);
    EXPECT_TRUE(flags.is_set(Flag::ZERO));
    EXPECT_FALSE(flags.is_set(Flag::CARRY));
}

TEST(FlagsTest, ClearFlags) {
    Flags flags;
    flags.set(Flag::CARRY);
    EXPECT_TRUE(flags.is_set(Flag::CARRY));
    flags.clear(Flag::CARRY);
    EXPECT_FALSE(flags.is_set(Flag::CARRY));
}

// Test register with flags
TEST(RegisterWithFlagsTest, IncrementSetsFlags) {
    auto flags = std::make_shared<Flags>();
    Register reg(flags);
    
    reg.set_value(0);
    reg.inc();
    EXPECT_EQ(reg.get_value(), 1);
    EXPECT_FALSE(reg.is_flag_set(Flag::ZERO));
    
    reg.set_value(0xFFFF);
    reg.inc();
    EXPECT_EQ(reg.get_value(), 0);
    EXPECT_TRUE(reg.is_flag_set(Flag::ZERO));
    EXPECT_TRUE(reg.is_flag_set(Flag::CARRY));
}

TEST(RegisterWithFlagsTest, DecrementSetsFlags) {
    auto flags = std::make_shared<Flags>();
    Register reg(flags);
    
    reg.set_value(1);
    reg.dec();
    EXPECT_EQ(reg.get_value(), 0);
    EXPECT_TRUE(reg.is_flag_set(Flag::ZERO));
    
    reg.set_value(0);
    reg.dec();
    EXPECT_EQ(reg.get_value(), 0xFFFF);
    EXPECT_TRUE(reg.is_flag_set(Flag::CARRY));
}

// Test ALU operations
TEST(ALUTest, Addition) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    acc->set_value(5);
    alu.add(3);
    EXPECT_EQ(acc->get_value(), 8);
}

TEST(ALUTest, AdditionWithOverflow) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    acc->set_value(0xFFFF);
    alu.add(1);
    EXPECT_EQ(acc->get_value(), 0);
    EXPECT_TRUE(acc->is_flag_set(Flag::ZERO));
    EXPECT_TRUE(acc->is_flag_set(Flag::CARRY));
}

TEST(ALUTest, Subtraction) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    acc->set_value(10);
    alu.sub(3);
    EXPECT_EQ(acc->get_value(), 7);
}

TEST(ALUTest, Comparison) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    // Test a < b
    acc->set_value(5);
    alu.cmp(10);
    EXPECT_EQ(acc->get_value(), 0xFFFF); // -1
    
    // Test a == b
    acc->set_value(10);
    alu.cmp(10);
    EXPECT_EQ(acc->get_value(), 0); // 0
    
    // Test a > b
    acc->set_value(15);
    alu.cmp(10);
    EXPECT_EQ(acc->get_value(), 1); // 1
}

TEST(ALUTest, BitwiseOperations) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    // AND
    acc->set_value(0xFF0F);
    alu.bit_and(0x0F0F);
    EXPECT_EQ(acc->get_value(), 0x0F0F);
    
    // OR
    acc->set_value(0xFF00);
    alu.bit_or(0x00FF);
    EXPECT_EQ(acc->get_value(), 0xFFFF);
    
    // XOR
    acc->set_value(0xFFFF);
    alu.bit_xor(0xAAAA);
    EXPECT_EQ(acc->get_value(), 0x5555);
    
    // NOT
    acc->set_value(0xAAAA);
    alu.bit_not();
    EXPECT_EQ(acc->get_value(), 0x5555);
}
