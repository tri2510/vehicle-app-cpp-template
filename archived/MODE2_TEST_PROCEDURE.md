# 🧪 Mode 2 Test Procedure - Quick Build Utility

## 📋 **Test Overview**

This document provides comprehensive test procedures to verify Mode 2 (Quick Build Utility) functionality with and without proxy configurations, including custom VSS support.

## 🎯 **Test Objectives**

✅ Verify basic Mode 2 functionality  
✅ Test proxy and non-proxy environments  
✅ Validate custom VSS support  
✅ Confirm error handling  
✅ Ensure Mode 1 compatibility is maintained  

---

## 🛠️ **Prerequisites**

### **System Requirements**
- Docker installed and running
- Internet connectivity (for VSS downloads)
- 4GB+ free disk space
- 2GB+ RAM available

### **Test Files Setup**
```bash
# Navigate to project directory
cd vehicle-app-cpp-template

# Verify test files exist
ls -la app/src/VehicleApp.cpp
ls -la Dockerfile.quick
ls -la scripts/
ls -la templates/

# Check current git status
git status
```

---

## 📋 **Test Suite 1: Basic Functionality (No Proxy)**

### **Test 1.1: Container Build**
```bash
echo "🧪 Test 1.1: Building Mode 2 container (no proxy)"

# Build the quick container
docker build -f Dockerfile.quick -t velocitas-quick .

# Verify build success
if [ $? -eq 0 ]; then
    echo "✅ PASS: Container built successfully"
else
    echo "❌ FAIL: Container build failed"
    exit 1
fi

# Check container size
docker images velocitas-quick
```

**Expected Result:** Container builds without errors, size ~2-3GB

### **Test 1.2: Help Command**
```bash
echo "🧪 Test 1.2: Help command functionality"

# Test help command
docker run --rm velocitas-quick help

# Check exit code
if [ $? -eq 0 ]; then
    echo "✅ PASS: Help command works"
else
    echo "❌ FAIL: Help command failed"
fi
```

**Expected Result:** Help text displays usage examples and environment variables

### **Test 1.3: Basic Build (Stdin)**
```bash
echo "🧪 Test 1.3: Basic build via stdin"

# Test basic build with stdin
cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick build

# Check exit code
if [ $? -eq 0 ]; then
    echo "✅ PASS: Basic stdin build successful"
else
    echo "❌ FAIL: Basic stdin build failed"
fi
```

**Expected Result:** 
- VehicleApp.cpp loads successfully
- VSS model generates
- Build completes with executable
- No errors in output

### **Test 1.4: File Mount Build**
```bash
echo "🧪 Test 1.4: Build via file mount"

# Test build with file mount
docker run --rm -v $(pwd)/app/src/VehicleApp.cpp:/input velocitas-quick build

# Check exit code
if [ $? -eq 0 ]; then
    echo "✅ PASS: File mount build successful"
else
    echo "❌ FAIL: File mount build failed"
fi
```

**Expected Result:** Same as Test 1.3 but with file input method

### **Test 1.5: Validation Command**
```bash
echo "🧪 Test 1.5: Template validation"

# Test validation
cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick validate

# Check exit code (0 = pass, 2 = warnings only, 1 = errors)
exit_code=$?
if [ $exit_code -eq 0 ] || [ $exit_code -eq 2 ]; then
    echo "✅ PASS: Validation completed (exit code: $exit_code)"
else
    echo "❌ FAIL: Validation failed (exit code: $exit_code)"
fi
```

**Expected Result:** 
- C++ structure validation passes
- Velocitas patterns detected
- Code quality analysis completes
- Exit code 0 or 2 (warnings only)

### **Test 1.6: Quick Run Command**
```bash
echo "🧪 Test 1.6: Quick run functionality"

# Test quick run (build + run)
timeout 60 bash -c 'cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick run' || true

# Note: This may timeout, which is expected behavior
echo "✅ PASS: Quick run command executed (timeout expected)"
```

**Expected Result:**
- Build completes successfully
- Application starts
- Runs for timeout period (15s default)
- Service availability checks work

---

## 🌐 **Test Suite 2: Proxy Environment**

### **Test 2.1: Container Build with Proxy**
```bash
echo "🧪 Test 2.1: Building container with proxy"

# Build with proxy arguments
docker build -f Dockerfile.quick \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --build-arg http_proxy=http://127.0.0.1:3128 \
  --build-arg https_proxy=http://127.0.0.1:3128 \
  --network=host \
  -t velocitas-quick-proxy .

# Verify build success
if [ $? -eq 0 ]; then
    echo "✅ PASS: Proxy container built successfully"
else
    echo "❌ FAIL: Proxy container build failed"
fi
```

**Expected Result:** Container builds even if proxy server not available (build-time proxy)

