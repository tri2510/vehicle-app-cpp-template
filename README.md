# Ultra-Minimal Vehicle App - Docker Development Environment

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

The **simplest possible** [Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) Vehicle App in C++. One file, one signal, zero complexity. Perfect for learning and Docker-based development.

## 🎯 What This Is

**Ultra-minimal Vehicle App** that:
- Reads `Vehicle.Speed` from Vehicle Data Broker
- Logs speed values in m/s and km/h  
- **Single file**: `app/app.cpp` contains everything
- **No MQTT**: Only Vehicle Data Broker communication
- **No headers**: Class definition and main() in one file
- **Zero complexity**: Perfect starting point

## 🚀 Quick Start

### Prerequisites
- [Docker](https://docker.com) installed and running

### 3-Step Development Workflow

#### Step 1: Build Development Environment
```bash
# Clone and build
git clone https://github.com/tri2510/vehicle-app-cpp-template.git
cd vehicle-app-cpp-template
docker build -f Dockerfile.dev -t velocitas-dev .

# With proxy (if needed):
docker build -f Dockerfile.dev \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --build-arg http_proxy=http://127.0.0.1:3128 \
  --build-arg https_proxy=http://127.0.0.1:3128 \
  --network=host \
  -t velocitas-dev .
```

#### Step 2: Build Your Vehicle App
```bash
# Enter development container
docker run -it --privileged -v $(pwd):/workspace \
  --network=host velocitas-dev

# With proxy (if needed):
docker run -it --privileged -v $(pwd):/workspace \
  --network=host \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  -e http_proxy=http://127.0.0.1:3128 \
  -e https_proxy=http://127.0.0.1:3128 \
  velocitas-dev

# Build the minimal app
gen-model      # Generate Vehicle.Speed model from VSS
install-deps   # Install Velocitas SDK
build-app      # Compile app.cpp into executable
```

#### Step 3: Run and Test
```bash
# Start Vehicle Data Broker (separate terminal)
docker run -d -p 55555:55555 ghcr.io/eclipse/kuksa.val/databroker:0.4.3

# Run your vehicle app (in container)
run-app

# Test with speed data (another terminal)
docker run -it --rm --network=host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main \
  "grpc://localhost:55555"

# In Kuksa client, set speed:
setValue Vehicle.Speed 25.0    # App will log: "25.00 m/s (90.0 km/h)"
```

**That's it! 🎉 Your minimal vehicle app is running and logging speed data.**

## 📁 Ultra-Simple Project Structure

```
vehicle-app-cpp-template/
├── app/
│   ├── app.cpp              # 🎯 EVERYTHING in one file
│   ├── AppManifest.json     # Minimal config (Vehicle.Speed only)
│   └── CMakeLists.txt       # Simple build (one source file)
├── Dockerfile.dev           # Complete development environment  
├── docker-compose.dev.yml   # Runtime services (VDB)
└── README.md               # This file
```

## 🔧 The Minimal App (`app/app.cpp`)

**Complete vehicle app in ~80 lines:**

```cpp
#include "sdk/VehicleApp.h"
#include "sdk/Logger.h"
#include "vehicle/Vehicle.hpp"

class MyApp : public velocitas::VehicleApp {
public:
    MyApp(std::shared_ptr<velocitas::IVehicleDataBrokerClient> vdbClient)
        : VehicleApp(vdbClient, nullptr) {}  // No MQTT client needed

protected:
    void onStart() override {
        // Subscribe to Vehicle.Speed only
        subscribeDataPoints(QueryBuilder::select(m_vehicle->Speed).build())
            ->onItem([this](auto&& item) { 
                auto speed = item.get(m_vehicle->Speed)->value();
                logger().info("Speed: {:.2f} m/s ({:.1f} km/h)", 
                             speed, speed * 3.6);
            });
    }

private:
    std::unique_ptr<vehicle::Vehicle> m_vehicle;
};

int main() {
    auto vdbClient = VehicleDataBrokerClient::createInstance("localhost:55555");
    MyApp app(vdbClient);
    app.run();
    return 0;
}
```

## 🛠️ Development Commands

### Build & Run
```bash
gen-model         # Generate Vehicle.Speed class from VSS
install-deps      # Install Velocitas SDK and dependencies  
build-app         # Compile app.cpp into executable
run-app           # Run the vehicle app
```

### Testing
```bash
# Start VDB first
docker run -d -p 55555:55555 ghcr.io/eclipse/kuksa.val/databroker:0.4.3

# Test with Kuksa client
docker run -it --rm --network=host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main \
  "grpc://localhost:55555"

# Set speed values:
setValue Vehicle.Speed 15.0    # City driving
setValue Vehicle.Speed 30.0    # Highway driving  
setValue Vehicle.Speed 0.0     # Stopped
```

## ⚡ What's Different (Ultra-Minimal)

### Before (Complex SpeedMonitorApp):
- ❌ 500+ lines across multiple files
- ❌ MQTT broker dependency
- ❌ Complex alert systems
- ❌ Configuration management
- ❌ Statistics tracking
- ❌ Multiple vehicle signals

### After (Ultra-Minimal):
- ✅ **~80 lines in ONE file**
- ✅ **No MQTT** - only VDB connection
- ✅ **One signal** - Vehicle.Speed only
- ✅ **No headers** - everything in app.cpp
- ✅ **No launcher** - main() included
- ✅ **Zero complexity** - perfect for learning

## 🎯 Key Features

### ✅ Ultra-Simple
- Single source file contains everything
- One vehicle signal (Vehicle.Speed)  
- No MQTT, no complexity, no confusion

### ✅ Full Velocitas SDK
- Complete C++ toolchain integration
- Vehicle Signal Specification (VSS) support
- Vehicle Data Broker communication
- Production-ready foundation

### ✅ Docker-First Development
- No devcontainer setup required
- Complete development environment in Docker
- Works with any IDE or editor

### ✅ Perfect Learning Platform
- Minimal example to understand Velocitas
- Easy to extend with more signals
- Clear path to production apps

## 📖 Next Steps

### Extend Your App
```cpp
// Add more vehicle signals to AppManifest.json:
{
  "path": "Vehicle.Acceleration.Longitudinal", 
  "access": "read"
}

// Subscribe in onStart():
subscribeDataPoints(QueryBuilder::select(
  m_vehicle->Speed, 
  m_vehicle->Acceleration.Longitudinal
).build())
```

### Add MQTT (Optional)
1. Add `pubsub` interface to AppManifest.json
2. Pass MQTT client to MyApp constructor  
3. Use `publishDataPoint()` to send messages

### Scale to Production
- Add error handling and logging
- Implement configuration management
- Add unit tests and CI/CD
- Package as production container

## 📋 Requirements

- **Docker**: Version 20.10+ recommended
- **System**: Linux, macOS, or Windows with WSL2
- **Memory**: 2GB+ RAM for development
- **Network**: Access to pull Velocitas base images

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## 🏷️ Version

- **Template**: Ultra-Minimal v1.0
- **Velocitas SDK**: >= 0.7.0
- **Base Image**: eclipse-velocitas/devcontainer-base-images/cpp:v0.4

---

**The simplest way to start with vehicle app development! 🚗⚡**

*Perfect for Docker-based build engines and learning Velocitas fundamentals.*