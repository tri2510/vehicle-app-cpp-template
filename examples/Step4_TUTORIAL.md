# 🎓 Step 4: Advanced Fleet Manager Tutorial

## 📚 Learning Objectives

In this final step, you'll build a production-ready fleet management system:

- **Fleet Architecture**: Design scalable multi-vehicle tracking systems
- **GPS Geofencing**: Implement location-based speed zones and compliance
- **Maintenance Scheduling**: Create predictive maintenance systems
- **Enterprise Reporting**: Build comprehensive fleet analytics dashboards

**Difficulty**: ⭐⭐⭐⭐ Professional | **Time**: 60 minutes

## 🎯 What You'll Build

A complete fleet management application that:
- Tracks multiple vehicles with unique IDs and real-time status
- Enforces GPS-based speed limits across different zones
- Generates compliance reports for regulatory requirements
- Schedules predictive maintenance based on usage patterns
- Ranks driver performance across the fleet
- Provides enterprise-grade reporting and analytics

## 📋 Prerequisites

**Required Setup:**
```bash
# Ensure KUKSA databroker is running
docker ps | grep velocitas-vdb || docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create dedicated persistent volumes for Step 4 (isolated from other steps)
docker volume ls | grep step4-build || docker volume create step4-build
docker volume ls | grep step4-deps || docker volume create step4-deps  
docker volume ls | grep step4-vss || docker volume create step4-vss
```

**Container Image:**
```bash
# We'll use the pre-built image:
# ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest
```

## 🧪 Step-by-Step Tutorial

### **Phase 1: Build Fleet Manager**

```bash
# Build Step 4 fleet management application
docker run --rm --network host \
  -v step4-build:/quickbuild/build \
  -v step4-deps:/home/vscode/.conan2 \
  -v step4-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/Step4_AdvancedFleetManager.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
```

**Expected Build Output:**
```
✅ [SUCCESS] Source validated: /app.cpp (800+ lines)
✅ [SUCCESS] Vehicle model exists, skipping
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
🎉 Build completed successfully!
```

### **Phase 2: Run Fleet Manager**

```bash
# Start fleet manager with extended runtime
docker run -d --network host --name step4-app \
  -v step4-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 300
```

**Monitor Fleet Operations:**
```bash
# Watch fleet management in action
docker logs step4-app --follow
```

**Expected Startup Logs:**
```
🎓 Step 4: Starting Advanced Fleet Manager Tutorial
🎯 Learning Goal: Build production fleet system
🚛 Features: GPS zones, compliance, maintenance
🎓 Step 4: Advanced Fleet Manager starting...
🚚 Initializing fleet management system...
📡 Connecting to Vehicle Data Broker...
🗺️  Setting up GPS zones and geofencing...
📊 Configuring fleet analytics engine...
✅ Fleet Manager initialized for vehicle: FLEET-001
🏢 Ready for enterprise fleet management
🚀 Step 4: Starting Advanced Fleet Manager!
🗺️  Loaded 5 GPS speed zones
✅ Fleet management signals configured
📍 GPS zones active: 5
```

### **Phase 3: GPS Zone Testing**

**Test 1: School Zone Violation**
```bash
# Set location to school zone (30 km/h limit)
echo "setValue Vehicle.CurrentLocation.Latitude 40.7585" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CurrentLocation.Longitude -73.9850" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Set speed above school zone limit
echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check violation alert
docker logs step4-app --tail 10
```

**Expected School Zone Alert:**
```
🚛 Vehicle FLEET-001 Status: Speed 54.0 km/h | Fuel 0.0% | Location (40.758500, -73.985000)
📍 Vehicle FLEET-001 in Downtown School Zone: Speed 54.0/30.0 km/h
🚨 SPEED VIOLATION: Vehicle FLEET-001 - 54.0 km/h in Downtown School Zone
🏫 CRITICAL: Vehicle FLEET-001 speeding in SCHOOL ZONE!
```

**Test 2: Highway Zone Compliance**
```bash
# Move to highway zone (100 km/h limit)
echo "setValue Vehicle.CurrentLocation.Latitude 40.7650" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CurrentLocation.Longitude -73.9250" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Set highway speed
echo "setValue Vehicle.Speed 27.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check compliance
docker logs step4-app --tail 5
```

**Expected Highway Compliance:**
```
🚛 Vehicle FLEET-001 Status: Speed 97.2 km/h | Fuel 0.0% | Location (40.765000, -73.925000)
📍 Vehicle FLEET-001 in Highway I-495: Speed 97.2/100.0 km/h
```

**Test 3: Fuel Management**
```bash
# Set fuel level and monitor efficiency
echo "setValue Vehicle.Powertrain.FuelSystem.Level 85.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Simulate fuel consumption
for fuel in 84 82 80 77 74 70; do
  echo "setValue Vehicle.Powertrain.FuelSystem.Level $fuel.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 3
done

# Set engine data
echo "setValue Vehicle.Powertrain.Engine.Speed 2800" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check fuel efficiency
docker logs step4-app --tail 10
```

