#!/bin/bash
# Test script to verify driver generator functionality

echo "=========================================="
echo "Driver Generator - Verification Test"
echo "=========================================="
echo ""

# Test 1: Generator script exists
echo "[TEST 1] Checking if generator script exists..."
if [ -f "driver_generator.py" ]; then
    echo "✓ PASS: driver_generator.py found"
else
    echo "✗ FAIL: driver_generator.py not found"
    exit 1
fi

# Test 2: Generator is executable
echo ""
echo "[TEST 2] Checking if generator script is valid Python..."
if python3 -m py_compile driver_generator.py 2>/dev/null; then
    echo "✓ PASS: driver_generator.py is valid Python"
else
    echo "✗ FAIL: driver_generator.py has syntax errors"
    exit 1
fi

# Test 3: Generate a test project
echo ""
echo "[TEST 3] Generating test project..."
TEST_DIR="./test_output_$$"
if python3 driver_generator.py TestDriver "$TEST_DIR" <<< "yes" >/dev/null 2>&1; then
    echo "✓ PASS: Test project generated successfully"
else
    echo "✗ FAIL: Failed to generate test project"
    exit 1
fi

# Test 4: Verify project structure
echo ""
echo "[TEST 4] Verifying project structure..."
REQUIRED_FILES=(
    "$TEST_DIR/driver/driver.h"
    "$TEST_DIR/driver/driver.c"
    "$TEST_DIR/usermode/driver_interface.h"
    "$TEST_DIR/usermode/driver_interface.cpp"
    "$TEST_DIR/usermode/main.cpp"
    "$TEST_DIR/CMakeLists.txt"
    "$TEST_DIR/build.py"
    "$TEST_DIR/README.md"
    "$TEST_DIR/config.json"
)

ALL_EXIST=true
for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file (missing)"
        ALL_EXIST=false
    fi
done

if [ "$ALL_EXIST" = true ]; then
    echo "✓ PASS: All required files generated"
else
    echo "✗ FAIL: Some files missing"
    exit 1
fi

# Test 5: Verify content
echo ""
echo "[TEST 5] Verifying generated content..."
if grep -q "IOCTL_READ_MEMORY" "$TEST_DIR/driver/driver.h" && \
   grep -q "ReadProcessMemory" "$TEST_DIR/driver/driver.c" && \
   grep -q "DriverInterface" "$TEST_DIR/usermode/driver_interface.h"; then
    echo "✓ PASS: Generated code contains expected content"
else
    echo "✗ FAIL: Generated code missing expected content"
    exit 1
fi

# Test 6: Verify documentation
echo ""
echo "[TEST 6] Verifying documentation..."
if [ -f "README.md" ] && [ -f "QUICKSTART.md" ] && [ -f "example_test.py" ]; then
    echo "✓ PASS: All documentation files present"
else
    echo "✗ FAIL: Some documentation missing"
    exit 1
fi

# Cleanup
echo ""
echo "[CLEANUP] Removing test output..."
rm -rf "$TEST_DIR"
echo "✓ Test output cleaned up"

# All tests passed
echo ""
echo "=========================================="
echo "✓ ALL TESTS PASSED!"
echo "=========================================="
echo ""
echo "The driver generator is working correctly."
echo "You can now use it to create driver projects:"
echo ""
echo "  python3 driver_generator.py MyDriver ./output"
echo ""
exit 0