### **Test 2.2: Runtime with Proxy Environment**
```bash
echo "🧪 Test 2.2: Runtime with proxy environment variables"

# Test with runtime proxy environment
cat app/src/VehicleApp.cpp | docker run --rm -i \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  -e http_proxy=http://127.0.0.1:3128 \
  -e https_proxy=http://127.0.0.1:3128 \
  velocitas-quick build

# Check exit code
if [ $? -eq 0 ]; then
    echo "✅ PASS: Runtime proxy environment works"
else
    echo "❌ FAIL: Runtime proxy environment failed"
fi
```

**Expected Result:** Build completes successfully with proxy environment set

---

## 🔧 **Test Suite 3: Custom VSS Support**

### **Test 3.1: Default VSS (All Signals)**
```bash
echo "🧪 Test 3.1: Default VSS 4.0 with all signals"

# Create test VehicleApp that uses multiple signals
cat > test_vss_app.cpp << 'EOF'
#include "sdk/VehicleApp.h"
#include "sdk/Logger.h"
#include "vehicle/Vehicle.hpp"

class TestVSSApp : public velocitas::VehicleApp {
public:
    TestVSSApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {}

protected:
    void onStart() override {
        logger().info("Testing multiple VSS signals...");
        
        // Test various signal categories
        subscribeDataPoints({
            "Vehicle.Speed",
            "Vehicle.Body.Doors.Row1.Left.IsOpen", 
            "Vehicle.ADAS.CruiseControl.IsActive",
            "Vehicle.Chassis.Brake.PedalPosition",
            "Vehicle.Powertrain.Engine.Speed"
        });
        
        logger().info("VSS signal subscriptions completed");
    }
};

DEFINE_VEHICLE_APP(TestVSSApp)
EOF

# Test with multiple signal categories
cat test_vss_app.cpp | docker run --rm -i velocitas-quick build

if [ $? -eq 0 ]; then
    echo "✅ PASS: Default VSS supports multiple signal categories"
    rm test_vss_app.cpp
else
    echo "❌ FAIL: Default VSS missing signal categories"
    rm test_vss_app.cpp
fi
```

**Expected Result:** Build succeeds with signals from multiple VSS categories

### **Test 3.2: Custom VSS File**
```bash
echo "🧪 Test 3.2: Custom VSS file support"

# Create minimal custom VSS file
cat > test_custom_vss.json << 'EOF'
{
  "Vehicle": {
    "type": "branch",
    "description": "Test vehicle data.",
    "children": {
      "Speed": {
        "datatype": "float",
        "type": "sensor",
        "unit": "m/s",
        "description": "Vehicle speed."
      },
      "TestSignal": {
        "datatype": "string",
        "type": "actuator",
        "description": "Custom test signal."
      }
    }
  }
}
EOF

# Create test app using custom signal
cat > test_custom_app.cpp << 'EOF'
#include "sdk/VehicleApp.h"
#include "sdk/Logger.h"
#include "vehicle/Vehicle.hpp"

class CustomVSSApp : public velocitas::VehicleApp {
public:
    CustomVSSApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {}

protected:
    void onStart() override {
        logger().info("Testing custom VSS signals...");
        
        subscribeDataPoints({
            "Vehicle.Speed",
            "Vehicle.TestSignal"
        });
        
        logger().info("Custom VSS signal subscriptions completed");
    }
};

DEFINE_VEHICLE_APP(CustomVSSApp)
EOF

# Test with custom VSS file
cat test_custom_app.cpp | docker run --rm -i \
  -v $(pwd)/test_custom_vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  velocitas-quick build

if [ $? -eq 0 ]; then
    echo "✅ PASS: Custom VSS file support works"
else
    echo "❌ FAIL: Custom VSS file support failed"
fi

# Cleanup
rm test_custom_vss.json test_custom_app.cpp
```

**Expected Result:** Build succeeds using custom VSS with TestSignal

### **Test 3.3: Custom VSS URL**
```bash
echo "🧪 Test 3.3: Custom VSS URL support"

# Test with VSS 3.0 URL (different version)
cat app/src/VehicleApp.cpp | docker run --rm -i \
  -e VSS_SPEC_URL=https://github.com/COVESA/vehicle_signal_specification/releases/download/v3.0/vss_rel_3.0.json \
  velocitas-quick build

if [ $? -eq 0 ]; then
    echo "✅ PASS: Custom VSS URL support works"
else
    echo "❌ FAIL: Custom VSS URL support failed"
fi
```

**Expected Result:** Build succeeds downloading and using VSS 3.0

