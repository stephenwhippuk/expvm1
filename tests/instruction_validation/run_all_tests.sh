#!/bin/bash
# Test Runner for Instruction Validation Suite
# Runs all instruction validation tests and reports results

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VM_BIN="${SCRIPT_DIR}/../../build/lvm"
ASM_BIN="${SCRIPT_DIR}/../../build/asm"
TEMP_DIR="/tmp/vm_test_$$"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# Check if binaries exist
check_binaries() {
    if [ ! -f "$VM_BIN" ]; then
        echo -e "${RED}Error: VM binary not found at $VM_BIN${NC}"
        echo "Please build the project first: cmake --build build"
        exit 1
    fi
    
    if [ ! -f "$ASM_BIN" ]; then
        echo -e "${RED}Error: Assembler binary not found at $ASM_BIN${NC}"
        echo "Please build the project first: cmake --build build"
        exit 1
    fi
}

# Create temp directory
mkdir -p "$TEMP_DIR"

# Run a single test
run_test() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .asm)
    local category=$(basename $(dirname "$test_file"))
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Assemble the test
    "$ASM_BIN" "$test_file" -o "$TEMP_DIR/$test_name.bin" 2>"$TEMP_DIR/$test_name.asm_err"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAIL${NC} [$category] $test_name - Assembly failed"
        cat "$TEMP_DIR/$test_name.asm_err"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
    
    # Run the test
    "$VM_BIN" "$TEMP_DIR/$test_name.bin" 0x0000 >"$TEMP_DIR/$test_name.out" 2>"$TEMP_DIR/$test_name.err"
    local exit_code=$?
    
    # For now, we just check if VM ran without crashing
    # Future: Parse expected results from test file and verify
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}PASS${NC} [$category] $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}FAIL${NC} [$category] $test_name - VM exit code: $exit_code"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Run all tests in a category
run_category() {
    local category_dir="$1"
    local category_name=$(basename "$category_dir")
    
    echo ""
    echo -e "${YELLOW}=== Testing Category: $category_name ===${NC}"
    
    # Find all .asm files in category
    local test_files=$(find "$category_dir" -name "*.asm" -type f | sort)
    
    if [ -z "$test_files" ]; then
        echo "  No tests found"
        return
    fi
    
    for test_file in $test_files; do
        run_test "$test_file"
    done
}

# Main execution
main() {
    echo "=========================================="
    echo "  Instruction Validation Test Suite"
    echo "=========================================="
    echo ""
    
    check_binaries
    
    # If specific category provided, test only that
    if [ $# -gt 0 ]; then
        for category in "$@"; do
            category_path="$SCRIPT_DIR/$category"
            if [ -d "$category_path" ]; then
                run_category "$category_path"
            else
                echo -e "${RED}Error: Category not found: $category${NC}"
            fi
        done
    else
        # Test all categories
        for category_dir in "$SCRIPT_DIR"/[0-9]*; do
            if [ -d "$category_dir" ]; then
                run_category "$category_dir"
            fi
        done
    fi
    
    # Print summary
    echo ""
    echo "=========================================="
    echo "  Test Summary"
    echo "=========================================="
    echo "Total Tests:   $TOTAL_TESTS"
    echo -e "Passed:        ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Failed:        ${RED}$FAILED_TESTS${NC}"
    echo -e "Skipped:       ${YELLOW}$SKIPPED_TESTS${NC}"
    echo ""
    
    if [ $FAILED_TESTS -eq 0 ] && [ $TOTAL_TESTS -gt 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        cleanup_and_exit 0
    elif [ $TOTAL_TESTS -eq 0 ]; then
        echo -e "${YELLOW}No tests found!${NC}"
        cleanup_and_exit 1
    else
        echo -e "${RED}Some tests failed!${NC}"
        cleanup_and_exit 1
    fi
}

# Cleanup
cleanup_and_exit() {
    rm -rf "$TEMP_DIR"
    exit $1
}

# Handle Ctrl+C
trap cleanup_and_exit INT

# Run main
main "$@"
