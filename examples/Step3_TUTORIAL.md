# 🎓 Step 3: Custom VSS Tutorial

## 📚 Learning Objectives

In this step, you'll learn comprehensive custom VSS signal processing:

- **Advanced Custom VSS Creation**: Design and implement professional-grade custom signals
- **KUKSA Custom Loading**: Configure KUKSA databroker with sophisticated VSS specification
- **Multi-Branch Signal Subscription**: Subscribe to complex signal hierarchies
- **Real-World Signal Processing**: Process fleet management, driver behavior, and diagnostic signals
- **End-to-End Integration**: Test complete custom VSS workflow with realistic scenarios

**Difficulty**: ⭐⭐⭐⭐ Expert | **Time**: 60 minutes

## 🎯 What You'll Build

A comprehensive custom VSS application that demonstrates:
- **Driver Behavior Analysis**: Score, AggressiveEvents, SmoothDrivingTime
- **Fleet Management**: VehicleId, Status, Efficiency metrics
- **Vehicle Diagnostics**: SystemHealth, MaintenanceRequired, AlertLevel
- **Professional VSS Structure**: Organized signal branches with proper metadata
- **Production-Ready Patterns**: Type-safe signal handling and validation

## 📋 Prerequisites

**Required Setup:**
```bash
# Stop any existing KUKSA instance
docker stop velocitas-vdb kuksa-tutorial-vss 2>/dev/null || true

# Create persistent volumes for Step 3
docker volume rm step3-build step3-deps step3-vss 2>/dev/null || true
docker volume create step3-build
docker volume create step3-deps  
docker volume create step3-vss
```

## 🚀 Step-by-Step Tutorial

### **Phase 1: Start KUKSA with Tutorial Custom VSS**

```bash
# Start KUKSA databroker with comprehensive tutorial VSS specification
docker run -d --rm --name kuksa-tutorial-vss --network host \
  -v $(pwd)/examples/Step3_CustomVSS.json:/vss.json \
  ghcr.io/eclipse-kuksa/kuksa-databroker:main \
  --address 0.0.0.0 --port 55555 --insecure --vss /vss.json
```

**Expected Output:**
```bash
# Check KUKSA logs
docker logs kuksa-tutorial-vss --tail 10
# Should show:
# INFO: Loading VSS metadata from /vss.json
# INFO: Found 240+ signals in VSS specification
# INFO: Loaded Vehicle.Tutorial.DriverBehavior branch
# INFO: Loaded Vehicle.Tutorial.Fleet branch  
# INFO: Loaded Vehicle.Tutorial.Diagnostics branch
# INFO: Listening on 0.0.0.0:55555
```

### **Phase 2: Build Tutorial Custom VSS Application**

```bash
# Build comprehensive tutorial VSS app with custom VSS specification
docker run --rm --network host \
  -v step3-build:/quickbuild/build \
  -v step3-deps:/home/vscode/.conan2 \
  -v step3-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_CustomVSS.cpp:/app.cpp \
  -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose --force
```

**Expected Build Output:**
```
🚗 Configuring Vehicle Signal Specification...
ℹ️  [INFO] Using custom VSS file: /custom_vss.json
✅ [SUCCESS] VSS configuration updated
ℹ️  [INFO] VSS source: file:///quickbuild/custom-vss.json
🏗️  Generating vehicle model from VSS...
✅ [SUCCESS] Vehicle model generated successfully
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: ~16M (comprehensive tutorial VSS)
🎉 Tutorial Custom VSS build completed!
```

### **Phase 3: Run Tutorial Custom VSS Application**

```bash
# Run comprehensive tutorial VSS application
docker run -d --network host --name step3-tutorial-vss \
  -v step3-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 300
```

**Expected Runtime Output:**
```bash
# Check application logs
docker logs step3-tutorial-vss --tail 15
```