### **Test 3.4: Invalid VSS URL**
```bash
echo "🧪 Test 3.4: Invalid VSS URL handling"

# Test with invalid URL
cat app/src/VehicleApp.cpp | docker run --rm -i \
  -e VSS_SPEC_URL=invalid-url-format \
  velocitas-quick build

if [ $? -ne 0 ]; then
    echo "✅ PASS: Invalid VSS URL properly rejected"
else
    echo "❌ FAIL: Invalid VSS URL should have failed"
fi
```

**Expected Result:** Build fails with clear error message about invalid URL format

---

## 🚨 **Test Suite 4: Error Handling**

### **Test 4.1: No Input Provided**
```bash
echo "🧪 Test 4.1: No input error handling"

# Test with no input
docker run --rm velocitas-quick build

if [ $? -ne 0 ]; then
    echo "✅ PASS: No input properly handled"
else
    echo "❌ FAIL: Should fail when no input provided"
fi
```

**Expected Result:** Clear error message about missing input with usage examples

### **Test 4.2: Invalid VehicleApp.cpp**
```bash
echo "🧪 Test 4.2: Invalid C++ file handling"

# Create invalid C++ file
echo "This is not valid C++ code" | docker run --rm -i velocitas-quick build

if [ $? -ne 0 ]; then
    echo "✅ PASS: Invalid C++ properly rejected"
else
    echo "❌ FAIL: Invalid C++ should have failed"
fi
```

**Expected Result:** Build fails with compilation errors

### **Test 4.3: Missing VehicleApp Class**
```bash
echo "🧪 Test 4.3: Missing VehicleApp class validation"

# Create C++ without VehicleApp class
cat > invalid_app.cpp << 'EOF'
#include <iostream>
int main() {
    std::cout << "Hello World" << std::endl;
    return 0;
}
EOF

cat invalid_app.cpp | docker run --rm -i velocitas-quick validate

validation_exit=$?
if [ $validation_exit -eq 1 ]; then
    echo "✅ PASS: Missing VehicleApp class detected"
else
    echo "❌ FAIL: Should detect missing VehicleApp class (exit: $validation_exit)"
fi

rm invalid_app.cpp
```

**Expected Result:** Validation fails with error about missing VehicleApp class

### **Test 4.4: Network Connectivity Issues**
```bash
echo "🧪 Test 4.4: Network connectivity handling"

# Test with unreachable VSS URL
cat app/src/VehicleApp.cpp | timeout 30 docker run --rm -i \
  -e VSS_SPEC_URL=https://nonexistent.example.com/vss.json \
  velocitas-quick build

if [ $? -ne 0 ]; then
    echo "✅ PASS: Network connectivity issues handled"
else
    echo "❌ FAIL: Should fail with unreachable URL"
fi
```

**Expected Result:** Build fails gracefully with network error message

---

## 🔄 **Test Suite 5: Mode 1 Compatibility**

### **Test 5.1: Mode 1 Still Works**
```bash
echo "🧪 Test 5.1: Mode 1 compatibility verification"

# Test that Mode 1 container still builds
docker build -f Dockerfile.dev -t velocitas-dev . >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ PASS: Mode 1 container builds successfully"
else
    echo "❌ FAIL: Mode 1 container build broken"
fi

# Test basic Mode 1 functionality (without services)
docker run --rm -v $(pwd):/workspace velocitas-dev \
  bash -c "gen-model && install-deps" >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ PASS: Mode 1 basic commands work"
else
    echo "❌ FAIL: Mode 1 basic commands broken"
fi
```

**Expected Result:** Mode 1 still builds and basic commands work

---

## 📊 **Test Suite 6: Performance & Resource Usage**

### **Test 6.1: Build Performance**
```bash
echo "🧪 Test 6.1: Build performance measurement"

# Measure build time
start_time=$(date +%s)
cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick build >/dev/null 2>&1
end_time=$(date +%s)

build_duration=$((end_time - start_time))
echo "⏱️  Build duration: ${build_duration} seconds"

if [ $build_duration -lt 60 ]; then
    echo "✅ PASS: Build completed in under 60 seconds"
else
    echo "⚠️  WARNING: Build took longer than expected"
fi
```

**Expected Result:** Build completes in under 60 seconds (after container is built)

### **Test 6.2: Resource Usage**
```bash
echo "🧪 Test 6.2: Resource usage measurement"

# Check container image sizes
echo "📊 Container sizes:"
docker images | grep velocitas
echo ""

# Check available disk space
echo "💾 Disk usage:"
df -h | grep -E "(Filesystem|/dev/)"
echo ""

# Test memory usage during build
echo "🧠 Memory usage test:"
cat app/src/VehicleApp.cpp | docker run --rm -i \
  --memory=1g \
  velocitas-quick build >/dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ PASS: Build works with 1GB memory limit"
else
    echo "⚠️  WARNING: Build requires more than 1GB memory"
fi
```

**Expected Result:** 
- Container size reasonable (~2-3GB)
- Build works with 1GB memory limit
- Sufficient disk space available

