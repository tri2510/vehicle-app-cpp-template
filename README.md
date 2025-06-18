# Velocitas C++ Quick Build Template

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

🚀 **Zero-setup Docker utility** for building [Eclipse Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) vehicle applications in C++. Enhanced template with quick build, corporate proxy support, custom VSS specifications, and comprehensive learning guide.

> **Based on [Eclipse Velocitas Vehicle App Template](https://github.com/eclipse-velocitas/vehicle-app-cpp-template)** - Enhanced for instant containerized building without local development environment setup.

## ⚡ Ultra-Fast Quick Start

### Quick Build Setup
```bash
# 1. Build the utility container (one-time setup, 3-5 minutes)
docker build -f Dockerfile.quick -t velocitas-quick .

# 2. Run the pre-built template instantly (no input needed)
docker run --rm --network=host velocitas-quick rerun

# 3. Build your custom VehicleApp.cpp
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick
```

💡 **Want to use pre-built images?** See **[PREBUILT_IMAGES.md](PREBUILT_IMAGES.md)** for instant building without local setup.

### 📚 **Learning Template Available**
New to Velocitas? Check out the comprehensive template that shows you how to configure vehicle signals:
```bash
# View the instructive template with signal examples
cat templates/app/src/VehicleApp.template.cpp

# Build the template to see how it works
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick
```
The template includes:
- **50+ vehicle signal examples** with descriptions
- **Step-by-step signal subscription** guide
- **Common use case patterns** (speed monitoring, fuel tracking, etc.)
- **VSS customization** instructions

## 🎯 Key Features

✅ **Zero Dependencies** - No host installation required  
✅ **Lightning Fast** - Pre-compiled dependencies (60-90 seconds total)  
✅ **Proxy Ready** - Full corporate network support  
✅ **VSS Flexible** - Use default VSS 4.0 or custom specifications  
✅ **Multi-Input** - stdin, file mount, or directory mount  
✅ **Validation** - Code validation without full build  
✅ **Production Ready** - Optimized executable output  
✅ **CI/CD Optimized** - Perfect for automated pipelines  
✅ **Instant Demo** - Pre-built template runs immediately after container build  

---

## 🔧 Proxy Support (Corporate Networks)

```bash
# Build with proxy support
docker build -f Dockerfile.quick \
  --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
  --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
  --network=host \
  -t velocitas-quick .

# Use with runtime proxy
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  velocitas-quick
```

## 📝 Multiple Input Methods

```bash
# Method 1: Pipe from stdin (fastest)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick

# Method 2: Mount single file
docker run --rm -v $(pwd)/templates/app/src/VehicleApp.template.cpp:/input velocitas-quick

# Method 3: Mount entire directory
docker run --rm -v $(pwd):/input velocitas-quick

# Method 4: Validation only (no build)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick validate

# Method 5: Build and run with services (smart rebuild)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i --network=host velocitas-quick run

# Method 6: Run pre-built template (no input needed, fastest)
docker run --rm --network=host velocitas-quick rerun

# Method 7: Granular build steps
docker run --rm velocitas-quick gen-model  # Step 3: Generate vehicle model only
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick compile  # Step 4: Compile only
docker run --rm velocitas-quick finalize  # Step 5: Build summary
```

## 🎛️ Custom VSS Support

```bash
# Use custom VSS file
docker run --rm -i \
  -v $(pwd)/my-custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp

# Use custom VSS URL
docker run --rm -i \
  -e VSS_SPEC_URL=https://company.com/vehicle-signals.json \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp
```

## 🏢 Corporate Integration Examples

```bash
# Jenkins/CI Pipeline
curl -s $BUILD_SERVER/latest/VehicleApp.cpp | \
  docker run --rm -i -e HTTP_PROXY=$CORPORATE_PROXY velocitas-quick

# GitLab CI with template example
docker run --rm -i \
  -e VSS_SPEC_URL=$COMPANY_VSS_ENDPOINT \
  -e HTTP_PROXY=$CORPORATE_PROXY \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp

# Azure DevOps with file mount
docker run --rm \
  -v $BUILD_SOURCESDIRECTORY/src:/input \
  -e HTTPS_PROXY=$AGENT_PROXY \
  velocitas-quick
```

---

## 🧪 Testing & Validation

### Automated Testing Script

```bash
# Run comprehensive test suite (no proxy)
./test-mode2.sh

# Run tests with proxy
./test-mode2.sh --proxy

# Custom proxy and timeout
./test-mode2.sh --proxy --proxy-host company-127.0.0.1:3128 --timeout 180

# Custom output directory
./test-mode2.sh --output my_test_results
```

The test script validates:
- Container building with/without proxy
- Multiple input methods (stdin, file mount, directory mount)
- Custom VSS support
- Validation functionality
- Error handling
- Build performance and reliability

### Manual Testing Examples

```bash
# Test with instructive template (shows how to use vehicle signals)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick

# Test validation only
echo 'invalid code' | docker run --rm -i velocitas-quick validate

# Test with custom VSS
docker run --rm -i \
  -e VSS_SPEC_URL=https://raw.githubusercontent.com/COVESA/vehicle_signal_specification/main/spec/VehicleSignalSpecification.json \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp
```

### Testing with Live Services

```bash
# 1. Start Vehicle Data Broker
docker compose -f docker-compose.dev.yml up vehicledatabroker -d

# 2. Run your app with services
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i --network=host velocitas-quick run

# 3. Test with KUKSA client (in another terminal)
docker run -it --rm --network=host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main
# Inside kuksa-client: setValue Vehicle.Speed 65.0

# 4. Stop services
docker compose -f docker-compose.dev.yml down
```

---

## 🚗 Vehicle Application Examples

This template includes a comprehensive **Speed Monitor & Alert System** demonstrating production-ready vehicle application development.

### ✅ Speed Monitor & Alert System
- **Real-time speed monitoring** with configurable limits
- **Safety event detection** (hard braking, rapid acceleration)
- **MQTT communication** for alerts and configuration
- **Comprehensive statistics** and performance tracking
- **Production-ready** with full error handling

### Example Implementation Features
- **Vehicle Signal Integration:** `Vehicle.Speed`, `Vehicle.Acceleration.Longitudinal`
- **MQTT Topics:** `speedmonitor/alerts`, `speedmonitor/config`, `speedmonitor/status`  
- **Event-Driven Architecture:** Asynchronous processing with robust error handling
- **Configurable Thresholds:** Dynamic speed limits and alert cooldowns

### Additional Examples Available
- **Fuel Efficiency Tracker** - Consumption monitoring and optimization
- **Maintenance Reminder System** - Proactive maintenance based on diagnostics
- **Parking Assistant** (Tutorial) - Proximity sensor guidance
- **Climate Control Optimizer** (Tutorial) - Intelligent HVAC management

---

## 📁 Project Structure

```
vehicle-app-cpp-template/
├── 🚀 Quick Build Utility (Main)
│   ├── Dockerfile.quick             # Main utility container
│   ├── scripts/quick-build.sh       # Entry script
│   ├── scripts/quick-run.sh         # Build and run script
│   ├── scripts/validate-template.sh # Validation script
│   └── templates/                   # Fixed configurations & learning template
├── 🧪 Testing & Validation
│   ├── test-mode2.sh               # Automated test script
│   └── test_results/               # Test output logs
├── 🔧 Configuration
│   ├── conanfile.txt               # C++ dependencies
│   ├── requirements.txt            # Python dependencies
│   └── .velocitas.json             # Velocitas configuration
├── 🛠️ Traditional Development (Optional)
│   ├── docker-compose.dev.yml      # Complete development stack
│   └── config/mosquitto.conf       # MQTT configuration
├── 🔄 CI/CD & Automation
│   └── .github/workflows/          # GitHub Actions for builds & releases
└── 📚 Documentation
    ├── README.md                   # This file
    ├── PREBUILT_IMAGES.md          # Pre-built Docker images guide
    ├── DEVELOPER_WORKFLOW.md       # Complete development workflows
    └── NOTICE                      # License attribution
```

---

## 🏢 Production & Enterprise Use

### CI/CD Integration

```yaml
# GitHub Actions Example
- name: Build Vehicle App
  run: |
    docker build -f Dockerfile.quick -t builder .
    cat src/VehicleApp.cpp | docker run --rm -i \
      -e VSS_SPEC_URL=${{ secrets.COMPANY_VSS_URL }} \
      -e HTTP_PROXY=${{ secrets.CORPORATE_PROXY }} \
      builder > app-executable

# Jenkins Pipeline Example  
pipeline {
    agent any
    steps {
        script {
            sh '''
                docker build -f Dockerfile.quick -t velocitas-quick .
                docker run --rm -v ${WORKSPACE}/src:/input \
                  -e HTTP_PROXY=${CORPORATE_PROXY} \
                  velocitas-quick
            '''
        }
    }
}
```

### Corporate Network Configuration

```bash
# Set persistent proxy environment
export HTTP_PROXY=http://corporate-127.0.0.1:3128
export HTTPS_PROXY=http://corporate-127.0.0.1:3128

# Build with corporate settings
docker build -f Dockerfile.quick \
  --build-arg HTTP_PROXY=$HTTP_PROXY \
  --build-arg HTTPS_PROXY=$HTTPS_PROXY \
  --network=host \
  -t velocitas-quick .

# Use with company VSS specification
docker run --rm -i \
  -e VSS_SPEC_URL=https://company.com/vss/spec.json \
  -e HTTP_PROXY=$HTTP_PROXY \
  velocitas-corporate < templates/app/src/VehicleApp.template.cpp
```

---

## 🚀 Performance & Benchmarks

### Performance Metrics
- **Container Build Time:** 3-5 minutes (one-time)
- **App Build Time:** 60-90 seconds (cached dependencies)
- **Memory Usage:** ~2GB during build, ~500MB runtime
- **Executable Size:** ~13-15MB optimized binary
- **Network:** Proxy-friendly with minimal external dependencies

### Optimization Tips
```bash
# Pre-build container for CI/CD
docker build -f Dockerfile.quick -t velocitas-quick .
docker push your-registry/velocitas-quick:latest

# Use in pipeline
docker run --rm -i your-registry/velocitas-quick:latest < templates/app/src/VehicleApp.template.cpp
```

---

## 🔧 Advanced Configuration

### Custom VSS Specifications

```bash
# Option 1: Mount custom VSS file
docker run --rm -i \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp

# Option 2: Use VSS URL (supports authentication)
docker run --rm -i \
  -e VSS_SPEC_URL=https://api.company.com/vss/v2.0/spec.json \
  -e VSS_AUTH_TOKEN=your-token \
  velocitas-quick < templates/app/src/VehicleApp.template.cpp
```

### Build Customization

```bash
# Debug build with symbols
docker run --rm -i -e BUILD_TYPE=Debug velocitas-quick < templates/app/src/VehicleApp.template.cpp

# Custom build flags
docker run --rm -i -e CMAKE_FLAGS="-DCUSTOM_FLAG=ON" velocitas-quick < templates/app/src/VehicleApp.template.cpp

# Verbose build output (shows detailed command output)
docker run --rm -i -e VERBOSE_BUILD=1 velocitas-quick < templates/app/src/VehicleApp.template.cpp
```

### Available Commands

```bash
# Help and information
docker run --rm velocitas-quick help

# Validation only (fast)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick validate

# Build application (default)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick build
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick  # same as build

# Build and run application with live output (smart rebuild)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i --network=host velocitas-quick run

# Run pre-built template (no input needed, fastest)
docker run --rm --network=host velocitas-quick rerun

# Granular build commands
docker run --rm velocitas-quick gen-model     # Generate vehicle model (Step 3)
docker run --rm velocitas-quick model         # Alias for gen-model
docker run --rm velocitas-quick compile       # Compile C++ app (Step 4)
docker run --rm velocitas-quick build-cpp     # Alias for compile  
docker run --rm velocitas-quick finalize      # Build summary (Step 5)
```

### Environment Variables

| Variable | Purpose | Example | Usage |
|----------|---------|---------|-------|
| `VSS_SPEC_URL` | Custom VSS specification URL | `https://company.com/vss.json` | Custom vehicle signals |
| `VSS_SPEC_FILE` | Custom VSS file path in container | `/vss.json` | Local VSS specification |
| `HTTP_PROXY` | HTTP proxy for corporate networks | `http://proxy:3128` | Corporate firewalls |
| `HTTPS_PROXY` | HTTPS proxy for corporate networks | `http://proxy:3128` | Corporate firewalls |
| `BUILD_TYPE` | Build configuration | `Debug`, `Release` | Development vs production |
| `CMAKE_FLAGS` | Additional CMake flags | `-DCUSTOM_FLAG=ON` | Custom build options |

---

## 🛠️ Traditional Development Environment (Optional)

For users who need comprehensive development features, debugging, or educational exploration, a full development environment is available.

### Setup Traditional Environment

```bash
# Build development container
docker build -f Dockerfile.dev -t velocitas-dev .

# Start runtime services
docker compose -f docker-compose.dev.yml up mosquitto vehicledatabroker -d

# Enter development environment  
docker run -it --privileged -v $(pwd):/workspace \
  --network=host velocitas-dev

# Inside container - full development cycle
gen-model      # Generate C++ classes from VSS
install-deps   # Install dependencies with Conan
build-app      # Build with CMake/Ninja
run-app        # Run the vehicle application
```

### Traditional Development Commands

```bash
# Code Generation & Build
gen-model         # Generate Vehicle.hpp from VSS
install-deps      # Install Velocitas SDK and dependencies  
build-app         # Compile C++ application
build-app -r      # Release build (optimized)

# Runtime & Testing
run-app           # Run the vehicle application
check-code        # Run linting and formatting
./build/bin/app_utests  # Execute unit tests

# Development Tools  
gen-grpc          # Generate gRPC code from proto
vdb-cli           # Vehicle Data Broker CLI
```

**Note:** The traditional environment requires more setup and is primarily for comprehensive development, debugging, and learning scenarios.

---

## 📖 Documentation & Resources

### Internal Documentation
- **[PREBUILT_IMAGES.md](PREBUILT_IMAGES.md)** - Guide to using pre-built Docker images
- **[BUILD_FLOW.md](BUILD_FLOW.md)** - Build file flow and architecture internals
- **Test Results** - Stored in `test_results/` after running tests

### External Resources
- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation
- **[Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)** - VSS reference  
- **[KUKSA.val](https://github.com/eclipse/kuksa.val)** - Vehicle Data Broker
- **[Eclipse Velocitas GitHub](https://github.com/eclipse-velocitas)** - Source code and examples

---

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Test your changes:
   ```bash
   # Test quick build utility
   ./test-mode2.sh --proxy
   
   # Test with instructive template
   cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i velocitas-quick
   ```
4. Commit changes: `git commit -m 'Add amazing feature'`
5. Push to branch: `git push origin feature/amazing-feature`
6. Open Pull Request

---

## 🐛 Troubleshooting

### Common Issues

**Build fails:**
```bash
# Check container logs
docker run --rm -i velocitas-quick validate < templates/app/src/VehicleApp.template.cpp

# Rebuild container
docker build --no-cache -f Dockerfile.quick -t velocitas-quick .
```

**Proxy connection issues:**
```bash
# Test proxy connectivity
docker run --rm -e HTTP_PROXY=http://127.0.0.1:3128 alpine wget -q --spider http://github.com

# Verify proxy settings
docker run --rm velocitas-quick env | grep -i proxy
```

**Permission problems:**
```bash
# Fix file permissions
docker run --rm -v $(pwd):/workspace --privileged velocitas-quick \
  bash -c "chown -R $(id -u):$(id -g) /workspace"
```

**VSS specification errors:**
```bash
# Verify VSS URL accessibility
curl -s $VSS_SPEC_URL | jq . > /dev/null && echo "VSS OK" || echo "VSS Failed"

# Use default VSS (fallback)
docker run --rm -i velocitas-quick < templates/app/src/VehicleApp.template.cpp  # Uses VSS 4.0 default
```

**Container size issues:**
```bash
# Clean Docker cache
docker system prune -f

# Remove old images
docker rmi $(docker images -f "dangling=true" -q)
```

---

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## 🏷️ Version Information

- **Template Version**: Quick Build v2.0
- **Velocitas SDK**: >= 0.7.0
- **Conan**: 2.15.1+
- **Base Image**: eclipse-velocitas/devcontainer-base-images/cpp:v0.4

---

## 🎯 Why This Approach?

### ✅ Advantages
- **Zero Setup Time** - Ready to use immediately  
- **Corporate Ready** - Full proxy and firewall support  
- **CI/CD Optimized** - Perfect for automated pipelines  
- **Flexible Input** - Multiple ways to provide code  
- **Custom VSS** - Adapt to any vehicle specification  
- **Reproducible** - Same environment, predictable results  
- **Lightweight** - Minimal resource usage  

### 🎪 Use Cases
- **Rapid Prototyping** - Quick iteration and testing
- **CI/CD Pipelines** - Automated building and testing
- **Corporate Environments** - Proxy and firewall compatibility
- **Educational** - No complex setup for learning
- **Production Builds** - Consistent, reproducible binaries
- **Custom VSS** - Company-specific vehicle specifications

---

**Happy Vehicle App Development! 🚗💨**

*Build your Velocitas C++ vehicle apps instantly with zero setup complexity.*