**Should show:**
```
🎓 Step 3: Tutorial Custom VSS starting...
📡 Connecting to Vehicle Data Broker at 127.0.0.1:55555...
🚀 Step 3: Starting Tutorial Custom VSS Application!
📊 Setting up tutorial VSS signal subscriptions...
   └── Vehicle.Tutorial.DriverBehavior.* (3 signals)
   └── Vehicle.Tutorial.Fleet.* (4 signals)
   └── Vehicle.Tutorial.Diagnostics.* (3 signals)
✅ Tutorial VSS signal subscriptions completed (10 signals total)
🔄 Waiting for tutorial VSS signals...
💡 Test with tutorial signals:
   echo 'setValue Vehicle.Tutorial.DriverBehavior.Score 85' | kuksa-client
   echo 'setValue Vehicle.Tutorial.Fleet.Status "DRIVING"' | kuksa-client
   echo 'setValue Vehicle.Tutorial.Diagnostics.SystemHealth 95' | kuksa-client
```

## 📊 Testing Tutorial Custom VSS Signals

### **Test 1: Driver Behavior Signals**

```bash
# Test driver behavior score
echo "setValue Vehicle.Tutorial.DriverBehavior.Score 85" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test aggressive events count
echo "setValue Vehicle.Tutorial.DriverBehavior.AggressiveEvents 3" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test smooth driving time percentage
echo "setValue Vehicle.Tutorial.DriverBehavior.SmoothDrivingTime 78.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-tutorial-vss --tail 10
```

**Expected Output:**
```
📡 Received tutorial VSS signal update: Vehicle.Tutorial.DriverBehavior.Score
🏆 Driver Behavior Score: 85/100 - Good driving performance!
📡 Received tutorial VSS signal update: Vehicle.Tutorial.DriverBehavior.AggressiveEvents
⚠️  Aggressive Events Count: 3 - Room for improvement
📡 Received tutorial VSS signal update: Vehicle.Tutorial.DriverBehavior.SmoothDrivingTime
🚗 Smooth Driving Time: 78.5% - Well done!
🎯 Processing driver behavior analytics...
```

### **Test 2: Fleet Management Signals**

```bash
# Test vehicle ID (attribute)
echo "setValue Vehicle.Tutorial.Fleet.VehicleId \"FLEET-TRUCK-001\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test vehicle status
echo "setValue Vehicle.Tutorial.Fleet.Status \"DRIVING\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test fuel consumption
echo "setValue Vehicle.Tutorial.Fleet.Efficiency.FuelConsumption 8.5" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test eco score
echo "setValue Vehicle.Tutorial.Fleet.Efficiency.EcoScore 92" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-tutorial-vss --tail 12
```

**Expected Output:**
```
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Fleet.VehicleId
🚛 Fleet Vehicle ID: FLEET-TRUCK-001
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Fleet.Status
🚗 Fleet Status: DRIVING - Vehicle is operational
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Fleet.Efficiency.FuelConsumption
⛽ Fuel Consumption: 8.5 L/100km - Efficient driving
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Fleet.Efficiency.EcoScore
🌱 Eco Score: 92/100 - Excellent efficiency!
🎯 Processing fleet management data...
```

### **Test 3: Diagnostic Signals**

```bash
# Test system health percentage
echo "setValue Vehicle.Tutorial.Diagnostics.SystemHealth 95" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test maintenance required flag
echo "setValue Vehicle.Tutorial.Diagnostics.MaintenanceRequired false" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test alert level
echo "setValue Vehicle.Tutorial.Diagnostics.AlertLevel \"NORMAL\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check processing
docker logs step3-tutorial-vss --tail 10
```

**Expected Output:**
```
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Diagnostics.SystemHealth
💚 System Health: 95% - Excellent system condition
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Diagnostics.MaintenanceRequired
✅ Maintenance Required: false - No maintenance needed
📡 Received tutorial VSS signal update: Vehicle.Tutorial.Diagnostics.AlertLevel
🟢 Alert Level: NORMAL - All systems operating normally
🎯 Processing diagnostic information...
```

### **Test 4: Alert and Warning Scenarios**

