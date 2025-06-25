# 🎓 Step 3: Data Analysis & Alerts Tutorial

## 📚 Learning Objectives

In this advanced step, you'll master sophisticated vehicle data analytics:

- **Historical Analysis**: Implement rolling windows for trend detection
- **Pattern Recognition**: Detect harsh braking, rapid acceleration, erratic driving
- **Alert Framework**: Build multi-tier alert system with priorities
- **Predictive Analytics**: Anticipate maintenance needs and fuel consumption

**Difficulty**: ⭐⭐⭐ Advanced | **Time**: 45 minutes

## 🎯 What You'll Build

An advanced analytics application that:
- Tracks historical data with 50-point rolling windows
- Detects complex driving patterns using statistical analysis
- Implements INFO/WARNING/CRITICAL/EMERGENCY alert levels
- Calculates driver behavior scores
- Generates predictive maintenance recommendations
- Produces comprehensive vehicle health reports

## 📋 Prerequisites

**Required Setup:**
```bash
# Ensure KUKSA databroker is running
docker ps | grep velocitas-vdb || docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create dedicated persistent volumes for Step 3 (isolated from other steps)
docker volume ls | grep step3-build || docker volume create step3-build
docker volume ls | grep step3-deps || docker volume create step3-deps  
docker volume ls | grep step3-vss || docker volume create step3-vss
```

**Container Image:**
```bash
# We'll use the pre-built image throughout this tutorial:
# ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest
```

## 🧪 Step-by-Step Tutorial

### **Phase 1: Build the Analytics Application**

**Using Pre-built Image:**
```bash
# Build Step 3 application with analytics capabilities
docker run --rm --network host \
  -v step3-build:/quickbuild/build \
  -v step3-deps:/home/vscode/.conan2 \
  -v step3-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step3_DataAnalysisAlerts.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] Source validated: /app.cpp (620 lines)
✅ [SUCCESS] Vehicle model exists, skipping
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
🎉 Build completed successfully!
```

### **Phase 2: Run the Application**

```bash
# Start analytics app with extended runtime for testing
docker run -d --network host --name step3-app \
  -v step3-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 180
```

**Monitor Application:**
```bash
# Watch analytics and alerts in real-time
docker logs step3-app --follow
```

**Expected Startup Logs:**
```
🎓 Step 3: Starting Data Analysis & Alerts Tutorial
🎯 Learning Goal: Master advanced analytics & alerts
📊 Features: Pattern detection, predictive analytics
🎓 Step 3: Data Analysis & Alerts starting...
📊 Learning objective: Advanced pattern analysis
🚨 Features: Multi-tier alerts, predictive analytics
✅ Data Analysis & Alerts initialized
🚀 Step 3: Starting Data Analysis & Alerts!
📊 Initializing historical data tracking...
✅ Analytics signal subscriptions active
🔄 Collecting data for pattern analysis...
💡 Generate varied driving patterns to see analytics!
```

### **Phase 3: Pattern Detection Testing**

**Test 1: Normal Driving Baseline**
```bash
# Establish normal driving pattern
for speed in 20 22 24 25 23 22 24; do
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  echo "setValue Vehicle.Powertrain.Engine.Speed $((speed * 100))" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 1
done

# Set fuel level
echo "setValue Vehicle.Powertrain.FuelSystem.Level 65.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

**Expected Normal Driving Output:**
```
📊 Current: Speed 86.4 km/h | RPM 2400 | Fuel 65.0%
✅ Stable driving pattern detected
```

**Test 2: Harsh Braking Detection**
```bash
# Simulate harsh braking event
echo "setValue Vehicle.Speed 30.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
sleep 2
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check harsh braking alert
docker logs step3-app --tail 10
```

**Expected Harsh Braking Alert:**
```
📊 Current: Speed 18.0 km/h | RPM 2400 | Fuel 65.0%
🚨 HARSH BRAKING: -12.5 m/s² (threshold: -5.0)
⚠️  [WARNING] Harsh braking detected: -12.5 m/s²
```

**Test 3: Rapid Acceleration Detection**
```bash
# Simulate rapid acceleration
echo "setValue Vehicle.Speed 10.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
sleep 1
echo "setValue Vehicle.Speed 35.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check acceleration alert
docker logs step3-app --tail 10
```

**Expected Rapid Acceleration Alert:**
```
📊 Current: Speed 126.0 km/h | RPM 2400 | Fuel 65.0%
⚡ RAPID ACCELERATION: 25.0 m/s² (threshold: 4.0)
⚠️  [WARNING] Rapid acceleration: 25.0 m/s²
```

**Test 4: Erratic Driving Pattern**
```bash
# Generate erratic speed changes
for speed in 10 30 15 40 20 45 10 35 25; do
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 2
done

