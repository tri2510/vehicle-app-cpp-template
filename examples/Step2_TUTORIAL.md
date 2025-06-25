# 🎓 Step 2: Multi-Signal Processor Tutorial

## 📚 Learning Objectives

In this second step, you'll advance to handling multiple vehicle signals:

- **Multi-Signal Subscription**: Subscribe to speed and GPS location signals together
- **Data Correlation**: Analyze relationships between different signals
- **State Management**: Track comprehensive vehicle state efficiently  
- **Data Simulation**: Generate realistic derived data (RPM/fuel) from available signals
- **Robust Error Handling**: Handle partial signal updates and missing data gracefully

**Difficulty**: ⭐⭐ Intermediate | **Time**: 30 minutes

## 🎯 What You'll Build

A multi-signal processing application that:
- Monitors Vehicle.Speed and GPS coordinates (real VSS signals) simultaneously
- Simulates Engine RPM and fuel level based on speed and distance
- Correlates data to detect driving patterns and vehicle modes
- Provides comprehensive vehicle status reports with location tracking
- Demonstrates robust error handling for missing or unavailable signals

## 📋 Prerequisites

**Required Setup:**
```bash
# Ensure KUKSA databroker is running
docker ps | grep velocitas-vdb || docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create fresh persistent volumes for Step 2 (isolated from other steps)
docker volume rm step2-build step2-deps step2-vss 2>/dev/null || true
docker volume create step2-build
docker volume create step2-deps  
docker volume create step2-vss
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
# Build Step 2 application with dedicated persistent storage and proxy support
docker run --rm --network host \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  -v step2-build:/quickbuild/build \
  -v step2-deps:/home/vscode/.conan2 \
  -v step2-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step2_MultiSignalProcessor.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] Source validated: /app.cpp (483 lines)
🔍 [DEBUG] Source preview (first 10 lines):
    // ============================================================================
    // 🎓 STEP 2: MULTI-SIGNAL PROCESSOR - Handle Multiple Vehicle Data Streams
    // ============================================================================
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
  -v step2-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 300
```

**Monitor Application Logs:**
```bash
# Watch multi-signal processing in action
docker logs step2-app --follow
```

**Expected Startup Logs:**
```
🎓 Step 2: Multi-Signal Processor starting...
📡 Connecting to Vehicle Data Broker...
🚗 Learning objective: Process multiple vehicle signals
📊 Signals: Speed, GPS Location + Simulated RPM/Fuel
✅ Multi-Signal Processor initialized
🚀 Step 2: Starting Multi-Signal Processor!
📊 Setting up multiple signal subscriptions...
✅ Multi-signal subscriptions completed
🔄 Monitoring: Speed + GPS Location (RPM/Fuel simulated)
💡 Test with multiple signals:
   echo 'setValue Vehicle.Speed 25.0' | kuksa-client
   echo 'setValue Vehicle.CurrentLocation.Latitude 40.7589' | kuksa-client
   echo 'setValue Vehicle.CurrentLocation.Longitude -73.9851' | kuksa-client
App is running.
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

**Test 2: GPS Coordinates Update**
```bash
# Inject GPS coordinates to test location tracking
echo "setValue Vehicle.CurrentLocation.Latitude 40.7589" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CurrentLocation.Longitude -73.9851" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check GPS update
docker logs step2-app --tail 10
```

**Expected GPS Response:**
```
📡 Received multi-signal update
   Latitude: 40.758900
   Longitude: -73.985100
🚗 Vehicle Status Update:
   📊 Speed: 90.0 km/h | RPM: 2400 | Fuel: 75.0%
   📍 Location: (40.758900, -73.985100)
🎯 Driving Mode Changed: UNKNOWN → HIGHWAY_CRUISE
✅ Optimal driving conditions for fuel efficiency
⛽ Fuel Efficiency: 6.2 L/100km
👍 Good fuel efficiency
```

**Test 3: Educational Error Handling**
```bash
# Test the educational error messages when signals aren't available
# The Step 2 app shows how to handle missing signals gracefully
echo "setValue Vehicle.Speed 10.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check educational output about signal availability
docker logs step2-app --tail 15
```

**Expected Educational Response:**
```
📡 Received multi-signal update
   Speed: 10.00 m/s
