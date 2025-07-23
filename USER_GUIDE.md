# Vehicle App C++ Template - User Guide

This guide provides step-by-step commands to understand and test the vehicle application build system.

## Prerequisites

Ensure you have Docker installed and the repository cloned:

```bash
git clone https://github.com/tri2510/vehicle-app-cpp-template.git
cd vehicle-app-cpp-template
```

## Step 1: Build the Base Docker Image

First, build the Velocitas development environment:

```bash
docker build -f Dockerfile.quick -t velocitas-quick . \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --build-arg http_proxy=http://127.0.0.1:3128 \
  --build-arg https_proxy=http://127.0.0.1:3128 \
  --network host
```

## Step 2: Start the Persistent Engine

The persistent engine keeps a container running for instant builds:

```bash
# Start the persistent engine
./velocitas-engine start

# Check engine status
./velocitas-engine status
```

You should see output like:
```
✅ Status: Running
📦 Container: velocitas-engine
💾 Workspace: velocitas-workspace
```

## Step 3: Build Your First Vehicle App

Build the minimal vehicle app example:

```bash
# Build the minimal vehicle app
./velocitas-engine build minimal-vehicle-app.cpp
```

Expected output:
```
✅ C++ compilation completed successfully
🔍 Verifying build output...
Found executable at: /quickbuild/build-linux-x86_64/Release/bin/app
```

## Step 4: Extract and Test the Binary

```bash
# Extract the compiled binary
docker cp velocitas-engine:/quickbuild/build-linux-x86_64/Release/bin/app ./my-vehicle-app

# Check the binary
ls -la my-vehicle-app
file my-vehicle-app
```

## Step 5: Create a Test Vehicle App Container

Create a simple Dockerfile for your app:

```bash
cat > Dockerfile.my-app << 'EOF'
FROM velocitas-quick:latest

# Copy executable
COPY my-vehicle-app /app/vehicle-app

WORKDIR /app
ENV SDV_VEHICLEDATABROKER_ADDRESS=""
ENTRYPOINT ["/app/vehicle-app"]
EOF
```

Build the container:

```bash
chmod +x my-vehicle-app

docker build -f Dockerfile.my-app -t my-vehicle-app . \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --network host
```

## Step 6: Test the Vehicle App

### Quick Test (Standalone)
```bash
# Test the app for 10 seconds
timeout 10 docker run --rm my-vehicle-app || echo "App ran successfully"
```

Expected output:
```
INFO  : Starting app ...
INFO  : === Minimal Vehicle App Started ===
🚗 Minimal Vehicle App Started
🔌 Connecting to KUKSA.val Databroker...
✅ Subscription to Vehicle.Speed created
📊 Waiting for vehicle signals...
```

### Full Integration Test (with KUKSA.val)

Run the complete test with KUKSA.val databroker:

```bash
# Run complete integration test
./test-kuksa-complete.sh --verbose
```

This will:
1. Start KUKSA.val databroker
2. Build and run your vehicle app
3. Test positive/negative cases
4. Inject test signals
5. Show comprehensive results

## Step 7: Understanding the Engine Commands

```bash
# View all available commands
./velocitas-engine help

# Build different apps
./velocitas-engine build my-custom-app.cpp
./velocitas-engine build another-app.cpp

# Run an app directly (build + run)
./velocitas-engine run minimal-vehicle-app.cpp

# Check engine logs
./velocitas-engine logs

# Open shell in engine for debugging
./velocitas-engine shell

# Restart engine if needed
./velocitas-engine restart

# Clean all data and start fresh
./velocitas-engine clean
```

## Step 8: Create Your Own Vehicle App

Create a custom vehicle app:

```bash
cat > my-speed-monitor.cpp << 'EOF'
#include "sdk/VehicleApp.h"
#include "sdk/Logger.h"
#include "vehicle/Vehicle.hpp"
#include <iostream>
#include <memory>

using namespace velocitas;

class SpeedMonitorApp : public VehicleApp {
private:
    vehicle::Vehicle Vehicle;
    
public:
    SpeedMonitorApp(std::shared_ptr<IVehicleDataBrokerClient> vdbClient, 
                    std::shared_ptr<IPubSubClient> pubSubClient)
        : VehicleApp(vdbClient, pubSubClient) {
    }

    void onStart() override {
        std::cout << "🚗 Speed Monitor Started!" << std::endl;
        
        auto subscription = subscribeDataPoints("SELECT Vehicle.Speed");
        subscription->onItem([this](const DataPointReply& reply) {
            auto speedDataPoint = reply.get(Vehicle.Speed);
            if (speedDataPoint && speedDataPoint->isValid()) {
                float speed = speedDataPoint->value();
                std::cout << "Current Speed: " << speed << " km/h" << std::endl;
                
                if (speed > 120.0f) {
                    std::cout << "⚠️  SPEEDING! Slow down!" << std::endl;
                }
            }
        });
    }
};

int main(int argc, char** argv) {
    auto app = std::make_unique<SpeedMonitorApp>(nullptr, nullptr);
    app->run();
    return 0;
}
EOF
```

Build and test your custom app:

```bash
./velocitas-engine build my-speed-monitor.cpp
./velocitas-engine run my-speed-monitor.cpp
```

## Step 9: Performance Monitoring

Check engine performance and caching:

```bash
# Check cache status
./velocitas-engine status

# Time a build (should be very fast after first build)
time ./velocitas-engine build minimal-vehicle-app.cpp

# Monitor container resources
docker stats velocitas-engine
```

## Step 10: Cleanup

When done testing:

```bash
# Stop test containers
docker stop kuksa-databroker vehicle-app 2>/dev/null || true
docker rm kuksa-databroker vehicle-app 2>/dev/null || true

# Stop persistent engine
./velocitas-engine stop

# Remove test images (optional)
docker rmi my-vehicle-app vehicle-app-test 2>/dev/null || true

# Clean all engine data (optional - removes all cached builds)
./velocitas-engine clean
```

## Troubleshooting

### Build Issues
```bash
# Check engine logs if build fails
./velocitas-engine logs

# Restart engine if stuck
./velocitas-engine restart

# Clean and rebuild if corrupted
./velocitas-engine clean
./velocitas-engine start
```

### Container Issues
```bash
# Check if base image exists
docker images | grep velocitas-quick

# Rebuild base image if needed
docker build -f Dockerfile.quick -t velocitas-quick . --network host
```

### Network Issues
```bash
# Test KUKSA connectivity
docker run --rm --network vehicle-test-network ghcr.io/eclipse/kuksa.val/databroker:master --help
```

## Key Benefits

- **⚡ Instant Builds**: Persistent engine eliminates container startup time
- **💾 Smart Caching**: Dependencies, models, and builds are cached
- **🔄 Cache Invalidation**: Source changes trigger clean rebuilds automatically  
- **🚗 Real Testing**: Integration with actual KUKSA.val databroker
- **📊 Comprehensive**: Tests positive/negative cases with signal injection

Your vehicle apps will now build in seconds instead of minutes thanks to the persistent caching system!