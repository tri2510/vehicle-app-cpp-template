# Vehicle App C++ Template - Docker Development Environment

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

A modern, Docker-only development environment for creating [Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) Vehicle Apps in C++. No devcontainer setup required - just Docker!

## 🚀 Quick Start

### Prerequisites
- [Docker](https://docker.com) installed and running
- Git for version control

### Get Started in 3 Steps
```bash
# 1. Clone the repository
git clone https://github.com/tri2510/vehicle-app-cpp-template.git
cd vehicle-app-cpp-template

# 2. Build development environment
docker build -f Dockerfile.dev -t velocitas-dev .

# 3. Start developing
docker run -it --privileged -v $(pwd):/workspace \
  -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev
```

### Inside the Container
```bash
# Install dependencies and build
install-deps && build-app

# Run your vehicle app
run-app

# Start development services
runtime-up
```

**That's it! 🎉 You're ready to develop vehicle applications.**

## 🏗️ Docker Architecture

### Development Environment Components

| Component | Purpose | Port |
|-----------|---------|------|
| **Development Container** | Complete C++ toolchain with Velocitas SDK | - |
| **MQTT Broker** | Message communication (Eclipse Mosquitto) | 1883, 9001 |
| **Vehicle Data Broker** | Vehicle signal management (KUKSA.val) | 55555 |
| **Development Tools** | Build, test, lint, format, debug tools | - |

### Docker Files

- **`Dockerfile.dev`** - Complete development environment
- **`docker-compose.dev.yml`** - Full development stack with services
- **`config/mosquitto.conf`** - MQTT broker configuration

## 🛠️ Development Commands

The Docker environment provides convenient commands for all development tasks:

### Build & Dependencies
```bash
install-deps       # Install C++ dependencies via Conan
build-app         # Build the vehicle application  
build-app -r      # Build in Release mode
```

### Runtime Services
```bash
runtime-up        # Start MQTT broker and Vehicle Data Broker
runtime-down      # Stop runtime services
run-app           # Run the vehicle app with services
```

### Code Quality & Testing
```bash
check-code        # Run linting, formatting, and static analysis
./build/bin/app_utests  # Run unit tests
```

### Development Tools
```bash
gen-model         # Generate vehicle model from VSS
gen-grpc          # Generate gRPC SDKs
vdb-cli           # Access Vehicle Data Broker CLI
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

## 🚗 Example Vehicle Applications

This repository includes comprehensive examples demonstrating different aspects of vehicle app development:

### 1. Speed Monitor & Alert System
- Real-time speed monitoring with configurable alerts
- Speed limit violation detection  
- Hard braking and rapid acceleration detection
- MQTT-based configuration and alerting

### 2. Coming Soon
- Fuel Efficiency Tracker
- Maintenance Reminder System
- Parking Assistant
- Climate Control Optimizer

Each example includes complete source code, tests, and detailed tutorials.

## 🔧 Development Workflow

### Option 1: Quick Development (Recommended)
```bash
# Single container with all tools
docker run -it --privileged -v $(pwd):/workspace \
  -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

# Inside container
install-deps && build-app && run-app
```

### Option 2: Full Development Stack
```bash
# Complete environment with all services
docker-compose -f docker-compose.dev.yml up -d

# Access development container
docker-compose -f docker-compose.dev.yml exec dev bash
```

### Option 3: Custom Development
```bash
# Build custom image with your modifications
docker build -f Dockerfile.dev -t my-velocitas-dev .

# Run with custom configuration
docker run -it --privileged -v $(pwd):/workspace \
  -e SDV_MQTT_ADDRESS=my-broker:1883 \
  my-velocitas-dev
```

## 🧪 Testing Your Vehicle App

### Unit Testing
```bash
# Build and run tests
build-app
./build/bin/app_utests
```

### Integration Testing
```bash
# Start services
runtime-up

# Test with real services
run-app

# Send test MQTT messages
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_pub \
  -h localhost -t "sampleapp/getSpeed" -m '{"requestId": "test"}'
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