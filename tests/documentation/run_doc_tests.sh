#!/bin/bash
# ============================================
# Documentation Example Test Runner
# ============================================
# Tests all assembly examples from documentation
# to ensure they assemble correctly
# ============================================

set -u  # Exit on undefined variable, but not on command failure

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR/../.."
ASSEMBLER="$PROJECT_ROOT/build/asm"
TEST_DIR="$SCRIPT_DIR/testable"
ERROR_DIR="$SCRIPT_DIR/error_cases"
RESULTS_DIR="$SCRIPT_DIR/results"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0
SKIPPED=0

# Options
VERBOSE=0
CATEGORY=""
STOP_ON_FAIL=0

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -c|--category)
            CATEGORY="$2"
            shift 2
            ;;
        -s|--stop-on-fail)
            STOP_ON_FAIL=1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -v, --verbose        Show detailed output"
            echo "  -c, --category DIR   Test only specific category (e.g., 'quick')"
            echo "  -s, --stop-on-fail   Stop on first failure"
            echo "  -h, --help           Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                          # Run all tests"
            echo "  $0 -c quick                 # Run quick tests only"
            echo "  $0 -v -s                    # Verbose with stop on fail"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Check if assembler exists
if [ ! -f "$ASSEMBLER" ]; then
    echo -e "${RED}Error: Assembler not found at $ASSEMBLER${NC}"
    echo "Please build the project first:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. && make"
    exit 1
fi

# Create results directory
mkdir -p "$RESULTS_DIR"

# Print header
echo "======================================"
echo "Documentation Example Tests"
echo "======================================"
echo "Assembler: $ASSEMBLER"
echo "Test dir:  $TEST_DIR"
echo ""

# Function to test a single file
test_file() {
    local asm_file="$1"
    local base=$(basename "$asm_file" .asm)
    local dir=$(dirname "$asm_file")
    local relative=$(echo "$asm_file" | sed "s|$TEST_DIR/||")
    
    if [ $VERBOSE -eq 1 ]; then
        echo ""
        echo "Testing: $relative"
    else
        printf "%-50s " "$relative"
    fi
    
    # Assemble
    local output="$RESULTS_DIR/${base}.bin"
    local errlog="$RESULTS_DIR/${base}.err"
    
    if "$ASSEMBLER" "$asm_file" -o "$output" > "$errlog" 2>&1; then
        # Success
        if [ $VERBOSE -eq 1 ]; then
            echo -e "  ${GREEN}✓ PASS${NC} (assembled successfully)"
            ls -lh "$output" | awk '{print "  Binary size:", $5}'
        else
            echo -e "${GREEN}✓ PASS${NC}"
        fi
        ((PASSED++))
        return 0
    else
        # Failed
        if [ $VERBOSE -eq 1 ]; then
            echo -e "  ${RED}✗ FAIL${NC} (assembly failed)"
            echo "  Error output:"
            sed 's/^/    /' "$errlog"
        else
            echo -e "${RED}✗ FAIL${NC}"
        fi
        ((FAILED++))
        
        if [ $STOP_ON_FAIL -eq 1 ]; then
            echo ""
            echo "Stopping on failure (--stop-on-fail)"
            cat "$errlog"
            exit 1
        fi
        return 1
    fi
}

# Find test files
if [ -n "$CATEGORY" ]; then
    TEST_PATH="$TEST_DIR/$CATEGORY"
    if [ ! -d "$TEST_PATH" ]; then
        echo -e "${RED}Error: Category not found: $CATEGORY${NC}"
        exit 1
    fi
else
    TEST_PATH="$TEST_DIR"
fi

# Count total tests
TOTAL=$(find "$TEST_PATH" -name "*.asm" -type f | wc -l)

if [ $TOTAL -eq 0 ]; then
    echo -e "${YELLOW}No test files found in $TEST_PATH${NC}"
    exit 0
fi

echo "Found $TOTAL test file(s)"
echo "======================================"

# Run tests
while IFS= read -r asm_file; do
    test_file "$asm_file" || true  # Don't exit on test failure
done < <(find "$TEST_PATH" -name "*.asm" -type f | sort)

# Recalculate counters based on actual results
PASSED=$(ls -1 "$RESULTS_DIR"/*.bin 2>/dev/null | wc -l || echo "0")
TOTAL_ERRORS=$(ls -1 "$RESULTS_DIR"/*.err 2>/dev/null | wc -l || echo "0")
FAILED=$((TOTAL - PASSED))

# Print summary
echo ""
echo "======================================"
echo "Test Results:"
echo "======================================"
echo -e "  ${GREEN}Passed:${NC}  $PASSED"
echo -e "  ${RED}Failed:${NC}  $FAILED"
echo "  Total:   $TOTAL"
echo "======================================"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    find "$RESULTS_DIR" -name "*.err" -type f | while read errfile; do
        base=$(basename "$errfile" .err)
        if [ ! -f "$RESULTS_DIR/${base}.bin" ]; then
            echo "  - $base"
        fi
    done
    exit 1
else
    echo ""
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
