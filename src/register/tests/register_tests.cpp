#include <gtest/gtest.h>
#include "register.h"
#include "flags.h"

using namespace lvm;

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
