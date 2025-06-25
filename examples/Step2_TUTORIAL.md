# 🎓 Step 2: Multi-Signal Processor Tutorial

## 📚 Learning Objectives

In this second step, you'll advance to handling multiple vehicle signals:

- **Multi-Signal Subscription**: Subscribe to speed, RPM, and fuel level together
- **Data Correlation**: Analyze relationships between different signals
- **State Management**: Track comprehensive vehicle state efficiently  
- **Derived Metrics**: Calculate fuel efficiency and driving patterns

**Difficulty**: ⭐⭐ Intermediate | **Time**: 30 minutes

## 🎯 What You'll Build

A multi-signal processing application that:
- Monitors Vehicle.Speed, Engine.Speed (RPM), and FuelSystem.Level simultaneously
- Correlates data to detect driving patterns
- Calculates real-time fuel efficiency
- Provides comprehensive vehicle status reports
- Generates intelligent alerts based on combined signals

## 📋 Prerequisites

**Required Setup:**
```bash
# Ensure KUKSA databroker is running
docker ps | grep velocitas-vdb || docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Reuse persistent volumes from Step 1 (or create new ones)
docker volume ls | grep tutorial-build || docker volume create tutorial-build
docker volume ls | grep tutorial-deps || docker volume create tutorial-deps  
docker volume ls | grep tutorial-vss || docker volume create tutorial-vss
```

**Container Image Options:**
```bash
# Option A: Use pre-built image (RECOMMENDED - instant start!)
# Throughout this tutorial, we'll use:
# ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest

# Option B: Build locally (if you haven't already)
docker build -f Dockerfile.quick -t velocitas-quick .
```

## 🧪 Step-by-Step Tutorial

### **Phase 1: Build the Multi-Signal Application**

**Using Pre-built Image (Recommended):**
```bash
# Build Step 2 application with persistent storage
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v tutorial-deps:/home/vscode/.conan2 \
  -v tutorial-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step2_MultiSignalProcessor.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] Source validated: /app.cpp (385 lines)
✅ [SUCCESS] Vehicle model exists, skipping
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
🎉 Build completed successfully!
```

### **Phase 2: Run the Application**

**Using Pre-built Image:**
```bash
# Start the multi-signal processor
docker run -d --network host --name step2-app \
  -v tutorial-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 120
```

**Monitor Application Logs:**
```bash
# Watch multi-signal processing in action
docker logs step2-app --follow
```

**Expected Startup Logs:**
```
🎓 Step 2: Starting Multi-Signal Processor Tutorial
🎯 Learning Goal: Master multi-signal correlation
📊 Processing: Speed + RPM + Fuel Level
🎓 Step 2: Multi-Signal Processor starting...
📡 Connecting to Vehicle Data Broker...
🚗 Learning objective: Process multiple vehicle signals
📊 Signals: Speed, Engine RPM, Fuel Level
✅ Multi-Signal Processor initialized
🚀 Step 2: Starting Multi-Signal Processor!
✅ Multi-signal subscriptions completed
🔄 Monitoring: Speed + RPM + Fuel Level
```

### **Phase 3: Multi-Signal Testing**

**Test 1: Individual Signal Updates**
```bash
# First, inject speed signal
echo "setValue Vehicle.Speed 20.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check logs to see partial update
docker logs step2-app --tail 5
```

**Expected Response:**
```
📡 Received multi-signal update
   Speed: 20.00 m/s
🚗 Vehicle Status Update:
   📊 Speed: 72.0 km/h | RPM: 0 | Fuel: 0.0%
```

**Test 2: Complete Multi-Signal Update**
```bash
# Inject all three signals
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Powertrain.Engine.Speed 2500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Powertrain.FuelSystem.Level 75.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check comprehensive status
docker logs step2-app --tail 10
```