🚗 Vehicle Status Update:
   📊 Speed: 36.0 km/h | RPM: 1200 | Fuel: 75.0%
   📍 Location: (40.758900, -73.985100)
🎯 Driving Mode Changed: HIGHWAY_CRUISE → CITY_NORMAL
⛽ Fuel Efficiency: 7.5 L/100km
👍 Good fuel efficiency
```

**Test 4: Signal Processing Demonstration**
```bash
# Test multiple speed values to see simulation in action
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check simulated data generation
docker logs step2-app --tail 10
```

**Expected Simulation Output:**
```
📡 Received multi-signal update
   Speed: 15.00 m/s
🚗 Vehicle Status Update:
   📊 Speed: 54.0 km/h | RPM: 2960 | Fuel: 74.0%
   📍 Location: (40.758900, -73.985100)
🎯 Driving Mode Changed: CITY_NORMAL → HIGHWAY_CRUISE
⛽ Fuel Efficiency: 6.8 L/100km
👍 Good fuel efficiency
```

**Test 5: High Speed Detection**
```bash
# Test high speed alert thresholds
echo "setValue Vehicle.Speed 35.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check high speed processing
docker logs step2-app --tail 8
```

**Expected High Speed Response:**
```
📡 Received multi-signal update
   Speed: 35.00 m/s
🚗 Vehicle Status Update:
   📊 Speed: 126.0 km/h | RPM: 6240 | Fuel: 73.0%
   📍 Location: (40.758900, -73.985100)
🎯 Driving Mode Changed: HIGHWAY_CRUISE → HIGH_SPEED
⚠️  Aggressive acceleration detected! RPM: 6240 at 126.0 km/h
⛽ Fuel Efficiency: 12.8 L/100km
💸 Poor fuel efficiency - consider adjusting driving style
```

**Test 6: Comprehensive Status Report**
```bash
# Generate multiple data points for statistics
for i in {1..5}; do
  speed=$((15 + i * 5))
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 3
done

# Check periodic status report (appears every 10th data point)
docker logs step2-app --tail 15
```

**Expected Status Report:**
```
📊 === VEHICLE STATUS REPORT ===
📈 Average Speed: 82.5 km/h
🔧 Average RPM: 4100 (simulated)
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
        .select(Vehicle.CurrentLocation.Latitude)    // Chain multiple signals
        .select(Vehicle.CurrentLocation.Longitude)   // GPS coordinates
        .build()
)
```

**2. Individual Signal Processing with Error Handling:**
```cpp
// Each signal validated separately with try-catch
try {
    if (reply.get(Vehicle.Speed)->isValid()) {
        m_vehicleState.speed = reply.get(Vehicle.Speed)->value();
    }
} catch (const std::exception& e) {
    velocitas::logger().debug("Speed signal not available: {}", e.what());
}
```

**3. State Management Structure:**
```cpp
struct VehicleState {
    double speed = 0.0;
    double latitude = 0.0;        // GPS coordinates
    double longitude = 0.0;
    double engineRpm = 0.0;       // Simulated from speed
    double fuelLevel = 75.0;      // Simulated consumption
    bool dataValid = false;
    std::chrono::steady_clock::time_point lastUpdate;
};
```

**4. Data Simulation and Correlation:**
```cpp
// Simulate RPM based on speed (realistic engine behavior)
m_vehicleState.engineRpm = speedKmh * 40.0 + 800.0;

