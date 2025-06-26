# 🎓 Step 3: Simple Custom VSS Tutorial

## 📚 Learning Objectives

In this step, you'll learn real custom VSS signal processing:

- **Custom VSS Creation**: Design and implement simple custom signals
- **KUKSA Custom Loading**: Configure KUKSA databroker with custom VSS specification
- **Raw Signal Subscription**: Subscribe to custom signals using direct paths
- **End-to-End Testing**: Test real custom signals from KUKSA injection to app processing

**Difficulty**: ⭐⭐⭐ Advanced | **Time**: 45 minutes

## 🎯 What You'll Build

A simple custom VSS application that:
- Creates 3 custom signals: Temperature, Message, Counter
- Loads custom VSS specification into KUKSA databroker
- Subscribes to custom signals using raw signal paths
- Processes real custom signals injected via KUKSA client
- Demonstrates end-to-end custom VSS workflow

## 📋 Prerequisites

**Required Setup:**
```bash
# Stop any existing KUKSA instance
docker stop velocitas-vdb kuksa-custom-vss 2>/dev/null || true

# Create persistent volumes for Step 3
docker volume rm step3-build step3-deps step3-vss 2>/dev/null || true
docker volume create step3-build
docker volume create step3-deps  
docker volume create step3-vss
```

## 🚀 Step-by-Step Tutorial

### **Phase 1: Start KUKSA with Custom VSS**

```bash
# Start KUKSA databroker with simple custom VSS specification
docker run -d --rm --name kuksa-simple-custom --network host \
  -v $(pwd)/examples/simple_custom_vss.json:/vss.json \
  ghcr.io/eclipse-kuksa/kuksa-databroker:main \
  --address 0.0.0.0 --port 55555 --insecure --vss /vss.json
```

**Expected Output:**
```bash
# Check KUKSA logs
docker logs kuksa-simple-custom --tail 5
# Should show:
# INFO: Loading VSS metadata from /vss.json
# INFO: Listening on 0.0.0.0:55555
```

### **Phase 2: Build Simple Custom VSS Application**

```bash
# Build simple custom VSS app
docker run --rm --network host \
  -v step3-build:/quickbuild/build \
  -v step3-deps:/home/vscode/.conan2 \
  -v step3-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step3_SimpleCustomVSS.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: ~14M (simple custom VSS)
🎉 Simple Custom VSS build completed!
```

### **Phase 3: Run Simple Custom VSS Application**

```bash
# Run simple custom VSS application
docker run -d --network host --name step3-simple-custom \
  -v step3-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 180
```

**Expected Runtime Output:**
```bash
# Check application logs
docker logs step3-simple-custom --tail 10
```

**Should show:**
```
🎓 Step 3: Simple Custom VSS starting...
📡 Connecting to Vehicle Data Broker...
🚀 Step 3: Starting Simple Custom VSS!
📊 Setting up custom VSS signal subscriptions...
✅ Custom VSS signal subscriptions completed
🔄 Waiting for custom VSS signals...
💡 Test with custom signals:
   echo 'setValue Vehicle.MyCustom.Temperature 25.5' | kuksa-client
```

## 📊 Testing Real Custom VSS Signals

### **Test 1: Custom Temperature Signal**

```bash
# Test custom temperature signal
echo "setValue Vehicle.MyCustom.Temperature 25.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-simple-custom --tail 5
```

**Expected Output:**
```
📡 Received custom VSS signal update
🌡️  Custom Temperature: 25.5°C
✅ Custom temperature normal: 25.5°C
🎯 Processing custom VSS signals...
📊 === CUSTOM VSS STATE ===
🌡️  Temperature: 25.5°C
🎉 Custom VSS signals working!
```

### **Test 2: Custom Message Signal**

```bash
# Test custom message signal
echo "setValue Vehicle.MyCustom.Message \"Hello Custom VSS!\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-simple-custom --tail 5
```

**Expected Output:**
```
📡 Received custom VSS signal update
💬 Custom Message: 'Hello Custom VSS!'
📝 Custom message received: 'Hello Custom VSS!'
🎯 Processing custom VSS signals...
💬 Message: 'Hello Custom VSS!'
🎉 Custom VSS signals working!
```

### **Test 3: Custom Counter Signal**

```bash
# Test custom counter signal
echo "setValue Vehicle.MyCustom.Counter 42" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-simple-custom --tail 5
```

**Expected Output:**
```
📡 Received custom VSS signal update
🔢 Custom Counter: 42
🔢 Custom counter: 42
🎯 Processing custom VSS signals...
🔢 Counter: 42
🎉 Custom VSS signals working!
```

### **Test 4: High Value Alerts**

