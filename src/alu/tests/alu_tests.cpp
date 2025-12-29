#include <gtest/gtest.h>
#include "memsize.h"
#include "register.h"
#include "flags.h"
#include "alu.h"
using namespace lvm;

// Test ALU operations
TEST(ALUTest, Addition) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    acc->get_accessor()->set_value(5);
    alu.add(3);
    EXPECT_EQ(acc->get_accessor()->get_value(), 8);
}

TEST(ALUTest, AdditionWithOverflow) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    acc->get_accessor()->set_value(0xFFFF);
    alu.add(1);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0);
    EXPECT_TRUE(acc->get_accessor()->is_flag_set(Flag::ZERO));
    EXPECT_TRUE(acc->get_accessor()->is_flag_set(Flag::CARRY));
}

TEST(ALUTest, Subtraction) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    // Basic subtraction
    acc->get_accessor()->set_value(10);
    alu.sub(3);
    EXPECT_EQ(acc->get_accessor()->get_value(), 7);
    EXPECT_FALSE(acc->get_accessor()->is_flag_set(Flag::ZERO));
    
    // Subtraction resulting in zero - THIS IS CRITICAL FOR JPNZ TEST
    acc->get_accessor()->set_value(5);
    alu.sub(5);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0);
    EXPECT_TRUE(acc->get_accessor()->is_flag_set(Flag::ZERO));
    
    // Subtraction resulting in negative (sign flag)
    acc->get_accessor()->set_value(3);
    alu.sub(5);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0xFFFE); // -2 in two's complement
    EXPECT_TRUE(acc->get_accessor()->is_flag_set(Flag::SIGN));
    EXPECT_FALSE(acc->get_accessor()->is_flag_set(Flag::ZERO));
}

TEST(ALUTest, Comparison) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    // Test a < b
    acc->get_accessor()->set_value(5);
    alu.cmp(10);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0xFFFF); // -1
    
    // Test a == b
    acc->get_accessor()->set_value(10);
    alu.cmp(10);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0); // 0
    
    // Test a > b
    acc->get_accessor()->set_value(15);
    alu.cmp(10);
    EXPECT_EQ(acc->get_accessor()->get_value(), 1); // 1
}

TEST(ALUTest, BitwiseOperations) {
    auto flags = std::make_shared<Flags>();
    auto acc = std::make_shared<Register>(flags);
    Alu alu(acc);
    
    // AND
    acc->get_accessor()->set_value(0xFF0F);
    alu.bit_and(0x0F0F);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0x0F0F);
    
    // OR
    acc->get_accessor()->set_value(0xFF00);
    alu.bit_or(0x00FF);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0xFFFF);
    
    // XOR
    acc->get_accessor()->set_value(0xFFFF);
    alu.bit_xor(0xAAAA);
    EXPECT_EQ(acc->get_accessor()->get_value(), 0x5555);
    
    // NOT
    acc->get_accessor()->set_value(0xAAAA);
    alu.bit_not();
    EXPECT_EQ(acc->get_accessor()->get_value(), 0x5555);
}