**Expected Multi-Signal Response:**
```
📡 Received multi-signal update
   Speed: 25.00 m/s
   RPM: 2500
   Fuel: 75.5%
🚗 Vehicle Status Update:
   📊 Speed: 90.0 km/h | RPM: 2500 | Fuel: 75.5%
🎯 Driving Mode Changed: UNKNOWN → HIGHWAY_CRUISE
✅ Optimal driving conditions for fuel efficiency
⛽ Fuel Efficiency: 6.2 L/100km
👍 Good fuel efficiency
```

**Test 3: City Driving Pattern**
```bash
# Simulate city driving (low speed, higher RPM)
echo "setValue Vehicle.Speed 10.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Powertrain.Engine.Speed 3500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check driving mode detection
docker logs step2-app --tail 8
```

**Expected Pattern Detection:**
```
🚗 Vehicle Status Update:
   📊 Speed: 36.0 km/h | RPM: 3500 | Fuel: 75.5%
🎯 Driving Mode Changed: HIGHWAY_CRUISE → CITY_AGGRESSIVE
⛽ Fuel Efficiency: 10.5 L/100km
💸 Poor fuel efficiency - consider adjusting driving style
```

**Test 4: Low Fuel Warning**
```bash
# Test low fuel scenario
echo "setValue Vehicle.Powertrain.FuelSystem.Level 12.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check fuel warning
docker logs step2-app --tail 5
```

**Expected Low Fuel Alert:**
```
🚗 Vehicle Status Update:
   📊 Speed: 36.0 km/h | RPM: 3500 | Fuel: 12.0%
⛽ Low fuel warning! 12.0% remaining (~60 km range)
```

**Test 5: Aggressive Acceleration Detection**
```bash
# High RPM at low speed
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Powertrain.Engine.Speed 4500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check acceleration warning
docker logs step2-app --tail 5
```

**Expected Acceleration Alert:**
```
🚗 Vehicle Status Update:
   📊 Speed: 18.0 km/h | RPM: 4500 | Fuel: 12.0%
⚠️  Aggressive acceleration detected! RPM: 4500 at 18.0 km/h
```

**Test 6: Comprehensive Status Report**
```bash
# Generate multiple data points for statistics
for i in {1..5}; do
  speed=$((15 + i * 5))
  rpm=$((2000 + i * 200))
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  echo "setValue Vehicle.Powertrain.Engine.Speed $rpm" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 2
done

# Check periodic status report
docker logs step2-app --tail 15
```

**Expected Status Report:**
```
📊 === VEHICLE STATUS REPORT ===
📈 Average Speed: 82.5 km/h
🔧 Average RPM: 2800
⛽ Fuel Efficiency: 7.8 L/100km
🎯 Driving Mode: HIGHWAY_CRUISE
📊 Data Points: 10
==============================
```

## 🔍 Code Analysis & Learning Points

### **Key Patterns You Learned:**

**1. Multi-Signal Subscription:**
```cpp
subscribeDataPoints(
    velocitas::QueryBuilder::select(Vehicle.Speed)
        .select(Vehicle.Powertrain.Engine.Speed)     // Chain multiple signals
        .select(Vehicle.Powertrain.FuelSystem.Level)
        .build()
)
```

**2. Individual Signal Processing:**
```cpp
// Each signal validated separately
if (reply.get(Vehicle.Speed)->isValid()) {
    m_vehicleState.speed = reply.get(Vehicle.Speed)->value();
}
if (reply.get(Vehicle.Powertrain.Engine.Speed)->isValid()) {
    m_vehicleState.engineRpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
}
```

**3. State Management Structure:**
```cpp
struct VehicleState {
    double speed = 0.0;
    double engineRpm = 0.0;
    double fuelLevel = 0.0;
    bool dataValid = false;
    std::chrono::steady_clock::time_point lastUpdate;
};
```

**4. Multi-Signal Correlation:**
```cpp
// Combine signals for insights
if (speedKmh < 30 && m_vehicleState.engineRpm > 4000) {
    // Aggressive acceleration detected
}
```