# Check erratic driving detection
docker logs step3-app --tail 15
```

**Expected Erratic Driving Alert:**
```
📊 Current: Speed 90.0 km/h | RPM 2400 | Fuel 65.0%
🔄 ERRATIC DRIVING: Speed variance 22.7 km/h exceeds threshold
🚨 [CRITICAL] Erratic driving pattern detected. Speed variance: 22.7 km/h
```

**Test 5: Low Fuel Predictive Alert**
```bash
# Set low fuel and monitor consumption
echo "setValue Vehicle.Powertrain.FuelSystem.Level 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Simulate fuel consumption
for fuel in 24 23 22 20 18 16; do
  echo "setValue Vehicle.Powertrain.FuelSystem.Level $fuel.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 3
done

# Check predictive fuel alert
docker logs step3-app --tail 10
```

**Expected Predictive Alert:**
```
📊 Current: Speed 90.0 km/h | RPM 2400 | Fuel 16.0%
🚨 [CRITICAL] Fuel critically low! Estimated 24 minutes remaining
```

**Test 6: Engine Anomaly Detection**
```bash
# Normal RPM pattern
echo "setValue Vehicle.Powertrain.Engine.Speed 2500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Sudden RPM spike (anomaly)
echo "setValue Vehicle.Powertrain.Engine.Speed 5500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check engine anomaly alert
docker logs step3-app --tail 10
```

**Expected Engine Anomaly Alert:**
```
📊 Current: Speed 90.0 km/h | RPM 5500 | Fuel 16.0%
🔧 ENGINE ANOMALY: RPM deviation 3.2σ from normal
⚠️  [WARNING] Engine RPM anomaly detected: 3.2 std deviations
```

**Test 7: Driver Behavior Report**
```bash
# Wait for periodic report (every 30 seconds)
# Or generate more events to trigger report
sleep 35

# Check driver behavior report
docker logs step3-app --tail 20 | grep -A 10 "DRIVER BEHAVIOR"
```

**Expected Driver Report:**
```
📋 === DRIVER BEHAVIOR REPORT ===
🏁 Driver Score: 84.5/100
🚨 Harsh Braking Events: 2
⚡ Rapid Acceleration Events: 3
🔄 Erratic Driving Events: 1
⭐ Overall Rating: Good
================================
```

## 🔍 Code Analysis & Advanced Patterns

### **Key Patterns You Learned:**

**1. Rolling Window Data Structure:**
```cpp
struct HistoricalData {
    std::deque<DataPoint> speed;
    std::deque<DataPoint> rpm;
    const size_t maxSize = 50;  // Keep last 50 points
    
    void addDataPoint(std::deque<DataPoint>& container, double value) {
        container.push_back({value, std::chrono::steady_clock::now()});
        if (container.size() > maxSize) {
            container.pop_front();
        }
    }
};
```

**2. Statistical Analysis:**
```cpp
double calculateStandardDeviation(const std::deque<DataPoint>& data) {
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    for (double value : values) {
        variance += std::pow(value - mean, 2);
    }
    return std::sqrt(variance / values.size());
}
```

**3. Multi-Tier Alert System:**
```cpp
enum class AlertPriority {
    INFO = 0,
    WARNING = 1,
    CRITICAL = 2,
    EMERGENCY = 3
};

void createAlert(const std::string& id, const std::string& message,
                AlertPriority priority, double value);