// Combine real and simulated signals for insights
if (speedKmh < 30 && m_vehicleState.engineRpm > 4000) {
    // Aggressive acceleration detected
}
```

**5. Educational Error Handling:**
```cpp
// Show how to handle signals that aren't available in the databroker
try {
    if (reply.get(Vehicle.CurrentLocation.Latitude)->isValid()) {
        m_vehicleState.latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
        anyUpdate = true;
    }
} catch (const std::exception& e) {
    velocitas::logger().debug("Latitude signal not available: {}", e.what());
}
```

## ✅ Success Criteria Validation

### **✅ Multi-Signal Processing:**
- **PASS**: Speed and GPS signals subscribed simultaneously
- **PASS**: Individual signal validation with error handling
- **PASS**: State management tracking real and simulated signals
- **PASS**: Updates processed as signals arrive

### **✅ Data Simulation and Correlation:**
- **PASS**: RPM and fuel simulated from speed data
- **PASS**: Driving mode detection from speed + simulated RPM
- **PASS**: Fuel efficiency calculation working
- **PASS**: Educational error handling for missing signals

### **✅ Educational Features:**
- **PASS**: Periodic status reports generated
- **PASS**: Statistical averaging implemented
- **PASS**: Graceful handling of unavailable VSS signals
- **PASS**: Realistic data simulation from available signals

## 📊 Performance Benchmarks

| Metric | Expected Value | Your Result |
|--------|----------------|-------------|
| **Build Time** | 15-30 seconds (cached) | ⏱️ _____ |
| **Application Size** | ~14MB | 📏 _____ |
| **Signal Response** | <100ms per update | ⚡ _____ |
| **Memory Usage** | ~80-120MB | 💾 _____ |
| **CPU Usage** | <5% idle, <10% active | 🔧 _____ |

## 🐛 Common Issues & Solutions

### **Issue 1: Understanding Signal Availability**
```bash
# Check which signals are available in the KUKSA databroker
echo "getValue Vehicle.Speed" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "getValue Vehicle.CurrentLocation.Latitude" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Note: Engine.Speed and FuelSystem.Level are not available in this KUKSA setup
# Step 2 demonstrates how to handle this gracefully with simulation
```

### **Issue 2: Understanding Simulated Data**
```bash
# RPM and fuel are simulated from speed - this is educational!
# Vary speed to see realistic RPM simulation:
# Low speed (5-15 m/s): Simulated city driving
# Medium speed (15-25 m/s): Simulated highway cruising  
# High speed (25+ m/s): Simulated aggressive driving
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Issue 3: Debug Messages vs Educational Output**
```bash
# Some "signal not available" messages are educational features
# They show how to handle missing VSS signals gracefully
# Check logs with --follow to see educational error handling:
docker logs step2-app --follow

# Status reports appear every 10 data points
for i in {1..12}; do echo "setValue Vehicle.Speed $((i*2)).0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555; sleep 2; done
```

## 🧹 Cleanup

```bash
# Stop the application
docker stop step2-app && docker rm step2-app

# Keep persistent volumes for next steps (each step has its own volumes)
# docker volume rm step2-build step2-deps step2-vss
```

## 🎓 Knowledge Check

**Before proceeding to Step 3, ensure you understand:**

1. **Multi-Signal Subscription**: How to chain `.select()` calls for multiple signals
2. **Error Handling**: Using try-catch blocks for graceful signal processing
3. **Data Simulation**: Generating realistic derived data from available signals
4. **Signal Availability**: Understanding which VSS signals are available in KUKSA
5. **Educational Patterns**: Learning from both real signals and simulated data
6. **Robust Architecture**: Building apps that work with limited signal availability

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
- ✅ Subscribed to speed and GPS signals simultaneously
- ✅ Processed real VSS signals with simulated supplementary data
- ✅ Detected driving modes from speed and simulated RPM correlation
- ✅ Calculated fuel efficiency metrics from simulated consumption
- ✅ Understood graceful error handling for unavailable signals
- ✅ Viewed periodic vehicle status reports with mixed real/simulated data
- ✅ Mastered robust multi-signal processing patterns

**🎉 Congratulations! You've mastered robust multi-signal processing with real-world constraints!**

---

*🚗💨 Build resilient apps that handle both available and unavailable VSS signals!*