```bash
# Test high temperature (should trigger warning)
echo "setValue Vehicle.MyCustom.Temperature 35.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test alert message (should trigger warning)
echo "setValue Vehicle.MyCustom.Message \"ALERT: System Check Required\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test high counter (should trigger warning)
echo "setValue Vehicle.MyCustom.Counter 150" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check all warnings
docker logs step3-simple-custom --tail 15 | grep -E "(WARN|🔥|🚨|📊)"
```

**Expected Alert Output:**
```
🔥 HIGH CUSTOM TEMPERATURE: 35.5°C
🚨 ALERT in custom message: 'ALERT: System Check Required'
📊 HIGH CUSTOM COUNTER: 150
```

### **Test 5: Complete Custom State**

```bash
# Set all custom signals at once
echo "setValue Vehicle.MyCustom.Temperature 28.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.MyCustom.Message \"All systems operational\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.MyCustom.Counter 75" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check complete state
docker logs step3-simple-custom --tail 10 | grep -A 10 "CUSTOM VSS STATE"
```

**Expected Complete State:**
```
📊 === CUSTOM VSS STATE ===
🌡️  Temperature: 28.0°C
💬 Message: 'All systems operational'
🔢 Counter: 75
🎉 Custom VSS signals working!
========================
```

## 🧪 Advanced Testing Scenarios

### **Scenario 1: Rapid Signal Updates**

```bash
# Test rapid updates to same signal
for i in {1..5}; do
  echo "setValue Vehicle.MyCustom.Counter $((i * 10))" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 1
done

# Check counter progression
docker logs step3-simple-custom | grep "Custom Counter:" | tail -5
```

### **Scenario 2: Temperature Range Testing**

```bash
# Test temperature range
temperatures=(-5.0 15.0 25.0 35.0 45.0)
for temp in "${temperatures[@]}"; do
  echo "setValue Vehicle.MyCustom.Temperature $temp" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 2
done

# Check temperature responses
docker logs step3-simple-custom | grep -E "(Custom Temperature|HIGH|LOW)" | tail -10
```

## 🎓 Learning Outcomes

After completing this tutorial, you will have learned:

### ✅ **Custom VSS Signal Creation**
- How to define custom VSS specifications in JSON format
- Simple signal types: sensor with float, string, uint32 datatypes
- Custom signal organization under Vehicle.MyCustom branch

### ✅ **KUKSA Custom VSS Integration**
- Loading custom VSS files into KUKSA databroker with `--vss` parameter
- Running KUKSA with custom signal specifications
- Verifying custom VSS loading in KUKSA logs

### ✅ **Raw Signal Subscription Patterns**
- Direct signal path subscription: "Vehicle.MyCustom.SignalName"
- Raw databroker client usage: `getDataBrokerClient()->subscribeDatapoints()`
- Processing signal responses without generated vehicle models

### ✅ **End-to-End Custom VSS Testing**
- Real signal injection: `setValue Vehicle.MyCustom.*` commands
- Signal processing verification through application logs
- Custom business logic implementation for custom signals

## 🔧 Custom VSS Specification Format

The `simple_custom_vss.json` file defines:

```json
{
  "Vehicle": {
    "type": "branch",
    "Speed": {
      "type": "sensor",
      "datatype": "float",
      "unit": "m/s"
    },
    "MyCustom": {
      "type": "branch",
      "Temperature": {
        "type": "sensor", 
        "datatype": "float",
        "unit": "celsius"
      },
      "Message": {
        "type": "sensor",
        "datatype": "string"
      },
      "Counter": {
        "type": "sensor", 
        "datatype": "uint32"
      }
    }
  }
}
```

## 🔄 Next Steps

### **Expand Custom VSS:**
1. **Add More Signal Types**: boolean, int32, double, string arrays
2. **Create Signal Branches**: Organize signals into logical groups
3. **Add Signal Metadata**: descriptions, units, min/max values
4. **Complex Data Types**: arrays, nested structures

### **Production Integration:**
1. **Vehicle Model Generation**: Generate C++ bindings for custom signals
2. **Type-Safe Subscriptions**: Use generated Vehicle model classes
3. **Signal Validation**: Add range checking and validation rules
4. **Performance Optimization**: Batch subscriptions and efficient processing

## 🎉 Congratulations!

You have successfully built and tested a **real custom VSS application**! You now know how to:
- ✅ Create custom VSS specifications
- ✅ Load custom VSS into KUKSA databroker  
- ✅ Subscribe to custom signals in C++ applications
- ✅ Process real custom signals end-to-end
- ✅ Test custom VSS with KUKSA client injection

**Your custom VSS application is fully functional and ready for expansion!** 🚀

## 🧹 Cleanup

```bash
# Stop containers
docker stop step3-simple-custom kuksa-simple-custom

# Remove containers  
docker rm step3-simple-custom kuksa-simple-custom

# Clean volumes (optional)
docker volume rm step3-build step3-deps step3-vss
```