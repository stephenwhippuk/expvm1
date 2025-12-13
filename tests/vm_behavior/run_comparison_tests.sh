#!/bin/bash

# VM Comparison Test Runner
# Assembles and runs comparison tests, recording actual VM behavior

set -e

ASM="../../build/asm"
LVM="../../build/lvm"
TEST_DIR="comparison"
RESULTS_DIR="comparison_results"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "======================================"
echo "VM Comparison Behavior Tests"
echo "======================================"
echo ""

# Create results directory
mkdir -p "$RESULTS_DIR"

# Counter
total=0
assembled=0

# Find all test files
for test_file in "$TEST_DIR"/*.asm; do
    if [ ! -f "$test_file" ]; then
        continue
    fi
    
    total=$((total + 1))
    test_name=$(basename "$test_file" .asm)
    bin_file="$RESULTS_DIR/${test_name}.bin"
    
    echo -n "Testing $test_name... "
    
    # Assemble
    if $ASM "$test_file" -o "$bin_file" 2>"$RESULTS_DIR/${test_name}.err"; then
        echo -e "${GREEN}assembled${NC}"
        assembled=$((assembled + 1))
        
        # Note: Actual VM execution would go here
        # For now, just verify assembly works
        # In future: run VM and capture register/memory state
        
    else
        echo -e "${RED}FAILED${NC}"
        echo "  Error details in $RESULTS_DIR/${test_name}.err"
    fi
done

echo ""
echo "======================================"
echo "Results:"
echo "======================================"
echo "  Assembled: $assembled / $total"
echo ""

if [ $assembled -eq $total ]; then
    echo -e "${GREEN}All tests assembled successfully${NC}"
    echo ""
    echo "To execute tests manually:"
    echo "  $LVM $RESULTS_DIR/<test>.bin 0x0000"
    echo ""
    echo "Then inspect memory/registers to validate behavior"
    exit 0
else
    echo -e "${RED}Some tests failed to assemble${NC}"
    exit 1
fi