**5. Derived Metrics Calculation:**
```cpp
double calculateInstantFuelConsumption(double speed, double rpm) {
    // Fuel efficiency based on speed and RPM
    return baseConsumption * rpmFactor * speedFactor;
}
```

## ✅ Success Criteria Validation

### **✅ Multi-Signal Processing:**
- **PASS**: Three signals subscribed simultaneously
- **PASS**: Individual signal validation working
- **PASS**: State management tracking all signals
- **PASS**: Updates processed as signals arrive

### **✅ Data Correlation:**
- **PASS**: Driving mode detection from speed + RPM
- **PASS**: Fuel efficiency calculation working
- **PASS**: Pattern detection (aggressive acceleration)
- **PASS**: Combined signal alerts functional

### **✅ Advanced Features:**
- **PASS**: Periodic status reports generated
- **PASS**: Statistical averaging implemented
- **PASS**: Low fuel warnings with range estimate
- **PASS**: Comprehensive vehicle state tracking

## 📊 Performance Benchmarks

| Metric | Expected Value | Your Result |
|--------|----------------|-------------|
| **Build Time** | 15-30 seconds (cached) | ⏱️ _____ |
| **Application Size** | ~14MB | 📏 _____ |
| **Signal Response** | <100ms per update | ⚡ _____ |
| **Memory Usage** | ~80-120MB | 💾 _____ |
| **CPU Usage** | <5% idle, <10% active | 🔧 _____ |

## 🐛 Common Issues & Solutions

### **Issue 1: Missing Signal Data**
```bash
# Verify all signals are being set
echo "getValue Vehicle.Speed" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "getValue Vehicle.Powertrain.Engine.Speed" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Issue 2: No Driving Mode Changes**
```bash
# Ensure varied speed/RPM combinations
# City: Low speed (10-40 km/h), varying RPM
# Highway: Medium speed (50-90 km/h), steady RPM
# Sport: Any speed with high RPM (>3500)
```

### **Issue 3: No Status Reports**
```bash
# Status reports appear every 10 data points
# Generate more signal updates to trigger report
```

## 🧹 Cleanup

```bash
# Stop the application
docker stop step2-app && docker rm step2-app

# Keep persistent volumes for next steps
# docker volume rm tutorial-build tutorial-deps tutorial-vss
```

## 🎓 Knowledge Check

**Before proceeding to Step 3, ensure you understand:**

1. **Multi-Signal Subscription**: How to chain `.select()` calls for multiple signals
2. **State Management**: Using structures to track multiple related values
3. **Signal Correlation**: Combining signals for pattern detection
4. **Derived Metrics**: Calculating new values from existing signals
5. **Validation Patterns**: Checking each signal individually in callbacks
6. **Statistical Tracking**: Maintaining averages and counts over time

## 🚀 What's Next?

**Ready for Step 3?** You'll learn advanced data analysis and alert systems:

- **Pattern Analysis**: Detect complex driving behaviors over time
- **Alert Framework**: Build multi-tier alert system with priorities
- **Historical Tracking**: Implement rolling windows and trends
- **Predictive Analytics**: Anticipate issues before they occur
- **Business Rules**: Implement sophisticated logic for fleet management

**[Continue to Step 3: Data Analysis & Alerts →](Step3_TUTORIAL.md)**

---

## 📈 Step 2 Completion Checklist

- ✅ Successfully built Step2_MultiSignalProcessor application
- ✅ Subscribed to three vehicle signals simultaneously
- ✅ Processed speed, RPM, and fuel level together
- ✅ Detected driving modes from signal correlation
- ✅ Calculated fuel efficiency metrics
- ✅ Generated alerts for aggressive driving and low fuel
- ✅ Viewed periodic vehicle status reports
- ✅ Understood multi-signal processing patterns

**🎉 Congratulations! You've mastered multi-signal vehicle data processing!**

---

*🚗💨 Correlate multiple signals for intelligent vehicle insights!*