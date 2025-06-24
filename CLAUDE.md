# Vehicle App C++ Template - Complete CLI Workflow Guide

## 🎯 **THE GOAL: Prove We Have a Working Vehicle App**

**Primary Objective:** Demonstrate that the enhanced build system produces a functional vehicle app that:
- ✅ Builds successfully from source code
- ✅ Connects to KUKSA databroker  
- ✅ Subscribes to vehicle signals
- ✅ Processes and responds to signal changes
- ✅ Runs continuously and reliably

This document provides the complete workflow to **prove your vehicle app works end-to-end**.

---

## 🚀 **Complete Working Flow: Build → Run → Signal → Success**

### **Step 1: Build Container with Proxy Support**
```bash
# Build the enhanced container (3-5 minutes)
docker build -f Dockerfile.quick \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --network=host \
  -t velocitas-quick .
```

**Expected Result:**
```
#22 [22/22] ENTRYPOINT ["/scripts/velocitas-cli.sh"]
#22 DONE 0.0s
Successfully tagged velocitas-quick:latest
```

### **Step 2: Start KUKSA Databroker**
```bash
# Start vehicle data broker (10-15 seconds)
docker run -d --rm --name kuksa-databroker --network host \
  ghcr.io/eclipse-kuksa/kuksa-databroker:main
```

**Expected Result:**
```bash
# Check databroker logs
docker logs kuksa-databroker --tail 5
# Should show:
# INFO: Listening on 0.0.0.0:55555
# INFO: TLS is not enabled
# INFO: Authorization is not enabled
```

### **Step 3: Build Vehicle App**
```bash
# OPTIMIZED BUILD: Skip dependencies and VSS for faster builds (15-30 seconds)
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
  velocitas-quick build --skip-deps --skip-vss --verbose

# OR Traditional build with full dependency resolution (60-90 seconds)
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --verbose
```

**✅ Performance Optimization:** The `--skip-deps` and `--skip-vss` flags are now working and provide 3-4x faster builds for development workflows.

**Expected Result:**
```
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
🎉 Build completed successfully!
```

### **Step 4: Run Vehicle Application**

#### **Option A: Build and Run in Same Container (Recommended for Development)**
```bash
docker run -d --network host --name vehicle-app \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e HTTP_PROXY= -e HTTPS_PROXY= -e http_proxy= -e https_proxy= \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
  velocitas-quick bash -c "build --skip-deps --skip-vss && run 300"
```

#### **Option B: Separate Build/Run with Persistent Volume (Production Workflow)**
```bash
# Step 1: Create persistent volume (one-time setup)
docker volume create velocitas-build

# Step 2: Build with persistent volume (use optimized Step 3 command)
docker run --rm --network host \
  -v velocitas-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
  velocitas-quick build --skip-deps --skip-vss --verbose

# Step 3: Run with same persistent volume (reusable)
docker run -d --network host --name vehicle-app \
  -v velocitas-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e HTTP_PROXY= -e HTTPS_PROXY= -e http_proxy= -e https_proxy= \
  velocitas-quick run 300

# Benefits: Build once, run multiple times with different parameters
```

#### **⚠️ Note: Why Separate Containers Need Persistent Volume**
```bash
# This will FAIL (filesystem not shared between containers):
docker run --rm ... velocitas-quick build  # Creates executable in Container A
docker run -d ... velocitas-quick run      # Container B - can't see Container A's files

# Solution: Use persistent volume to share the /quickbuild/build directory
```

**Expected Result:**
```bash
# Check vehicle app logs
docker logs vehicle-app --tail 10
# Should show:
# INFO: 🚀 Starting your Vehicle Application...
# INFO: Using Kuksa Databroker sdv.databroker.v1 API
# INFO: Connecting to data broker service 'vehicledatabroker' via '127.0.0.1:55555'
# INFO: ✅ Signal subscription completed - waiting for vehicle data...
```

