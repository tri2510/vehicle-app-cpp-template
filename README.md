# Vehicle App C++ Template - Cache Fixed

A zero-setup containerized build environment for Eclipse Velocitas C++ vehicle applications with **fixed cache invalidation** that ensures your new code is always built correctly.

## What's Fixed

🐛 **Problem**: Old cached builds were causing new C++ files to run stale code instead of fresh compilation.

✅ **Solution**: Added SHA256-based file change detection that automatically cleans build artifacts when source code changes.

## Quick Start

### 1. Build the Container

```bash
# For corporate networks with proxy
docker build --build-arg HTTP_PROXY=http://127.0.0.1:3128 --build-arg HTTPS_PROXY=http://127.0.0.1:3128 --build-arg http_proxy=http://127.0.0.1:3128 --build-arg https_proxy=http://127.0.0.1:3128 --network host -f Dockerfile.quick -t velocitas-quick .

# For regular networks
docker build -f Dockerfile.quick -t velocitas-quick .
```

### 2. Start Persistent Engine (Recommended)

```bash
# Start the always-on build engine
./velocitas-engine start

# Build your apps instantly
./velocitas-engine build example-app.cpp
./velocitas-engine build my-vehicle-app.cpp

# Check engine status
./velocitas-engine status
```

### 3. Alternative: One-time Builds

```bash
# Build your VehicleApp.cpp (slower, no cache)
docker run --rm -v $(pwd)/your-app.cpp:/input velocitas-quick build

# With proxy (if needed)
docker run --rm -v $(pwd)/your-app.cpp:/input -e HTTP_PROXY=http://127.0.0.1:3128 -e HTTPS_PROXY=http://127.0.0.1:3128 -e http_proxy=http://127.0.0.1:3128 -e https_proxy=http://127.0.0.1:3128 --network host velocitas-quick build
```

## Example Vehicle App

Create a file `my-vehicle-app.cpp`:

```cpp
#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include <iostream>

using namespace velocitas;

class VehicleApp : public App {
public:
    void onStart() override {
        std::cout << "=== My Vehicle App Started ===" << std::endl;
        
        // Monitor vehicle speed
        Vehicle.Speed->subscribe(DataPointReply::newBuilder()->build())
        ->onResult([](auto&& result) {
            if (result.isOk()) {
                auto speed = result.value().getValue();
                std::cout << "🚗 Current Speed: " << speed << " km/h" << std::endl;
                
                if (speed > 80) {
                    std::cout << "⚠️  High speed detected!" << std::endl;
                }
            }
        });
        
        std::cout << "🚀 Speed monitoring active..." << std::endl;
    }
};

REGISTER_MAIN(VehicleApp)
```

Then build it:

```bash
# With persistent engine (instant)
./velocitas-engine build my-vehicle-app.cpp

# Or one-time build (slower)
docker run --rm -v $(pwd)/my-vehicle-app.cpp:/input velocitas-quick build
```

## Persistent Engine Commands

| Command | Purpose | Example |
|---------|---------|---------|
| `start` | Start persistent engine | `./velocitas-engine start` |
| `build <file>` | Build app instantly | `./velocitas-engine build my-app.cpp` |
| `run <file>` | Build and run app | `./velocitas-engine run my-app.cpp` |
| `status` | Show engine and cache status | `./velocitas-engine status` |
| `stop` | Stop engine | `./velocitas-engine stop` |
| `restart` | Restart engine | `./velocitas-engine restart` |

## One-time Build Commands

| Command | Purpose | Example |
|---------|---------|---------|
| `build` | Compile your VehicleApp.cpp | `docker run --rm -v $(pwd)/app.cpp:/input velocitas-quick build` |
| `run` | Build and run with live output | `docker run --rm -v $(pwd)/app.cpp:/input velocitas-quick run` |
| `validate` | Check your code syntax | `docker run --rm -v $(pwd)/app.cpp:/input velocitas-quick validate` |

## Cache Behavior

✅ **New/Changed Files**: Automatically rebuilds with fresh compilation

✅ **Build Success**: Shows `✅ C++ compilation completed successfully`

✅ **Cache Status**: Displays `🎯 Cache invalidation is working correctly`

## Input Methods

```bash
# Mount file
docker run --rm -v $(pwd)/VehicleApp.cpp:/input velocitas-quick build

# Mount directory  
docker run --rm -v $(pwd):/input velocitas-quick build

# Pipe from stdin
cat VehicleApp.cpp | docker run --rm -i velocitas-quick build
```

## Requirements

- Docker
- Your VehicleApp.cpp file

## Features

- 🚀 **Zero Setup**: No local SDK installation needed
- ⚡ **Persistent Engine**: Always-on container for instant builds
- 💾 **Smart Caching**: Dependencies, models, and builds persist between runs
- 🔧 **Cache Fixed**: New files always trigger fresh builds, identical files use cache
- 🌐 **Proxy Support**: Works in corporate networks
- 📦 **Pre-built Dependencies**: Conan, CMake, Vehicle SDK ready
- 🔍 **Input Validation**: Checks your code before building
- 🎯 **Instant Builds**: 10-20 seconds for cached builds vs 2-4 minutes fresh

## Troubleshooting

**Build fails with dependency errors**:
- Ensure you're using correct Vehicle SDK includes: `#include "sdk/VehicleApp.h"`

**Proxy connection issues**:
- Add proxy environment variables to your docker run command

**File not found**:
- Use absolute paths: `docker run --rm -v $(pwd)/app.cpp:/input ...`

## What's Under the Hood

This template uses:
- **Eclipse Velocitas SDK** - Vehicle application framework
- **Docker containerization** - Zero-setup build environment  
- **SHA256 cache invalidation** - Ensures new code is always built
- **CMake + Conan** - Modern C++ build system
- **Vehicle Signal Specification (VSS)** - Automotive data standards

Your vehicle app can subscribe to signals like `Vehicle.Speed`, `Vehicle.Engine.RPM`, publish MQTT messages, and integrate with the Vehicle Data Broker.