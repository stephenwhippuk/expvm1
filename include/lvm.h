#pragma once

// LVM - Lightweight Virtual Machine
// Main header file - includes all library components

// Memory Unit
#include "../src/memunit/include/memsize.h"
#include "../src/memunit/include/accessMode.h"
#include "../src/memunit/include/mem_access.h"
#include "../src/memunit/include/memunit.h"
#include "../src/memunit/include/memory2.h"
#include "../src/memunit/include/vaddr.h"
#include "../src/memunit/include/context.h"
#include "../src/memunit/include/vmemunit.h"
#include "../src/memunit/include/paged_memory_accessor.h"
#include "../src/memunit/include/stack_accessor.h"

// Helpers
#include "../src/helpers/include/errors.h"
#include "../src/helpers/include/helpers.h"

// Register and Flags
#include "../src/register/include/flags.h"
#include "../src/register/include/register.h"

// ALU
#include "../src/alu/include/alu.h"

// Stack
#include "../src/stack/include/stack.h"

// Instruction Unit
#include "../src/instruction_unit/include/opcodes.h"
#include "../src/instruction_unit/include/instruction_unit.h"

// CPU
#include "../src/cpu/include/cpu.h"

// VM
#include "../src/vm/include/vm.h"