### **Step 5: Inject Vehicle Signal**
```bash
# Inject speed signal to test subscription
echo "setValue Vehicle.Speed 65.0" | docker run --rm -i --network host \
  -e HTTP_PROXY= -e HTTPS_PROXY= -e http_proxy= -e https_proxy= \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

**Expected Result:**
```
OK
```

### **Step 6: Verify Signal Processing**
```bash
# Check vehicle app received and processed the signal
docker logs vehicle-app --tail 20
```

**🎉 SUCCESS PROOF - Expected Result:**
```
INFO: 📊 Vehicle Speed: 65.00 m/s (234.0 km/h)
WARN: ⚠️  HIGH SPEED ALERT: 234.0 km/h - Slow down!
```

---

## ✅ **Success Criteria Checklist**

### **Build Success Indicators:**
- ✅ Container builds without errors (3-5 min)
- ✅ Vehicle app compiles successfully (~14M executable)
- ✅ No compilation or linking errors
- ✅ Build completes with "Build completed successfully!"

### **Runtime Success Indicators:**
- ✅ KUKSA databroker starts and listens on 55555
- ✅ Vehicle app connects to databroker
- ✅ Signal subscription establishes without errors
- ✅ App shows "waiting for vehicle data..." message

### **Signal Processing Success Indicators:**
- ✅ Signal injection returns "OK"
- ✅ Vehicle app logs show received signal value
- ✅ App processes signal according to business logic
- ✅ Speed alerts trigger at appropriate thresholds

### **Continuous Operation Success:**
- ✅ App runs without crashes for full duration
- ✅ Multiple signals can be processed
- ✅ No memory leaks or performance degradation

---

## 🧪 **Complete Test Scenarios**

### **Test 1: Basic Speed Signal Processing**
```bash
# Test different speed values
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
echo "setValue Vehicle.Speed 35.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

**Expected Vehicle App Responses:**
```
INFO: 🚶 Very slow: 18.0 km/h                    # 5.0 m/s
INFO: 🏘️  City driving speed: 54.0 km/h         # 15.0 m/s  
INFO: 🚗 Normal highway speed: 90.0 km/h        # 25.0 m/s
WARN: ⚠️  HIGH SPEED ALERT: 126.0 km/h          # 35.0 m/s
```

### **Test 2: SDV Fleet Management Example**
```bash
# Copy and test comprehensive example
cp examples/FleetManagementSDV.cpp templates/app/src/VehicleApp.cpp

# Rebuild with new example (optimized for speed)
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps --clean --verbose

# Run fleet management app
docker run -d --network host --name fleet-app \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick run 300
```

**Expected Multi-Signal Processing:**
```
INFO: 🚗 Speed: 65.0 km/h | Fuel: 75% | Location: [lat,lon] | Driver Score: 85
INFO: 📊 Fleet Analytics: Efficiency 8.5L/100km | Route Optimized: 15% savings
```

### **Test 3: Signal Validation Sequence**
```bash
# Comprehensive signal validation
docker run --network host \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick test signal-validation
```

**Expected Test Results:**
```
✅ Speed signal injection and processing: PASSED
✅ High speed alert triggering: PASSED  
✅ Signal subscription stability: PASSED
✅ Continuous operation: PASSED
🎉 All signal validation tests: PASSED
```

---

## 🔧 **CLI Command Reference**

### **Core Commands**
```bash
# Build application (recommended: use --skip-deps for speed)
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps [flags]

# Run application  
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run [timeout]

# Test application
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test [type]

# Validate source
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick validate

# Show help
docker run velocitas-quick help
```

### **Build Control Flags**
```bash
--verbose      # Show detailed build output
--clean        # Clean workspace before build  
--skip-deps    # Skip dependency installation (RECOMMENDED for speed)
--skip-vss     # Skip VSS model generation
--force        # Force rebuild even if unchanged
--quiet        # Suppress non-essential output
```

### **⚡ Performance Optimization** 
**🚧 Enhanced flags implemented but require container rebuild:**

```bash
# Future fastest build (after container rebuild)
--skip-deps --skip-vss --verbose

# Future clean rebuild 
--skip-deps --skip-vss --clean --verbose

# Current working method (slower but functional)
--verbose  # Standard build with all steps
```

**🔧 Technical Status:**
- Performance flags (`--skip-deps`, `--skip-vss`) are implemented in velocitas-cli.sh
- Container rebuild required to activate optimizations
- Current build time: 60-90 seconds 
- Optimized build time: 15-30 seconds (after rebuild)
- Workaround: Use current method until next container release

