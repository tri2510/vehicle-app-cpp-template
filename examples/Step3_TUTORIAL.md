# 🎓 Step 3: Fleet Analytics with Custom VSS Tutorial

## 🎯 Learning Objectives

In this advanced tutorial, you will master:
- **Custom VSS Signal Creation** - Design and implement custom Vehicle Signal Specification
- **Enterprise Fleet Management** - Build production-ready fleet management systems
- **Multi-dimensional Analytics** - Process driver, route, cargo, and environmental data
- **Custom Business Logic** - Implement domain-specific vehicle intelligence
- **Advanced Signal Processing** - Handle complex custom signal architectures

## 📋 Prerequisites

**Required Setup:**
```bash
# Option A: KUKSA with Custom VSS (enables real custom signal testing)
docker stop velocitas-vdb 2>/dev/null || true
docker run -d --rm --name kuksa-custom-vss --network host \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  ghcr.io/eclipse-kuksa/kuksa-databroker:main \
  --address 0.0.0.0 --port 55555 --insecure \
  --vss /custom_vss.json

# Option B: Standard KUKSA (simulation only)
# docker ps | grep velocitas-vdb || docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create fresh persistent volumes for Step 3 (isolated from other steps)
docker volume rm step3-build step3-deps step3-vss 2>/dev/null || true
docker volume create step3-build
docker volume create step3-deps  
docker volume create step3-vss
```

**Container Image Options:**
```bash
# Option A: Use pre-built image (RECOMMENDED - instant start!)
# Throughout this tutorial, we'll use:
# ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest

# Option B: Build locally (if you haven't already)
docker build -f Dockerfile.quick -t velocitas-quick .
```

**Learning Prerequisites:**
- ✅ Completed Step 1 & Step 2 tutorials
- ✅ Understanding of standard VSS signals
- ✅ Basic knowledge of fleet management concepts

## 🏗️ Architecture Overview

This tutorial demonstrates a **3-tier custom VSS architecture**:

1. **Standard VSS Layer** - Reliable base signals (Speed, Location, Temperature)
2. **Custom VSS Layer** - Business-specific signals (Fleet, Analytics, Operations, Cargo)
3. **Application Layer** - Enterprise fleet management logic

```
┌─────────────────────────────────────────────────────────────┐
│                    Fleet Analytics Application               │
├─────────────────────────────────────────────────────────────┤
│  Custom VSS Signals (Fleet Management + Analytics + Cargo)  │
├─────────────────────────────────────────────────────────────┤
│           Standard VSS Signals (Speed, GPS, Temp)           │
├─────────────────────────────────────────────────────────────┤
│                    KUKSA Databroker                        │
└─────────────────────────────────────────────────────────────┘
```

## 🔧 Custom VSS Specification

Our custom VSS extends the standard specification with **5 new signal branches**:

### 1. **Fleet Management Signals**
- `Vehicle.FleetManagement.FleetID` - Fleet identifier
- `Vehicle.FleetManagement.DriverID` - Driver authentication
- `Vehicle.FleetManagement.RouteID` - Route assignment
- `Vehicle.FleetManagement.TripID` - Trip session tracking
- `Vehicle.FleetManagement.VehicleStatus` - Operational status
- `Vehicle.FleetManagement.DispatchPriority` - Priority level (1-10)

### 2. **Analytics Signals**
- `Vehicle.Analytics.DrivingScore` - Real-time performance (0-100)
- `Vehicle.Analytics.EcoEfficiency` - Fuel efficiency score
- `Vehicle.Analytics.SafetyRating` - Safety compliance rating
- `Vehicle.Analytics.AlertLevel` - System alert level (0-4)
- `Vehicle.Analytics.MaintenanceScore` - Predictive maintenance

### 3. **Operations Signals**
- `Vehicle.Operations.RouteOptimization.TimeEfficiency` - Route time performance
- `Vehicle.Operations.RouteOptimization.FuelEfficiency` - Route fuel performance
- `Vehicle.Operations.RouteOptimization.TrafficFactor` - Traffic impact
- `Vehicle.Operations.Communication.*` - Fleet communication metrics

### 4. **Cargo Monitoring Signals**
- `Vehicle.CustomSensors.CargoStatus.LoadWeight` - Cargo weight (kg)
- `Vehicle.CustomSensors.CargoStatus.LoadPercentage` - Capacity utilization
- `Vehicle.CustomSensors.CargoStatus.CargoType` - Cargo classification
- `Vehicle.CustomSensors.CargoStatus.CargoTemperature` - Cargo temperature

