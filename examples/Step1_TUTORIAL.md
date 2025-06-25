# 🎓 Step 1: Basic Speed Monitor Tutorial

## 📚 Learning Objectives

In this first step, you'll master the fundamentals of vehicle application development:

- **Signal Subscription**: Learn how to connect to vehicle data streams
- **Data Processing**: Handle real-time vehicle speed information
- **Unit Conversion**: Transform technical units to practical formats
- **Error Handling**: Build robust signal processing logic

**Difficulty**: ⭐ Beginner | **Time**: 15 minutes

## 🎯 What You'll Build

A basic speed monitoring application that:
- Connects to KUKSA Vehicle Data Broker
- Subscribes to `Vehicle.Speed` signal
- Converts speed from m/s to km/h
- Categorizes speed levels (stopped, slow, normal, fast)
- Provides real-time speed feedback

## 📋 Prerequisites

**Required Setup:**
```bash
# Start KUKSA databroker
docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create persistent volumes for faster development
docker volume create tutorial-build
docker volume create tutorial-deps
docker volume create tutorial-vss

# Verify databroker is running
docker logs velocitas-vdb --tail 5
```

**Container Image Options:**
```bash
# Option A: Use pre-built image (RECOMMENDED - no build needed!)
# Throughout this tutorial, replace 'velocitas-quick' with:
# ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest

# Option B: Build locally (3-5 minutes)
docker build -f Dockerfile.quick -t velocitas-quick .
```

**Expected Databroker Output:**
```
[INFO] Listening on 0.0.0.0:55555
[INFO] TLS is not enabled
[INFO] Authorization is not enabled
```

## 🧪 Step-by-Step Tutorial

### **Phase 1: Build the Application**

**Using Pre-built Image (Recommended):**
```bash
# Build Step 1 application with persistent storage
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v tutorial-deps:/home/vscode/.conan2 \
  -v tutorial-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Using Local Build:**
```bash
# Same command with 'velocitas-quick' instead of full image name
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v tutorial-deps:/home/vscode/.conan2 \
  -v tutorial-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
  velocitas-quick build --skip-deps --verbose
```

**Expected Build Output:**
```
🔧 BUILD DEBUG - Skip Deps: true
🔧 BUILD DEBUG - Skip VSS: false
✅ [SUCCESS] Source validated: /app.cpp (245 lines)
✅ [SUCCESS] Vehicle model exists, skipping (use --force to regenerate)
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
🎉 Build completed successfully!
```

### **Phase 2: Run the Application**

**Using Pre-built Image (Recommended):**
```bash
# Start the basic speed monitor
docker run -d --network host --name step1-app \
  -v tutorial-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
```

**Using Local Build:**
```bash
# Same command with 'velocitas-quick'
docker run -d --network host --name step1-app \
  -v tutorial-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  velocitas-quick run 60
