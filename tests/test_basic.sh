#!/bin/bash

# CryptStream Basic Test Suite

set -e

echo "CryptStream Test Suite"
echo "======================"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

CRYPTSTREAM="../bin/cryptstream"
TEST_KEY="test_key_12345"

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Helper function to run test
run_test() {
    local test_name=$1
    local command=$2
    
    echo -n "Testing: $test_name... "
    
    if eval "$command" > /dev/null 2>&1; then
        echo -e "${GREEN}PASSED${NC}"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Create test directory
mkdir -p test_files
cd test_files

# Test 1: Create test file
echo "Test data for encryption" > test_input.txt
run_test "Create test file" "test -f test_input.txt"

# Test 2: Encrypt file
run_test "Encrypt file" "$CRYPTSTREAM encrypt test_input.txt test_encrypted.enc --key $TEST_KEY"

# Test 3: Verify encrypted file exists
run_test "Encrypted file exists" "test -f test_encrypted.enc"

# Test 4: Decrypt file
run_test "Decrypt file" "$CRYPTSTREAM decrypt test_encrypted.enc test_decrypted.txt --key $TEST_KEY"

# Test 5: Verify decrypted file exists
run_test "Decrypted file exists" "test -f test_decrypted.txt"

# Test 6: Verify content matches
run_test "Content matches" "diff test_input.txt test_decrypted.txt"

# Test 7: Test with different key (should produce different output)
$CRYPTSTREAM encrypt test_input.txt test_encrypted2.enc --key "different_key" > /dev/null 2>&1
run_test "Different key produces different output" "! diff test_encrypted.enc test_encrypted2.enc"

# Test 8: Large file test (1MB)
dd if=/dev/urandom of=large_file.dat bs=1024 count=1024 > /dev/null 2>&1
run_test "Encrypt large file" "$CRYPTSTREAM encrypt large_file.dat large_encrypted.enc --key $TEST_KEY --processes 4"
run_test "Decrypt large file" "$CRYPTSTREAM decrypt large_encrypted.enc large_decrypted.dat --key $TEST_KEY --processes 4"
run_test "Large file content matches" "diff large_file.dat large_decrypted.dat"

# Cleanup
cd ..
rm -rf test_files

# Summary
echo ""
echo "======================"
echo "     Test Summary     "
echo "======================"
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