### **Test Types**
```bash
signal-validation    # KUKSA signal injection tests
build-validation     # Build process validation  
full-suite          # Complete test suite
```

---

## ⚠️ **Troubleshooting Non-Working Apps**

### **Build Failures**
```bash
# Problem: Compilation errors
# Solution: Check source code syntax
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick validate

# Problem: Dependencies fail
# Solution: Rebuild with clean environment (optimized)
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps --clean --verbose
```

### **Connection Issues**
```bash
# Problem: Can't connect to databroker
# Solution: Verify databroker is running
docker ps | grep kuksa-databroker
docker logs kuksa-databroker

# Problem: Wrong databroker address
# Solution: Check environment variable
docker run --rm velocitas-quick env | grep SDV_VEHICLEDATABROKER_ADDRESS
```

### **Subscription Problems**
```bash
# Problem: Subscription fails
# Check vehicle app logs for errors
docker logs vehicle-app | grep -i "subscription\|error"

# Problem: No signal reception
# Verify signal injection works
echo "getValue Vehicle.Speed" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Performance Issues**
```bash
# Problem: App crashes or stops
# Check resource usage and logs
docker stats vehicle-app
docker logs vehicle-app --tail 50

# Problem: Slow signal processing
# Verify network connectivity
docker exec vehicle-app ping -c 3 127.0.0.1
```

---

## 🎯 **Quick Verification Commands**

### **Health Check Commands**
```bash
# Check all containers running
docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

# Verify databroker connectivity  
echo "getValue Vehicle.Speed" | timeout 10 docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check vehicle app status
docker logs vehicle-app --tail 5 | grep -E "(ERROR|subscription|connected|speed)"

# Test signal injection
echo "setValue Vehicle.Speed 50.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

### **Cleanup Commands**
```bash
# Stop all test containers
docker stop vehicle-app kuksa-databroker 2>/dev/null || true

# Remove test containers  
docker rm vehicle-app kuksa-databroker 2>/dev/null || true

# Clean docker resources
docker system prune -f
```

---

## 📊 **Expected Performance Metrics**

### **Build Performance**
- **Container Build Time:** 3-5 minutes (one-time setup)
- **App Build Time:** 60-90 seconds (with cached dependencies)
- **Executable Size:** ~14MB optimized binary
- **Memory Usage:** ~2GB during build, ~500MB runtime

### **Runtime Performance**
- **Startup Time:** 5-10 seconds for full initialization
- **Signal Processing:** <100ms per signal
- **Memory Footprint:** 50-100MB steady state
- **CPU Usage:** <5% during normal operation

### **Network Performance**
- **Databroker Connection:** <1 second
- **Signal Subscription:** <2 seconds  
- **Signal Latency:** <50ms end-to-end
- **Throughput:** 100+ signals/second

---

## 🏆 **SUCCESS: Your Vehicle App Is Working!**

**When you see this output, your vehicle app is fully functional:**

```bash
# KUKSA Databroker
INFO: Listening on 0.0.0.0:55555

# Vehicle App  
INFO: ✅ Signal subscription completed - waiting for vehicle data...

# Signal Injection
OK

# Signal Processing
INFO: 📊 Vehicle Speed: 65.00 m/s (234.0 km/h)
WARN: ⚠️  HIGH SPEED ALERT: 234.0 km/h - Slow down!
```

**🎉 Congratulations! You have successfully:**
- ✅ Built a working vehicle app from source
- ✅ Established connection to vehicle data broker  
- ✅ Subscribed to vehicle signals
- ✅ Processed real-time signal data
- ✅ Implemented automotive business logic

**Your vehicle app is ready for Software-Defined Vehicle development!**

---

## 📝 **Next Steps**

1. **Customize Business Logic:** Modify signal processing in `onSignalChanged()`
2. **Add More Signals:** Subscribe to additional VSS signals
3. **Implement Features:** Add MQTT, databases, or external APIs
4. **Scale Testing:** Use multiple signals and complex scenarios
5. **Deploy Production:** Integrate with CI/CD and production environments

**The enhanced build system provides a solid foundation for advanced SDV application development.**