```

**Monitor Application Logs:**
```bash
# Watch application startup and signal processing
docker logs step1-app --follow
```

**Expected Startup Logs:**
```
🎓 Step 1: Starting Basic Speed Monitor Tutorial
🎯 Learning Goal: Master vehicle signal processing
🎓 Step 1: Basic Speed Monitor starting...
📡 Connecting to Vehicle Data Broker...
🚗 Learning objective: Process Vehicle.Speed signal
✅ Basic Speed Monitor initialized
🚀 Step 1: Starting Basic Speed Monitor!
📊 Setting up Vehicle.Speed signal subscription...
✅ Vehicle.Speed subscription completed
🔄 Waiting for speed data from vehicle...
💡 Test with: echo 'setValue Vehicle.Speed 25.0' | kuksa-client
```

### **Phase 3: Interactive Signal Testing**

**Test 1: City Driving Speed**
```bash
# Inject city speed (15 m/s = 54 km/h)
echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check application response
docker logs step1-app --tail 5
```

**Expected Response:**
```
📡 Received vehicle signal data
🚗 Vehicle Speed: 54.0 km/h (15.00 m/s)
📊 Speed Category: HIGHWAY_SPEED
🛣️  Highway Speed: 54.0 km/h
```

**Test 2: Highway Speed**
```bash
# Inject highway speed (30 m/s = 108 km/h)
echo "setValue Vehicle.Speed 30.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check application response
docker logs step1-app --tail 5
```

**Expected Response:**
```
📡 Received vehicle signal data
🚗 Vehicle Speed: 108.0 km/h (30.00 m/s)
📊 Speed Category: HIGH_SPEED
⚠️  HIGH SPEED: 108.0 km/h - Drive safely!
```

**Test 3: Stopped Vehicle**
```bash
# Test stopped vehicle (0 m/s)
echo "setValue Vehicle.Speed 0.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check application response
docker logs step1-app --tail 5
```

**Expected Response:**
```
📡 Received vehicle signal data
🚗 Vehicle Speed: 0.0 km/h (0.00 m/s)
📊 Speed Category: STOPPED
🛑 Vehicle Stopped
```

**Test 4: Low Speed Driving**
```bash
# Test low speed (5 m/s = 18 km/h)
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check application response
docker logs step1-app --tail 5
```

**Expected Response:**
```
📡 Received vehicle signal data
🚗 Vehicle Speed: 18.0 km/h (5.00 m/s)
📊 Speed Category: VERY_SLOW
🐌 Low Speed: 18.0 km/h
```

## 🔍 Code Analysis & Learning Points

### **Key Code Patterns You Learned:**

**1. Signal Subscription Setup:**
```cpp
subscribeDataPoints(
    velocitas::QueryBuilder::select(Vehicle.Speed)  // Choose signal
        .build()                                     // Build query
)
->onItem([this](auto&& item) { 
    onSignalChanged(std::forward<decltype(item)>(item)); 
})
->onError([this](auto&& status) { 
    velocitas::logger().error("❌ Error: {}", status.errorMessage());
});
```

**2. Signal Validation:**
```cpp
if (reply.get(Vehicle.Speed)->isValid()) {
    double speedMs = reply.get(Vehicle.Speed)->value();
    processSpeedSignal(speedMs);
}
```

**3. Unit Conversion:**
```cpp
double speedKmh = speedMs * 3.6;  // m/s to km/h conversion
```

**4. Categorization Logic:**
```cpp
std::string categorizeSpeed(double speedKmh) {
    if (speedKmh == 0.0) return "STOPPED";
    else if (speedKmh <= 20.0) return "VERY_SLOW";
    else if (speedKmh <= 50.0) return "CITY_SPEED";
    else if (speedKmh <= 100.0) return "HIGHWAY_SPEED";
    else return "HIGH_SPEED";
}
```

## ✅ Success Criteria Validation

### **✅ Signal Processing Verification:**
- **PASS**: Application connects to KUKSA databroker
- **PASS**: Vehicle.Speed signal subscription established
- **PASS**: Speed data received and processed correctly
- **PASS**: Unit conversion (m/s → km/h) accurate

### **✅ Business Logic Verification:**
- **PASS**: Speed categorization working (STOPPED, VERY_SLOW, etc.)
- **PASS**: Conditional alerts for high speeds
- **PASS**: Structured logging with clear messages
- **PASS**: Real-time signal processing operational

### **✅ Application Lifecycle:**
- **PASS**: Clean startup and initialization
- **PASS**: Graceful signal handling and shutdown
- **PASS**: Error handling for invalid signals
- **PASS**: Continuous operation during signal injection

## 📊 Performance Benchmarks

| Metric | Expected Value | Your Result |
|--------|----------------|-------------|
| **Build Time** | 15-30 seconds (with --skip-deps) | ⏱️ _____ |
| **Application Size** | ~14MB | 📏 _____ |
| **Signal Response** | <100ms | ⚡ _____ |
| **Memory Usage** | ~50-100MB | 💾 _____ |

## 🐛 Common Issues & Solutions

### **Issue 1: No Signal Data Received**
```bash
# Check databroker status
docker logs velocitas-vdb --tail 10

# Verify signal injection command
echo "getValue Vehicle.Speed" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Issue 2: Application Not Starting**
```bash
# Check application logs for errors
docker logs step1-app

# Verify build was successful
docker run --rm -v tutorial-build:/quickbuild/build alpine ls -la /quickbuild/build/bin/
```

### **Issue 3: Build Failures**
```bash
# Clean rebuild if needed (with pre-built image)
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --clean --force --verbose

# Or with local build
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
  velocitas-quick build --clean --force --verbose
```

## 🧹 Cleanup

```bash
# Stop the application
docker stop step1-app && docker rm step1-app

# Optional: Stop databroker
docker compose -f docker-compose.dev.yml down

# Keep persistent volumes for next tutorial steps
# docker volume rm tutorial-build tutorial-deps tutorial-vss
```

## 🎓 Knowledge Check

**Before proceeding to Step 2, ensure you understand:**

1. **Signal Subscription**: How to use `QueryBuilder.select()` and `subscribeDataPoints()`
2. **Data Validation**: Why and how to check `isValid()` before processing
3. **Unit Conversion**: Converting automotive units (m/s to km/h) 
4. **Callback Pattern**: How lambda functions handle incoming signal data
5. **Error Handling**: Managing subscription errors and invalid data
6. **Logging**: Using structured logging for debugging and monitoring

## 🚀 What's Next?

**Ready for Step 2?** You'll learn to handle multiple signals simultaneously:

- **Multi-Signal Processing**: Handle speed, RPM, and fuel level together
- **Data Correlation**: Analyze relationships between different signals  
- **Enhanced Logic**: Implement more sophisticated vehicle behavior analysis
- **Performance Monitoring**: Track multiple vehicle systems in real-time

**[Continue to Step 2: Multi-Signal Processing →](Step2_TUTORIAL.md)**

---

## 📈 Step 1 Completion Checklist

- ✅ Successfully built Step1_BasicSpeedMonitor application
- ✅ Established Vehicle.Speed signal subscription  
- ✅ Processed speed data with correct unit conversion
- ✅ Implemented speed categorization logic
- ✅ Tested with multiple speed values
- ✅ Understood core SDV development patterns
- ✅ Ready to advance to multi-signal processing

**🎉 Congratulations! You've completed Step 1 of SDV Development!**

---

*🚗💨 Master vehicle signal processing step by step!*