**Expected Fuel Tracking:**
```
🚛 Vehicle FLEET-001 Status: Speed 97.2 km/h | Fuel 70.0% | Location (40.765000, -73.925000)
⛽ Vehicle FLEET-001 fuel efficiency: 12.5 L/100km
```

**Test 4: Maintenance Scheduling**
```bash
# Set odometer reading (triggers maintenance check)
echo "setValue Vehicle.OBD.DistanceWithMIL 15500" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Simulate harsh driving (affects brake maintenance)
echo "setValue Vehicle.Speed 35.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
sleep 2
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Wait for next report cycle (60 seconds) or check logs
sleep 5
docker logs step4-app --tail 30 | grep -A 20 "MAINTENANCE"
```

**Expected Maintenance Alert:**
```
🔧 MAINTENANCE DUE: Vehicle FLEET-001 - Oil Change
🔧 MAINTENANCE DUE: Vehicle FLEET-001 - Tire Rotation
```

**Test 5: Fleet Reports Generation**
```bash
# Generate varied driving data for comprehensive reports
# City driving
echo "setValue Vehicle.CurrentLocation.Latitude 40.7575" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
echo "setValue Vehicle.CurrentLocation.Longitude -73.9860" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

for speed in 10 12 8 15 11 13; do
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 5
done

# Wait for report generation (every 60 seconds)
sleep 30

# View comprehensive reports
docker logs step4-app --tail 60 | grep -A 50 "==="
```

**Expected Fleet Reports:**
```
📊 === FLEET MANAGEMENT REPORT ===
🚛 Total Vehicles: 1 (Active: 1)
📍 Total Distance: 2.5 km
🚗 Average Speed: 46.8 km/h
⛽ Average Fuel Efficiency: 12.5 L/100km
🚨 Active Alerts: 3 (Critical: 1)
⭐ Fleet Score: 80.0/100
⏱️  System Uptime: 00:02:35
==================================

📋 === COMPLIANCE REPORT ===
🚗 Vehicle FLEET-001: 2 speed violations
📊 Total Fleet Violations: 2
📍 SCHOOL Zone Violations: 1
📍 CITY Zone Violations: 1
✅ Compliance Rate: 92.0%
============================

🔧 === MAINTENANCE REPORT ===
⚠️  FLEET-001 - Oil Change OVERDUE
⚠️  FLEET-001 - Tire Rotation OVERDUE
📊 Overdue Services: 2
🔧 Oil Change: 1 vehicles
🔧 Tire Rotation: 1 vehicles
=============================

🏆 === DRIVER RANKINGS ===
1. Vehicle FLEET-001 - Score: 80.0/100 ⭐⭐⭐⭐
==========================
```

**Test 6: Multi-Zone Journey**
```bash
# Simulate journey through multiple zones
zones=(
  "40.7585,-73.9850,10"  # School zone
  "40.7575,-73.9860,12"  # City zone
  "40.7650,-73.9250,28"  # Highway zone
  "40.7300,-74.0050,15"  # Industrial zone
  "40.7150,-73.9550,11"  # Residential zone
)

for zone in "${zones[@]}"; do
  IFS=',' read -r lat lon speed <<< "$zone"
  echo "setValue Vehicle.CurrentLocation.Latitude $lat" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  echo "setValue Vehicle.CurrentLocation.Longitude $lon" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  echo "setValue Vehicle.Speed $speed.0" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 5
done

# Check journey summary
docker logs step4-app --tail 40
```

## 🔍 Advanced Architecture Patterns

### **Key Patterns You Learned:**

**1. Fleet Data Structure:**
```cpp
struct VehicleData {
    std::string vehicleId;
    double speed, rpm, fuel;
    double latitude, longitude;
    double odometer;
    double totalDistance;
    double totalFuelConsumed;
    int speedViolations;
    double driverScore;
};
std::map<std::string, VehicleData> m_fleet;
```

**2. GPS Zone Definition:**
```cpp
struct SpeedZone {
    std::string name;
    double minLat, maxLat;
    double minLon, maxLon;
    double speedLimit;
    std::string type;  // SCHOOL, CITY, HIGHWAY
};
```

**3. Geofencing Logic:**
```cpp
bool isInGeofence(double lat, double lon, const SpeedZone& zone) {
    return lat >= zone.minLat && lat <= zone.maxLat &&
           lon >= zone.minLon && lon <= zone.maxLon;
}
```

**4. Maintenance Scheduling:**
```cpp
struct MaintenanceItem {
    std::string vehicleId;
    std::string serviceType;
    double currentValue;
    double nextServiceAt;
    bool isOverdue;
    std::string priority;
};
```

