# 🎓 Step 3: Fleet Analytics with Custom VSS Tutorial

## 🎯 Learning Objectives

In this advanced tutorial, you will master:
- **Custom VSS Signal Creation** - Design and implement custom Vehicle Signal Specification
- **Enterprise Fleet Management** - Build production-ready fleet management systems
- **Multi-dimensional Analytics** - Process driver, route, cargo, and environmental data
- **Custom Business Logic** - Implement domain-specific vehicle intelligence
- **Advanced Signal Processing** - Handle complex custom signal architectures

## 📋 Prerequisites

- ✅ Completed Step 1 & Step 2 tutorials
- ✅ Understanding of standard VSS signals
- ✅ Basic knowledge of fleet management concepts
- ✅ KUKSA Databroker running and accessible

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

### Step 1: Build with Custom VSS

```bash
# Build container with custom VSS specification
docker build -f Dockerfile.quick \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --network=host \
  -t velocitas-quick .
```

### Step 2: Start KUKSA Databroker

```bash
# Start vehicle data broker
docker run -d --rm --name kuksa-databroker --network host \
  ghcr.io/eclipse-kuksa/kuksa-databroker:main
```

### Step 3: Build Custom VSS Fleet Application

```bash
# Build with custom VSS specification
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_FleetAnalytics.cpp:/app.cpp \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  velocitas-quick build --verbose
```

### Step 4: Run Fleet Analytics Application

```bash
# Run fleet analytics with custom VSS
docker run -d --network host --name fleet-analytics \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_FleetAnalytics.cpp:/app.cpp \
  -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
  velocitas-quick run 300
```

## 📊 Testing Custom VSS Signals

### Test 1: Fleet Driver Performance Analysis

```bash
# Test excellent driving performance
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check fleet analytics logs
docker logs fleet-analytics --tail 10
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
docker logs fleet-analytics --tail 15 | grep -E "(WARNING|performance|Score)"
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
docker logs fleet-analytics --tail 10 | grep -E "(CARGO|temperature|CRITICAL)"
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
docker logs fleet-analytics --tail 10 | grep -E "(Route|GPS|optimization)"
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
docker logs fleet-analytics --tail 10 | grep -E "(Environment|AQI|Noise|Road)"
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
docker logs fleet-analytics --tail 20 | grep -E "(Priority|Dispatch|EMERGENCY)"
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
docker logs fleet-analytics | grep "Driver Score" | tail -10
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
docker logs fleet-analytics --tail 15 | grep -E "(AQI|Environment|POOR_AIR)"
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