```bash
# Test poor driver behavior (should trigger warnings)
echo "setValue Vehicle.Tutorial.DriverBehavior.Score 35" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.DriverBehavior.AggressiveEvents 15" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test fleet issues (should trigger warnings)
echo "setValue Vehicle.Tutorial.Fleet.Status \"MAINTENANCE\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Fleet.Efficiency.FuelConsumption 15.2" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Test diagnostic alerts (should trigger warnings)
echo "setValue Vehicle.Tutorial.Diagnostics.SystemHealth 45" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Diagnostics.MaintenanceRequired true" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Diagnostics.AlertLevel \"CRITICAL\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check all warnings
docker logs step3-tutorial-vss --tail 20 | grep -E "(WARN|🔥|🚨|⚠️|❌|🔴)"
```

**Expected Alert Output:**
```
⚠️  LOW Driver Score: 35/100 - Training needed!
🔥 HIGH Aggressive Events: 15 - Dangerous driving detected!
🚨 Fleet Status: MAINTENANCE - Vehicle out of service
❌ HIGH Fuel Consumption: 15.2 L/100km - Inefficient driving
🔴 LOW System Health: 45% - Critical maintenance required
⚠️  Maintenance Required: true - Schedule service immediately
🚨 CRITICAL Alert Level: System requires immediate attention!
```

### **Test 5: Complete Tutorial VSS State**

```bash
# Set all tutorial signals to show comprehensive state
echo "setValue Vehicle.Tutorial.DriverBehavior.Score 88" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.DriverBehavior.AggressiveEvents 2" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.DriverBehavior.SmoothDrivingTime 85.3" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Fleet.VehicleId \"FLEET-BUS-042\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Fleet.Status \"DRIVING\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Fleet.Efficiency.FuelConsumption 7.8" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Fleet.Efficiency.EcoScore 91" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Diagnostics.SystemHealth 98" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Diagnostics.MaintenanceRequired false" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Tutorial.Diagnostics.AlertLevel \"NORMAL\"" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check complete state
docker logs step3-tutorial-vss --tail 15 | grep -A 15 "TUTORIAL VSS STATE"
```

**Expected Complete State:**
```
📊 === TUTORIAL VSS STATE ===
🏆 Driver Behavior:
   └── Score: 88/100 (Excellent)
   └── Aggressive Events: 2 (Good)
   └── Smooth Driving: 85.3% (Very Good)
🚛 Fleet Management:
   └── Vehicle ID: FLEET-BUS-042
   └── Status: DRIVING (Active)
   └── Fuel Consumption: 7.8 L/100km (Efficient)
   └── Eco Score: 91/100 (Excellent)
💚 Diagnostics:
   └── System Health: 98% (Excellent)
   └── Maintenance Required: false
   └── Alert Level: NORMAL
🎉 Tutorial VSS signals working perfectly!
======================================
```

## 🧪 Advanced Testing Scenarios

### **Scenario 1: Driver Behavior Analysis Sequence**

```bash
# Simulate a complete driving session with behavior analysis
scores=(95 88 75 60 45 30 15 5)
for score in "${scores[@]}"; do
  echo "setValue Vehicle.Tutorial.DriverBehavior.Score $score" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 1
done

# Check score progression and threshold alerts
docker logs step3-tutorial-vss | grep -E "(Driver.*Score|LOW|HIGH)" | tail -10
```

### **Scenario 2: Fleet Status Transitions**

```bash
# Test fleet status transitions
statuses=("IDLE" "DRIVING" "MAINTENANCE" "EMERGENCY" "DRIVING" "IDLE")
for status in "${statuses[@]}"; do
  echo "setValue Vehicle.Tutorial.Fleet.Status \"$status\"" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 2
done

# Check status transitions
docker logs step3-tutorial-vss | grep -E "(Fleet Status|Status:|MAINTENANCE|EMERGENCY)" | tail -8
```

### **Scenario 3: System Health Monitoring**

```bash
# Test system health degradation and recovery
health_levels=(100 95 80 65 50 35 20 10 25 40 55 70 85 100)
for health in "${health_levels[@]}"; do
  echo "setValue Vehicle.Tutorial.Diagnostics.SystemHealth $health" | docker run --rm -i --network host \
    ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
  sleep 1
done

# Check health monitoring and alerts
docker logs step3-tutorial-vss | grep -E "(System Health|CRITICAL|LOW|Excellent)" | tail -15
```

## 🎓 Learning Outcomes

After completing this tutorial, you will have learned:

### ✅ **Professional Custom VSS Design**
- How to design comprehensive VSS specifications with proper structure
- Complex signal hierarchies: branches with multiple child signals
- Professional datatypes: uint8, float, string, boolean with validation
- Signal metadata: descriptions, units, min/max values, allowed values

### ✅ **Advanced KUKSA Integration**
- Loading sophisticated custom VSS files into KUKSA databroker
- Managing complex signal subscriptions across multiple branches
- Verifying comprehensive VSS loading with hundreds of signals

### ✅ **Multi-Branch Signal Processing**
- Subscribing to organized signal hierarchies: `Vehicle.Tutorial.*`
- Processing different signal categories: DriverBehavior, Fleet, Diagnostics
- Implementing business logic for different signal types and ranges
- Type-safe signal handling for various datatypes

### ✅ **Real-World VSS Applications**
- Driver behavior analysis with scoring and event tracking
- Fleet management with vehicle identification and efficiency metrics
- Diagnostic monitoring with health assessment and maintenance alerts
- Production-ready signal validation and alert systems

### ✅ **End-to-End Custom VSS Workflow**
- Complete signal injection workflow: `setValue Vehicle.Tutorial.*` commands
- Comprehensive signal processing verification through structured logs
- Professional signal handling patterns for complex applications

## 🔧 Tutorial Custom VSS Specification Format

The `Step3_CustomVSS.json` file defines a comprehensive signal hierarchy:

```json
{
  "Vehicle": {
    "type": "branch",
    "Tutorial": {
      "type": "branch",
      "description": "Custom signals for Step 3 tutorial demonstrations",
      "children": {
        "DriverBehavior": {
          "type": "branch",
          "description": "Driver behavior analysis signals",
          "children": {
            "Score": {
              "datatype": "uint8",
              "description": "Driver behavior score from 0-100",
              "max": 100, "min": 0,
              "type": "sensor", "unit": "percent"
            },
            "AggressiveEvents": {
              "datatype": "uint32",
              "description": "Count of aggressive driving events",
              "type": "sensor", "unit": "count"
            },
            "SmoothDrivingTime": {
              "datatype": "float",
              "description": "Percentage of trip time spent driving smoothly",
              "max": 100, "min": 0,
              "type": "sensor", "unit": "percent"
            }
          }
        },
        "Fleet": {
          "type": "branch",
          "description": "Fleet management and tracking signals",
          "children": {
            "VehicleId": {
              "datatype": "string",
              "description": "Unique identifier for this vehicle in the fleet",
              "type": "attribute"
            },
            "Status": {
              "datatype": "string",
              "description": "Current operational status of the vehicle",
              "allowed": ["IDLE", "DRIVING", "MAINTENANCE", "EMERGENCY"],
              "type": "sensor"
            },
            "Efficiency": {
              "type": "branch",
              "children": {
                "FuelConsumption": {
                  "datatype": "float",
                  "description": "Average fuel consumption in L/100km",
                  "type": "sensor", "unit": "l/100km"
                },
                "EcoScore": {
                  "datatype": "uint8",
                  "description": "Eco-driving score from 0-100",
                  "max": 100, "min": 0,
                  "type": "sensor", "unit": "percent"
                }
              }
            }
          }
        },
        "Diagnostics": {
          "type": "branch",
          "description": "Custom diagnostic and monitoring signals",
          "children": {
            "SystemHealth": {
              "datatype": "uint8",
              "description": "Overall system health percentage",
              "max": 100, "min": 0,
              "type": "sensor", "unit": "percent"
            },
            "MaintenanceRequired": {
              "datatype": "boolean",
              "description": "Indicates if maintenance is required",
              "type": "sensor"
            },
            "AlertLevel": {
              "datatype": "string",
              "description": "Current system alert level",
              "allowed": ["NORMAL", "WARNING", "CRITICAL"],
              "type": "sensor"
            }
          }
        }
      }
    }
  }
}
```

**Key Features:**
- **Hierarchical Organization**: Signals organized in logical branches
- **Rich Metadata**: Descriptions, units, min/max values, allowed values  
- **Professional Types**: uint8, float, string, boolean with validation
- **Real-World Scenarios**: Driver behavior, fleet management, diagnostics

