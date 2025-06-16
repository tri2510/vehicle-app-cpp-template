# Vehicle App C++ Template - Docker Development Environment

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

A modern, Docker-only development environment for creating [Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) Vehicle Apps in C++. No devcontainer setup required - just Docker!

## 🚀 Quick Start

### Prerequisites
- [Docker](https://docker.com) installed and running
- Git for version control

### Development Workflow - 4 Simple Steps

#### Step 1: Build Development Environment
```bash
# Clone the repository
git clone https://github.com/tri2510/vehicle-app-cpp-template.git
cd vehicle-app-cpp-template

# Build development container
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

# Inside container - build your app
gen-model      # Generate C++ classes from Vehicle Signal Specification (VSS)
install-deps   # Install C++ dependencies using Conan package manager
build-app      # Compile the vehicle application with CMake/Ninja
```

#### Step 3: Start Runtime Services
```bash
# In a separate terminal, start MQTT broker and Vehicle Data Broker
docker compose -f docker-compose.dev.yml up mosquitto vehicledatabroker -d

# Optional: Start Kuksa client for testing (in another terminal)
docker compose -f docker-compose.dev.yml up kuksa-client

# Or use the standalone Kuksa client:
docker run -it --rm \
  --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main \
  "grpc://localhost:55555"
```

#### Step 4: Run and Test Your App
```bash
# Back in the development container, run your app
run-app

# Your vehicle app is now running and connected to:
# - MQTT Broker (localhost:1883)
# - Vehicle Data Broker (localhost:55555)
```

**That's it! 🎉 Your vehicle app is running and ready for development.**

## 🏗️ Docker Architecture

### Development Environment Components

| Component | Purpose | Port |
|-----------|---------|------|
| **Development Container** | Complete C++ toolchain with Velocitas SDK | - |
| **MQTT Broker** | Message communication (Eclipse Mosquitto) | 1883, 9001 |
| **Vehicle Data Broker** | Vehicle signal management (KUKSA.val) | 55555 |
| **Kuksa Client** | Interactive vehicle data testing client | - |
| **Development Tools** | Build, test, lint, format, debug tools | - |

### Docker Files

- **`Dockerfile.dev`** - Complete development environment
- **`docker-compose.dev.yml`** - Full development stack with services
- **`config/mosquitto.conf`** - MQTT broker configuration

## 🛠️ Development Commands

The Docker environment provides convenient commands for all development tasks:

### Build & Dependencies
```bash
gen-model         # Input: VSS spec (JSON) → Output: C++ classes in Vehicle.hpp
                  # Downloads VSS 4.0 spec and generates Vehicle.Speed, Vehicle.Acceleration classes
                  # Note: To use custom VSS signals, change "src" in app/AppManifest.json to your local VSS file
install-deps      # Input: conanfile.txt → Output: Downloaded C++ libraries
                  # Installs Velocitas SDK, fmt, nlohmann_json, gRPC to ~/.conan2/
build-app         # Input: C++ source files → Output: Executable in build/bin/app
build-app -r      # Same as above but optimized Release build
```

### Runtime Services
```bash
run-app           # Input: build/bin/app → Output: Running vehicle app with logs
                  # Note: Requires MQTT broker and VDB to be started separately via Docker Compose
```

### Code Quality & Testing
```bash
check-code        # Input: C++ source files → Output: Lint/format reports and fixes
./build/bin/app_utests  # Input: Test executable → Output: Test results and coverage
```

### Development Tools
```bash
gen-model         # Input: VSS spec URL → Output: Generated Vehicle.hpp classes
gen-grpc          # Input: .proto files → Output: Generated gRPC client/server code
vdb-cli           # Input: Commands → Output: Interactive VDB management console
```

## 📁 Project Structure

```
vehicle-app-cpp-template/
├── 🐳 Docker Development Environment
│   ├── Dockerfile.dev              # Development container
│   ├── docker-compose.dev.yml      # Full development stack
│   └── config/mosquitto.conf       # MQTT configuration
├── 📱 Vehicle Application
│   ├── app/src/                     # C++ source code
│   ├── app/tests/                   # Unit tests
│   ├── AppManifest.json            # App configuration
│   └── CMakeLists.txt               # Build configuration
├── 🔧 Development Configuration
│   ├── conanfile.txt               # C++ dependencies
│   ├── requirements.txt            # Python dependencies
│   └── .velocitas.json             # Velocitas configuration
├── 📖 Example Applications
│   └── examples/                   # Example vehicle apps
└── 📚 Documentation
    ├── DOCKER_DEVELOPMENT.md      # Complete development guide
    └── README.md                   # This file
```

## 🚗 Vehicle Application - Speed Monitor & Alert System

This repository contains a **production-ready Speed Monitor & Alert System** that demonstrates real-world vehicle application development using the Eclipse Velocitas framework.

### ✅ Speed Monitor & Alert System
- **Real-time speed monitoring** with configurable speed limits
- **Speed limit violation detection** with instant MQTT alerts  
- **Hard braking and rapid acceleration detection** for safety monitoring
- **MQTT-based configuration** for dynamic threshold updates
- **Comprehensive statistics** tracking and reporting
- **Status:** Complete with source code, comprehensive tests, and documentation

### 🔧 Features Demonstrated
- **Vehicle Signal Integration:** Real-time data from Vehicle.Speed and Vehicle.Acceleration.Longitudinal
- **MQTT Communication:** Bi-directional messaging for alerts and configuration
- **Event-Driven Architecture:** Asynchronous processing with error handling
- **Configurable Thresholds:** Dynamic speed limits and alert settings
- **Alert Cooldown Logic:** Prevents alert spam with intelligent timing
- **Production-Ready:** Full error handling, logging, and robust design

### 📊 MQTT Topics
- **`speedmonitor/config`** - Receive configuration updates
- **`speedmonitor/alerts`** - Publish speed violations and safety alerts
- **`speedmonitor/status`** - Publish current speed and system status
- **`speedmonitor/statistics`** - Publish performance metrics

This example provides a complete foundation for building sophisticated vehicle monitoring applications.

## 🔧 Development Workflow

### Recommended: Separate Container Workflow
This is the most reliable approach for vehicle app development:

```bash
# Terminal 1: Start runtime services
docker compose -f docker-compose.dev.yml up mosquitto vehicledatabroker -d

# Terminal 2: Development container
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

# Inside development container:
gen-model && install-deps && build-app && run-app
# 1. Generate vehicle classes from VSS spec
# 2. Install dependencies (Velocitas SDK, etc.)  
# 3. Compile the C++ application
# 4. Run the vehicle app
```

### Alternative: Single Container Development
For development without separate services (limited functionality):

```bash
# Single container for development only
docker run -it --privileged -v $(pwd):/workspace \
  --network=host velocitas-dev

# Inside container:
gen-model && install-deps && build-app
# Note: run-app requires separate MQTT broker and VDB via Docker Compose
```

### Using Docker Compose
For a complete development environment:

```bash
# Start all services including dev container
docker compose -f docker-compose.dev.yml up -d

# Access development container
docker compose -f docker-compose.dev.yml exec dev bash

# Start Kuksa client for testing (separate terminal)
docker compose -f docker-compose.dev.yml up kuksa-client --profile testing
```

## 🧪 Testing Your Vehicle App

### Unit Testing
```bash
# In development container
build-app
./build/bin/app_utests
```

### Integration Testing with Real Services
```bash
# Terminal 1: Start runtime services
docker compose -f docker-compose.dev.yml up mosquitto vehicledatabroker -d

# Terminal 2: Development container (with proxy if needed)
docker run -it --privileged -v $(pwd):/workspace \
  --network=host \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  -e http_proxy=http://127.0.0.1:3128 \
  -e https_proxy=http://127.0.0.1:3128 \
  velocitas-dev

# Inside development container:
gen-model && install-deps && build-app && run-app
```

### Testing Vehicle Data with Kuksa Client
```bash
# Option 1: Using Docker Compose (recommended)
docker compose -f docker-compose.dev.yml up kuksa-client

# Option 2: Standalone Kuksa client
docker run -it --rm \
  --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main \
  "grpc://localhost:55555"

# In Kuksa client, send vehicle data:
setValue Vehicle.Speed 25.0                    # Triggers speed monitoring (25 m/s = 90 km/h)
setValue Vehicle.Acceleration.Longitudinal -6.0 # Triggers braking alert
getValue Vehicle.Speed                          # Check current value
```

### Testing MQTT Communication
```bash
# Send MQTT configuration updates
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_pub \
  -h localhost -t "speedmonitor/config" \
  -m '{"speed_limit_kmh": 60.0, "enable_speed_limit_alerts": true}'

# Monitor MQTT alerts
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_sub \
  -h localhost -t "speedmonitor/alerts"
```

### Code Quality
```bash
# Run all quality checks
check-code

# Individual tools available:
# - clang-format (formatting)
# - clang-tidy (static analysis)  
# - cpplint (style checking)
# - cppcheck (error detection)
```

## 🎯 Key Features

### ✅ Zero Setup Complexity
- No devcontainer configuration required
- No VS Code dependencies
- No complex installation procedures
- Works with any IDE or editor

### ✅ Complete Development Environment
- Full C++ toolchain (GCC, CMake, Ninja)
- Velocitas SDK >= 0.7.0 integrated
- Conan 2.x package management
- All development tools included

### ✅ Vehicle-Specific Tools
- Vehicle Data Broker (KUKSA.val) integration
- MQTT communication (Eclipse Mosquitto)
- Vehicle Signal Specification (VSS) support
- gRPC SDK generation

### ✅ Production Ready
- Multi-stage Docker builds
- Optimized container images
- Environment configuration support
- Deployment-ready containers

## 🚀 Deployment

### Building Production Images
```bash
# Build your app first
build-app

# Create production container
docker build -f app/Dockerfile -t my-vehicle-app .

# Run in production
docker run -d --name vehicle-app \
  -e SDV_MQTT_ADDRESS=production-mqtt:1883 \
  -e SDV_VEHICLEDATABROKER_ADDRESS=production-vdb:55555 \
  my-vehicle-app
```

## 📖 Documentation

- **[DOCKER_DEVELOPMENT.md](DOCKER_DEVELOPMENT.md)** - Complete development guide
- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation
- **[Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)** - VSS reference
- **[KUKSA.val](https://github.com/eclipse/kuksa.val)** - Vehicle Data Broker

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes in the Docker environment
4. Test thoroughly: `check-code && build-app && ./build/bin/app_utests`
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

## 🆚 Migration from DevContainer

This repository has been migrated from a devcontainer-based setup to a pure Docker development environment. Benefits include:

### Before (DevContainer)
- ❌ VS Code dependency
- ❌ Complex devcontainer configuration
- ❌ IDE-specific setup
- ❌ Limited development flexibility

### After (Docker-Only)
- ✅ IDE/Editor agnostic
- ✅ Simple Docker workflow
- ✅ Faster setup and build
- ✅ Better portability
- ✅ Cleaner repository structure

### Migration Guide
If you have an existing devcontainer-based setup:

1. **Remove devcontainer files** (already done in this repo)
2. **Switch to Docker workflow**: Use `Dockerfile.dev` instead of devcontainer
3. **Update documentation**: Follow this README instead of devcontainer docs
4. **Rebuild environment**: `docker build -f Dockerfile.dev -t velocitas-dev .`

## 📋 Requirements

### System Requirements
- **Docker**: Version 20.10+ recommended
- **Operating System**: Linux, macOS, or Windows with WSL2
- **Memory**: 4GB+ RAM recommended for development
- **Storage**: 2GB+ free space for Docker images

### Velocitas Requirements
- **Velocitas C++ SDK**: >= 0.7.0 (included)
- **Conan**: 2.x (included) 
- **Vehicle Signals**: VSS 4.0 compatible (included)
- **Communication**: MQTT and gRPC support (included)

## 🐛 Troubleshooting

### Common Issues

**Docker build fails**
```bash
# Clean Docker cache and rebuild
docker system prune -f
docker build --no-cache -f Dockerfile.dev -t velocitas-dev .
```

**Permission errors**
```bash
# Fix file permissions inside container
docker run --rm -v $(pwd):/workspace --privileged velocitas-dev \
  bash -c "chown -R vscode:vscode /workspace"
```

**Port conflicts**
```bash
# Check for conflicting services
docker ps
netstat -tulpn | grep -E '(1883|55555|8080)'

# Use different ports
docker run -it --privileged -v $(pwd):/workspace \
  -p 8081:8080 -p 1884:1883 -p 55556:55555 velocitas-dev
```

**Build errors**
```bash
# Clean and rebuild
rm -rf build-linux-x86_64
install-deps && build-app
```

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## 🏷️ Version Information

- **Template Version**: Docker-enabled v1.0
- **Velocitas SDK**: >= 0.7.0  
- **Conan**: 2.15.1+
- **Base Image**: eclipse-velocitas/devcontainer-base-images/cpp:v0.4

---

**Happy Vehicle App Development! 🚗💨**

For detailed development instructions, see [DOCKER_DEVELOPMENT.md](DOCKER_DEVELOPMENT.md).