**5. Fleet Analytics:**
```cpp
struct FleetStats {
    int totalVehicles;
    int activeVehicles;
    double totalDistance;
    double avgSpeed;
    double avgFuelEfficiency;
    double fleetScore;
};
```

## ✅ Success Criteria Validation

### **✅ Fleet Management:**
- **PASS**: Multi-vehicle architecture implemented
- **PASS**: Real-time vehicle tracking operational
- **PASS**: Fleet-wide statistics aggregation
- **PASS**: Driver performance scoring system

### **✅ GPS Features:**
- **PASS**: 5 speed zones configured
- **PASS**: Geofencing detection working
- **PASS**: Zone-based speed limit enforcement
- **PASS**: Location tracking with compliance

### **✅ Enterprise Features:**
- **PASS**: Predictive maintenance scheduling
- **PASS**: Comprehensive reporting system
- **PASS**: Compliance rate calculations
- **PASS**: Driver ranking algorithms
- **PASS**: Alert management with severity levels

## 📊 Production Benchmarks

| Feature | Production Target | Tutorial Result |
|---------|------------------|-----------------|
| **Vehicles Tracked** | 1000+ | 1 (demo) |
| **Update Frequency** | <1 second | Real-time |
| **Report Generation** | <5 seconds | <1 second |
| **Zone Detection** | <100ms | Instant |
| **Alert Response** | <500ms | Immediate |
| **Data Retention** | 90 days | Session only |

## 🐛 Advanced Troubleshooting

### **Issue 1: Zone Not Detected**
```bash
# Verify GPS coordinates are within zone bounds
# List configured zones in logs
docker logs step4-app | grep "GPS speed zones"

# Test specific coordinates
echo "getValue Vehicle.CurrentLocation.Latitude" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Issue 2: Maintenance Not Triggered**
```bash
# Check current odometer value
echo "getValue Vehicle.OBD.DistanceWithMIL" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Maintenance triggers at 10,000 km intervals
```

### **Issue 3: Reports Not Generated**
```bash
# Reports generate every 60 seconds
# Force immediate report by waiting or restarting
docker restart step4-app
```

## 🧹 Cleanup

```bash
# Stop fleet manager
docker stop step4-app && docker rm step4-app

# Stop databroker if finished
docker compose -f docker-compose.dev.yml down

# Remove Step 4 volumes if completely done
docker volume rm step4-build step4-deps step4-vss
```

## 🎓 Final Knowledge Check

**You've mastered these production concepts:**

1. **Fleet Architecture**: Scalable multi-vehicle data structures
2. **GPS Integration**: Geofencing and location-based services
3. **Compliance Systems**: Regulatory reporting and monitoring
4. **Maintenance Scheduling**: Predictive service management
5. **Enterprise Reporting**: Dashboard-ready analytics
6. **Performance Optimization**: Efficient fleet-wide operations

## 🏆 Tutorial Series Completed!

**Congratulations! You've built:**

- ✅ **Step 1**: Basic speed monitoring application
- ✅ **Step 2**: Multi-signal processing system
- ✅ **Step 3**: Advanced analytics with alerts
- ✅ **Step 4**: Production fleet management platform

## 🚀 Next Steps in Your SDV Journey

### **Extend Your Fleet Manager:**
1. **Database Integration**: Add PostgreSQL for historical data
2. **REST API**: Create endpoints for fleet dashboard
3. **WebSocket Server**: Real-time updates for web clients
4. **Machine Learning**: Predictive analytics for maintenance
5. **Cloud Integration**: AWS/Azure fleet data pipeline

### **Production Deployment:**
1. **Kubernetes**: Deploy fleet manager to K8s cluster
2. **Monitoring**: Add Prometheus/Grafana metrics
3. **Security**: Implement authentication and encryption
4. **Scaling**: Multi-region fleet support
5. **CI/CD**: Automated testing and deployment

### **Advanced Features:**
1. **Route Optimization**: Integrate with mapping services
2. **Fuel Optimization**: ML-based efficiency recommendations
3. **Driver Training**: Gamification and scoring improvements
4. **Predictive Routing**: Traffic and weather integration
5. **Electric Vehicle**: EV-specific range and charging management

---

## 📈 Your Achievement Summary

| Skill | Level Achieved |
|-------|----------------|
| **Signal Processing** | ⭐⭐⭐⭐⭐ Expert |
| **Multi-Signal Correlation** | ⭐⭐⭐⭐⭐ Expert |
| **Pattern Detection** | ⭐⭐⭐⭐⭐ Expert |
| **Alert Systems** | ⭐⭐⭐⭐⭐ Expert |
| **GPS Integration** | ⭐⭐⭐⭐⭐ Expert |
| **Fleet Architecture** | ⭐⭐⭐⭐⭐ Expert |
| **Production Readiness** | ⭐⭐⭐⭐ Advanced |

**🎉 You're now ready to build production SDV applications!**

---

*🚗💨 Transform vehicles into intelligent, connected systems with your new SDV expertise!*