```

**4. Acceleration Calculation:**
```cpp
void calculateAcceleration() {
    if (speed.size() >= 2) {
        auto timeDiff = duration_cast<milliseconds>(
            latest.timestamp - previous.timestamp).count() / 1000.0;
        double accel = (latest.value - previous.value) / timeDiff;
    }
}
```

**5. Anomaly Detection (Z-Score):**
```cpp
double detectAnomaly(double value, const std::deque<DataPoint>& history) {
    double mean = calculateMean(history);
    double stdDev = calculateStandardDeviation(history);
    return (value - mean) / stdDev;  // Z-score
}
```

## ✅ Success Criteria Validation

### **✅ Pattern Detection:**
- **PASS**: Harsh braking detection with threshold -5.0 m/s²
- **PASS**: Rapid acceleration detection with threshold 4.0 m/s²
- **PASS**: Erratic driving pattern analysis using variance
- **PASS**: Engine anomaly detection using Z-score

### **✅ Alert System:**
- **PASS**: Multi-tier alerts (INFO/WARNING/CRITICAL/EMERGENCY)
- **PASS**: Alert aging and automatic removal after 5 minutes
- **PASS**: Priority-based alert processing
- **PASS**: Contextual alert messages with values

### **✅ Analytics Features:**
- **PASS**: Rolling window data storage (50 points)
- **PASS**: Statistical calculations (mean, std dev, trend)
- **PASS**: Driver behavior scoring system
- **PASS**: Predictive fuel consumption alerts
- **PASS**: Periodic reporting (30-second intervals)

## 📊 Performance Benchmarks

| Metric | Expected Value | Your Result |
|--------|----------------|-------------|
| **Build Time** | 15-30 seconds | ⏱️ _____ |
| **Memory Usage** | ~100-150MB | 💾 _____ |
| **Alert Response** | <200ms | ⚡ _____ |
| **Pattern Detection** | <500ms | 🔍 _____ |
| **Report Generation** | <100ms | 📊 _____ |

## 🐛 Troubleshooting

### **Issue 1: No Pattern Detection**
```bash
# Ensure enough data points collected (minimum 5-10)
# Generate more signal updates before expecting patterns
```

### **Issue 2: No Reports Generated**
```bash
# Reports generate every 30 seconds
# Check time since app start:
docker logs step3-app | grep "Starting Data Analysis"
```

### **Issue 3: Alerts Not Appearing**
```bash
# Verify thresholds are being exceeded
# Harsh braking: > 5 m/s² deceleration
# Rapid accel: > 4 m/s² acceleration
# Check current values in logs
```

## 🧹 Cleanup

```bash
# Stop the analytics application
docker stop step3-app && docker rm step3-app

# Optional: Clean up volumes if done with Step 3
# docker volume rm step3-build step3-deps step3-vss
```

## 🎓 Knowledge Check

**Before proceeding to Step 4, ensure you understand:**

1. **Rolling Windows**: Using `std::deque` for fixed-size historical data
2. **Statistical Analysis**: Calculating mean, standard deviation, trends
3. **Pattern Detection**: Identifying harsh events and erratic behavior
4. **Alert Management**: Multi-tier system with priority levels
5. **Predictive Analytics**: Using trends to anticipate future states
6. **Driver Scoring**: Quantifying behavior with penalty system

## 🚀 What's Next?

**Ready for Step 4?** You'll build a production-ready fleet management system:

- **GPS Integration**: Location-based speed zones and geofencing
- **Fleet Analytics**: Multi-vehicle tracking and comparison
- **Compliance Reporting**: Regulatory compliance automation
- **REST API**: External system integration
- **Dashboard Ready**: Real-time fleet visualization data

**[Continue to Step 4: Advanced Fleet Management →](Step4_TUTORIAL.md)**

---

## 📈 Step 3 Completion Checklist

- ✅ Built Step3_DataAnalysisAlerts application successfully
- ✅ Implemented rolling window historical tracking
- ✅ Detected harsh braking and rapid acceleration events
- ✅ Identified erratic driving patterns using variance
- ✅ Created multi-tier alert system with priorities
- ✅ Generated driver behavior scores and reports
- ✅ Implemented predictive fuel consumption alerts
- ✅ Understood statistical analysis patterns

**🎉 Congratulations! You've mastered advanced vehicle data analytics!**

---

*🚗💨 Detect patterns and predict vehicle behavior with advanced analytics!*