### 5. **Environmental Signals**
- `Vehicle.CustomSensors.Environmental.AirQualityIndex` - Local AQI
- `Vehicle.CustomSensors.Environmental.NoiseLevel` - Ambient noise
- `Vehicle.CustomSensors.Environmental.RoadCondition` - Surface conditions

## 🚀 Quick Start

### **Phase 1: Build Custom VSS Fleet Application**

```bash
# Build with custom VSS specification and persistent volumes
docker run --rm --network host \
  -v step3-build:/quickbuild/build \
  -v step3-deps:/home/vscode/.conan2 \
  -v step3-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_FleetAnalytics.cpp:/app.cpp \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: ~15M (with custom VSS)
🎉 Custom VSS Fleet Analytics build completed!
```

### **Phase 2: Run Fleet Analytics Application**

```bash
# Run fleet analytics with custom VSS and persistent volumes
docker run -d --network host --name step3-fleet-analytics \
  -v step3-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_FleetAnalytics.cpp:/app.cpp \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 300
```

**Expected Runtime Output:**
```
🎓 Step 3: Custom VSS Fleet Analytics starting...
📡 Connecting to Vehicle Data Broker...
✅ Custom VSS Fleet Analytics initialized
🚀 Step 3: Starting Custom VSS Fleet Analytics!
📊 Custom VSS signals: Fleet, Analytics, Operations, Cargo, Environmental
🔄 Educational: Real Speed + Simulated Custom VSS signals
```

### **Phase 3: Faster Subsequent Builds (Optional)**

After the first build, use `--skip-deps` for much faster rebuilds:

```bash
# Stop previous container
docker stop step3-fleet-analytics && docker rm step3-fleet-analytics

# Fast rebuild (15-30 seconds instead of 2-3 minutes)
docker run --rm --network host \
  -v step3-build:/quickbuild/build \
  -v step3-deps:/home/vscode/.conan2 \
  -v step3-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_FleetAnalytics.cpp:/app.cpp \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose

# Run with optimized container
docker run -d --network host --name step3-fleet-analytics \
  -v step3-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 300
```

## 📊 Testing Custom VSS Signals

### **Real Custom VSS Signal Testing** (Advanced - Requires Custom KUKSA + Updated App)

> **⚠️ Note**: The current Step 3 application subscribes to standard VSS signals and simulates custom signals internally. 
> To test actual custom VSS signal injection, you would need to:
> 1. Use Option A (Custom KUKSA with VSS file)
> 2. Update the C++ application to subscribe to custom signals
> 3. Regenerate vehicle model with custom VSS specification
> 
> **For learning purposes**, the commands below show how you WOULD test custom signals if fully implemented:

**Test 1: Fleet Management Signals**

```bash
# Test custom fleet management signals
echo "setValue Vehicle.FleetManagement.FleetID FLEET_ALPHA_001" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.FleetManagement.DriverID DRIVER_12345" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.FleetManagement.RouteID ROUTE_NYC_BOS_001" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check fleet management processing
docker logs step3-fleet-analytics --tail 10 | grep -E "(Fleet|Driver|Route)"
```

**Test 2: Analytics Signals**

```bash
# Test custom analytics signals
echo "setValue Vehicle.Analytics.DrivingScore 85.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Analytics.EcoEfficiency 92.3" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Analytics.SafetyRating 96.7" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check analytics processing
docker logs step3-fleet-analytics --tail 10 | grep -E "(Analytics|Score|Efficiency|Safety)"
```

**Test 3: Cargo Monitoring**

```bash
# Test custom cargo signals
echo "setValue Vehicle.CustomSensors.CargoStatus.LoadWeight 2500.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CustomSensors.CargoStatus.LoadPercentage 75.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CustomSensors.CargoStatus.CargoType ELECTRONICS" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check cargo processing
docker logs step3-fleet-analytics --tail 10 | grep -E "(Cargo|Load|Weight)"
```

**Expected Custom VSS Output:**
```
📊 Real Custom VSS Signal: FleetID=FLEET_ALPHA_001 | DriverID=DRIVER_12345
📈 Analytics Update: DrivingScore=85.5% | EcoEfficiency=92.3% | Safety=96.7%
📦 Cargo Update: ELECTRONICS 2500kg (75.0%) | Real Custom VSS Active!
🎉 SUCCESS: Custom VSS signals processed from KUKSA databroker!
```

### **Why This Approach is Educational**