## 🔄 Next Steps

### **Expand Tutorial VSS:**
1. **Add Advanced Signal Types**: arrays, nested structures, timestamps
2. **Create Additional Branches**: Navigation, Entertainment, Security
3. **Implement Signal Relationships**: Cross-signal validation and dependencies
4. **Add Historical Data**: Trend analysis and time-series signals

### **Production Integration:**
1. **Vehicle Model Generation**: Generate C++ bindings for tutorial signals
2. **Type-Safe Subscriptions**: Use generated VehicleTutorial model classes
3. **Advanced Signal Processing**: Machine learning integration for behavior analysis
4. **Performance Optimization**: Efficient multi-branch signal processing
5. **Database Integration**: Store tutorial signal data for analytics

## 🎉 Congratulations!

You have successfully built and tested a **comprehensive tutorial VSS application**! You now know how to:
- ✅ Design professional-grade custom VSS specifications with complex hierarchies
- ✅ Load sophisticated VSS into KUKSA databroker with multiple signal branches
- ✅ Subscribe to organized signal categories: DriverBehavior, Fleet, Diagnostics
- ✅ Process real-world signals with proper validation and business logic
- ✅ Implement production-ready signal handling patterns
- ✅ Test comprehensive VSS scenarios with realistic data

**Your tutorial VSS application demonstrates enterprise-level custom signal processing and is ready for advanced SDV development!** 🚀

## 🔧 Troubleshooting Custom VSS

### **Issue: Vehicle Model Generation Errors**

**Problem:** Build fails with `'class vehicle::Vehicle' has no member named 'Tutorial'`

**Root Cause:** The Vehicle model was not generated from the custom VSS file.

**Solution:** Ensure you include the `VSS_SPEC_FILE` environment variable:
```bash
# ❌ WRONG - Missing VSS_SPEC_FILE
docker run --rm --network host \
  -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
  velocitas-quick build

# ✅ CORRECT - With VSS_SPEC_FILE  
docker run --rm --network host \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
  velocitas-quick build --force
```

### **Issue: VSS Generation Unit Errors**

**Problem:** Build fails with `Unknown unit count for signal Vehicle.Tutorial.*`

**Root Cause:** Custom VSS uses unsupported units.

**Solution:** Use standard VSS units or omit the unit field:
```json
// ❌ WRONG - Unsupported unit
"AggressiveEvents": {
  "datatype": "uint32",
  "unit": "count"  // Not supported
}

// ✅ CORRECT - No unit for counters  
"AggressiveEvents": {
  "datatype": "uint32"
  // No unit field for simple counters
}
```

### **Issue: Custom VSS Not Loading**

**Problem:** Build shows "Using default VSS 4.0 specification" instead of custom VSS, or compilation fails with `'class vehicle::Vehicle' has no member named 'Tutorial'`.

**Root Cause:** Custom VSS file wasn't properly configured or mounted.

**Solution:** Ensure VSS_SPEC_FILE environment variable is set and file is mounted:
```bash
# Check if VSS_SPEC_FILE is set
docker run --rm --entrypoint="" velocitas-quick env | grep VSS

# Check if custom VSS file is mounted correctly
docker run --rm --entrypoint="" \
  -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
  velocitas-quick ls -la /custom_vss.json

# Force regeneration of vehicle model
docker run --rm \
  -e VSS_SPEC_FILE=/custom_vss.json \
  -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
  velocitas-quick build --force --verbose
```

### **Issue: Signal Subscription Failures**

**Problem:** App runs but subscription errors occur.

**Solution:** Verify the custom VSS matches the KUKSA databroker VSS:
```bash
# Check what signals are available in KUKSA
echo "getValue Vehicle.Tutorial.DriverBehavior.Score" | \
  docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

## 🧹 Cleanup

```bash
# Stop containers
docker stop step3-tutorial-vss kuksa-tutorial-vss

# Remove containers  
docker rm step3-tutorial-vss kuksa-tutorial-vss

# Clean volumes (optional)
docker volume rm step3-build step3-deps step3-vss

# Verify cleanup
docker ps -a | grep -E "(step3|kuksa)" || echo "✅ All tutorial containers cleaned up"
```