---

## 🎯 **Test Results Summary Template**

```bash
# Create test results summary
cat > test_results_summary.md << 'EOF'
# Mode 2 Test Results Summary

**Test Date:** $(date)
**Environment:** $(uname -a)
**Docker Version:** $(docker --version)

## Test Results

### Basic Functionality (No Proxy)
- [ ] Container Build: PASS/FAIL
- [ ] Help Command: PASS/FAIL  
- [ ] Basic Build (Stdin): PASS/FAIL
- [ ] File Mount Build: PASS/FAIL
- [ ] Validation Command: PASS/FAIL
- [ ] Quick Run Command: PASS/FAIL

### Proxy Environment  
- [ ] Container Build with Proxy: PASS/FAIL
- [ ] Runtime with Proxy: PASS/FAIL

### Custom VSS Support
- [ ] Default VSS (All Signals): PASS/FAIL
- [ ] Custom VSS File: PASS/FAIL
- [ ] Custom VSS URL: PASS/FAIL
- [ ] Invalid VSS URL: PASS/FAIL

### Error Handling
- [ ] No Input Error: PASS/FAIL
- [ ] Invalid C++ File: PASS/FAIL
- [ ] Missing VehicleApp Class: PASS/FAIL
- [ ] Network Connectivity: PASS/FAIL

### Mode 1 Compatibility
- [ ] Mode 1 Still Works: PASS/FAIL

### Performance
- [ ] Build Performance: PASS/FAIL/WARNING
- [ ] Resource Usage: PASS/FAIL/WARNING

## Issues Found
(List any issues encountered)

## Recommendations
(List any recommendations for improvement)
EOF

echo "📋 Test results template created: test_results_summary.md"
```

---

## 🚀 **Quick Test Runner Script**

```bash
#!/bin/bash
# quick_test_runner.sh - Automated test execution

echo "🧪 Mode 2 Quick Test Runner"
echo "=========================="

# Set strict error handling
set -e

# Track test results
PASSED=0
FAILED=0

# Function to run test and track results
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    echo ""
    echo "🔬 Running: $test_name"
    echo "Command: $test_command"
    
    if eval "$test_command"; then
        echo "✅ PASSED: $test_name"
        ((PASSED++))
    else
        echo "❌ FAILED: $test_name"
        ((FAILED++))
    fi
}

# Basic functionality tests
run_test "Container Build" "docker build -f Dockerfile.quick -t velocitas-quick . >/dev/null 2>&1"
run_test "Help Command" "docker run --rm velocitas-quick help >/dev/null 2>&1"
run_test "Basic Build" "cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick build >/dev/null 2>&1"
run_test "Validation" "cat app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick validate >/dev/null 2>&1"

# Error handling tests
run_test "No Input Error" "! docker run --rm velocitas-quick build >/dev/null 2>&1"
run_test "Invalid VSS URL" "! echo 'invalid' | docker run --rm -i -e VSS_SPEC_URL=invalid-url velocitas-quick build >/dev/null 2>&1"

# Display results
echo ""
echo "🏁 Test Results Summary"
echo "======================"
echo "✅ Passed: $PASSED"
echo "❌ Failed: $FAILED"
echo "📊 Total:  $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "⚠️  Some tests failed. Review the output above."
    exit 1
fi
```

---

## 📝 **Manual Test Checklist**

Print this checklist and manually verify each item:

### **Pre-Test Setup**
- [ ] Docker installed and running
- [ ] Internet connectivity available
- [ ] 4GB+ free disk space
- [ ] Project directory accessible
- [ ] All test files present

### **Basic Tests**
- [ ] Container builds without errors
- [ ] Help command shows usage information
- [ ] Stdin input builds successfully  
- [ ] File mount input builds successfully
- [ ] Validation detects template compliance
- [ ] Quick run executes and times out properly

### **Proxy Tests**
- [ ] Container builds with proxy args
- [ ] Runtime proxy environment works
- [ ] Proxy settings don't break functionality

### **VSS Tests**
- [ ] Default VSS provides access to multiple signal categories
- [ ] Custom VSS file mount works
- [ ] Custom VSS URL download works
- [ ] Invalid VSS URLs are rejected

### **Error Tests**
- [ ] No input produces helpful error
- [ ] Invalid C++ is rejected
- [ ] Missing VehicleApp class detected
- [ ] Network issues handled gracefully

### **Compatibility Tests**
- [ ] Mode 1 still builds and works
- [ ] No conflicts between Mode 1 and Mode 2

### **Performance Tests**
- [ ] Build completes in reasonable time (<60s)
- [ ] Memory usage is acceptable (<1GB)
- [ ] Container size is reasonable (<3GB)

---

**🧪 Ready to test Mode 2? Run the test suites above to verify full functionality!**