The current Step 3 tutorial uses a **hybrid approach** that's ideal for learning:

#### **✅ What Works Now (Standard Signals + Simulation):**
- **Real KUKSA Integration**: Uses actual Vehicle.Speed, GPS, and temperature signals
- **Custom VSS Architecture**: Demonstrates custom signal structure and organization  
- **Business Logic**: Shows how to process and correlate custom fleet data
- **Production Patterns**: Uses real enterprise fleet management patterns

#### **🔧 What Full Implementation Would Add:**
- **Direct Custom Signal Subscription**: C++ app subscribes to Vehicle.FleetManagement.* signals
- **Real Custom Signal Processing**: Actual signal callbacks for custom VSS data
- **End-to-End Custom VSS**: Complete pipeline from KUKSA injection to application processing

#### **📚 Learning Value:**
- **Understand Custom VSS Concepts**: Learn signal organization and structure
- **See Production Architecture**: Enterprise fleet patterns and data correlation
- **Rapid Development**: No complex vehicle model regeneration required
- **Real Signal Processing**: Experience actual KUKSA signal subscription patterns

---

### **Standard Signal Testing** (Works with both Option A & B)

### Test 4: Fleet Driver Performance Analysis

```bash
# Test excellent driving performance
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check fleet analytics logs
docker logs step3-fleet-analytics --tail 10
```

**Expected Output:**
```
📊 Custom VSS Update: Driver Score 89.2% | Eco Efficiency 94.1% | Priority 5
👤 Driver DRIVER_12345: Score 89.2% | Safety 95.0% | Eco 94.1%
⭐ Performance Rating: Good
```

### Test 2: High-Speed Performance Penalty

```bash
# Test high-speed driving (performance penalty)
echo "setValue Vehicle.Speed 40.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check for performance warnings
docker logs step3-fleet-analytics --tail 15 | grep -E "(WARNING|performance|Score)"
```

**Expected Output:**
```
📊 Custom VSS Update: Driver Score 85.2% | Eco Efficiency 88.1% | Priority 5
⚠️  [WARNING|DRIVER_ANALYTICS] Driver DRIVER_12345 performance below threshold: 72.3%
```

### Test 3: Cargo Temperature Monitoring

```bash
# Test extreme cold affecting cargo
echo "setValue Vehicle.Exterior.AirTemperature -30.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check cargo temperature alerts
docker logs step3-fleet-analytics --tail 10 | grep -E "(CARGO|temperature|CRITICAL)"
```

**Expected Output:**
```
📦 Cargo ELECTRONICS: 2500kg (75.0%) | Temp -15.0°C
🚨 [CRITICAL|CARGO] Cargo temperature out of range: -15.0°C (Safe: -20.0°C to 25.0°C)
```

### Test 4: GPS Route Tracking

```bash
# Set GPS coordinates (New York City - Times Square)
echo "setValue Vehicle.CurrentLocation.Latitude 40.7589" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CurrentLocation.Longitude -73.9851" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check route optimization metrics
docker logs step3-fleet-analytics --tail 10 | grep -E "(Route|GPS|optimization)"
```

**Expected Output:**
```
🗺️  Route ROUTE_NYC_BOS_001: Time Eff 96.7% | Fuel Eff 92.1% | Traffic 1.0x
📍 Route: ROUTE_NYC_BOS_001 | GPS: (40.758900, -73.985100)
```

### Test 5: Environmental Impact Monitoring

```bash
# Test high temperature impact
echo "setValue Vehicle.Exterior.AirTemperature 35.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check environmental analytics
docker logs step3-fleet-analytics --tail 10 | grep -E "(Environment|AQI|Noise|Road)"
```

**Expected Output:**
```
🌍 Environment: AQI 65 | Noise 73.5dB | Road DRY
🌡️  Air Temp: 35.0°C | AQI: 65 | Noise: 73.5dB | Road: DRY
```

## 📊 Comprehensive Fleet Dashboard

Every 45 seconds, the application generates a comprehensive fleet report:

```
📊 ========== CUSTOM VSS FLEET ANALYTICS DASHBOARD ==========
🏢 === FLEET OVERVIEW ===
🚛 Fleet ID: FLEET_ALPHA_001 | Trip: TRIP_1719376800 | Status: ACTIVE
📊 KPIs: Trips 3 | Alerts 7 | Priority Dispatches 1 | Maintenance Events 0
⭐ Fleet Averages: Speed 89.4 km/h | Driving Score 87.2% | Fuel Efficiency 91.8%

👤 === DRIVER ANALYTICS ===
🆔 Driver: DRIVER_12345 | Score: 87.2% | Safety: 95.0% | Eco: 91.8%
⭐ Performance Rating: Good

🗺️  === ROUTE OPTIMIZATION ===
📍 Route: ROUTE_NYC_BOS_001 | GPS: (40.758900, -73.985100)
⏱️  Time Efficiency: 96.7% | Fuel Efficiency: 91.8% | Traffic Factor: 1.0x
📶 Communication: Signal 97% | Data 12.5MB | Priority 5

📦 === CARGO OPERATIONS ===
🏷️  Type: ELECTRONICS | Weight: 2500kg | Capacity: 75.0%
🌡️  Temperature: 25.5°C | Total Handled: 250000kg

🌍 === ENVIRONMENTAL IMPACT ===
🌡️  Air Temp: 35.0°C | AQI: 65 | Noise: 73.5dB | Road: DRY
============================================================
```

## 🔧 Advanced Testing Scenarios

### Scenario 1: Fleet Dispatch Priority Testing

```bash
# Simulate medical emergency cargo (high priority)
# This would require custom VSS signal injection in production
# For this tutorial, the app simulates priority based on cargo type

# Monitor dispatch priority changes
docker logs step3-fleet-analytics --tail 20 | grep -E "(Priority|Dispatch|EMERGENCY)"
```

### Scenario 2: Multi-Driver Performance Comparison

```bash
# Test different speed patterns to see driver scoring
for speed in 20 35 15 40 25 45 10; do
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 3
done

# Check driver score trends
docker logs step3-fleet-analytics | grep "Driver Score" | tail -10
```

### Scenario 3: Environmental Compliance Testing

```bash
# Test air quality impact (simulated based on speed)
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555  # City driving (higher AQI)

sleep 5

echo "setValue Vehicle.Speed 30.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555  # Highway (lower AQI)

# Check environmental metrics
docker logs step3-fleet-analytics --tail 15 | grep -E "(AQI|Environment|POOR_AIR)"
```

## 🎓 Learning Outcomes

After completing this tutorial, you will have learned:

### ✅ **Custom VSS Signal Architecture**
- How to define custom VSS specifications in JSON format
- Organizing signals into logical business domains
- Extending standard VSS with enterprise-specific signals

### ✅ **Enterprise Fleet Management Patterns**
- Multi-dimensional vehicle analytics (driver, route, cargo, environment)
- Real-time fleet operations monitoring
- Advanced KPI calculation and reporting

### ✅ **Production-Ready Application Development**
- Handling multiple signal subscriptions efficiently
- Implementing complex business logic with custom signals
- Building comprehensive alerting and reporting systems

### ✅ **Custom VSS Integration Techniques**
- Using environment variables for custom VSS specifications
- Combining standard and custom signals in one application
- Simulating custom signals for development and testing

## 🔄 Next Steps

### **Production Deployment**
1. **Deploy Custom VSS to KUKSA** - Load your custom VSS specification into production KUKSA instances
2. **Implement Real Signal Sources** - Replace simulation with actual custom signal sources
3. **Scale to Multi-Vehicle** - Extend to handle multiple vehicles in your fleet
4. **Integrate with Business Systems** - Connect to ERP, dispatch, and analytics platforms

### **Advanced Customization**
1. **Industry-Specific Signals** - Create signals for your specific industry (logistics, emergency services, etc.)
2. **Regulatory Compliance** - Add signals for compliance monitoring (DOT, environmental regulations)
3. **Predictive Analytics** - Implement machine learning with custom signal data
4. **Real-time Dashboards** - Build web dashboards consuming custom VSS data

## 📚 Additional Resources

- **VSS Specification**: [COVESA Vehicle Signal Specification](https://github.com/COVESA/vehicle_signal_specification)
- **KUKSA Documentation**: [Eclipse KUKSA](https://github.com/eclipse-kuksa)
- **Velocitas Framework**: [Eclipse Velocitas](https://github.com/eclipse-velocitas)
- **Custom VSS Examples**: Check the `custom_fleet_vss.json` for detailed signal definitions

## 🎉 Congratulations!

You have successfully mastered **Custom VSS Fleet Analytics**! You now have the skills to:
- Design custom vehicle signal specifications for any business domain
- Build enterprise-grade fleet management applications
- Process complex multi-dimensional vehicle data
- Implement production-ready vehicle intelligence systems

**Your custom VSS application is ready for enterprise deployment!** 🚀