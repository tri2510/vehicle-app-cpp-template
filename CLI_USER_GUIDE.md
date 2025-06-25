# 🖥️ Velocitas CLI User Guide

Complete reference for building, running, and testing vehicle applications with the Velocitas CLI system.

---

## 📋 Table of Contents

- [🚀 Quick Start](#-quick-start)
- [💻 CLI Commands](#-cli-commands)
- [⚙️ Options & Flags](#️-options--flags)
- [📁 File Mounting](#-file-mounting)
- [🌍 Environment Variables](#-environment-variables)
- [🧪 Testing](#-testing)
- [🏢 Corporate Environment](#-corporate-environment)
- [🎯 Examples by Use Case](#-examples-by-use-case)
- [🐛 Troubleshooting](#-troubleshooting)
- [📚 Advanced Usage](#-advanced-usage)

---

## 🚀 Quick Start

### **Prerequisites**
```bash
# Ensure Docker is installed and running
docker --version

# Start KUKSA Vehicle Data Broker
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker

# Create persistent volumes for faster builds
docker volume create vehicle-build
docker volume create vehicle-deps  
docker volume create vehicle-vss
```

### **Basic Usage**
```bash
# Build and run the default template
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps

# Run the application
docker run -d --network host --name vehicle-app \
  -v vehicle-build:/quickbuild/build \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
```

---

## 💻 CLI Commands

The Velocitas CLI provides a unified interface for all vehicle app operations.

### **Command Structure**
```bash
docker run [docker-options] velocitas-quick <command> [options] [arguments]
```

### **Available Commands**

#### **`build`** - Build Vehicle Application
Compiles your C++ vehicle application from source code.

```bash
# Basic build
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build

# Verbose build with detailed output
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --verbose

# Fast build (skip dependencies if already installed)
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps
```

**What it does:**
1. ✅ Validates source code syntax
2. ✅ Installs C++ dependencies (Conan packages)
3. ✅ Generates Vehicle Signal Specification (VSS) models
4. ✅ Compiles C++ code using CMake & Ninja
5. ✅ Produces optimized executable (~14MB)

#### **`run [timeout]`** - Run Vehicle Application
Builds (if needed) and executes your vehicle application.

```bash
# Run with default 60-second timeout
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run

# Run for 5 minutes
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run 300

# Run indefinitely
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run 0
```

**What it does:**
1. ✅ Builds application if executable doesn't exist or source changed
2. ✅ Checks runtime environment (KUKSA databroker connectivity)
3. ✅ Starts vehicle application with timeout
4. ✅ Provides real-time logging output

#### **`test <type>`** - Run Test Suite
Executes various testing scenarios for your vehicle application.

```bash
# Test signal processing
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test signal-validation

# Test build process
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test build-validation

# Run all tests
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test full-suite
```

**Test Types:**
- **`signal-validation`**: Tests vehicle signal injection and processing
- **`build-validation`**: Validates build process and compilation
- **`full-suite`**: Comprehensive test suite including all tests

#### **`validate`** - Source Code Validation
Validates your C++ source code without building.

```bash
# Quick syntax validation
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick validate
```

**What it checks:**
- ✅ File accessibility and readability
- ✅ Basic C++ syntax validation
- ✅ File size and structure
- ✅ Required includes and patterns

#### **`clean`** - Clean Build Artifacts
Removes all build artifacts and forces fresh compilation.

```bash
# Clean build environment
docker run -v vehicle-build:/quickbuild/build velocitas-quick clean
```

#### **`help`** - Show Usage Information
Displays comprehensive help documentation.

```bash
# Show complete help
docker run velocitas-quick help
```

---

## ⚙️ Options & Flags

### **Global Options**
These options work with all commands:

| Option | Description | Example |
|--------|-------------|---------|
| `--verbose` | Show detailed command output and debug information | `build --verbose` |
| `--clean` | Clean workspace before operation | `build --clean` |
| `--skip-deps` | Skip dependency installation (use cached) | `build --skip-deps` |
| `--skip-vss` | Skip VSS model generation (use cached) | `build --skip-vss` |
| `--force` | Force rebuild even if unchanged | `build --force` |
| `--quiet` | Suppress non-essential output | `build --quiet` |

### **Build Optimization Strategies**

#### **Development Mode (Fast Iterations)**
```bash
# After first build, use this for 15-30 second builds
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps --skip-vss
```

#### **Production Mode (Full Build)**
```bash
# Complete rebuild with all dependencies
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --clean --force --verbose
```

#### **Debugging Mode (Maximum Verbosity)**
```bash
# Detailed logging for troubleshooting
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --verbose --clean
```

---

## 📁 File Mounting

The CLI supports multiple input methods for maximum flexibility.

### **Input Methods (Priority Order)**

#### **1. Mount Single File (Recommended)**
```bash
# Mount your C++ source file directly
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build
```

#### **2. Mount Source Directory**
```bash
# Mount entire source directory
docker run -v $(pwd)/src:/input velocitas-quick build
```

#### **3. Pipe from stdin**
```bash
# Pipe source code directly
cat MyApp.cpp | docker run -i velocitas-quick build
```

#### **4. Use Built-in Template**
```bash
# Use default template (no input required)
docker run velocitas-quick build
```

### **File Mapping Reference**

#### **Essential Mounts**
| Host Path | Container Path | Purpose | Required |
|-----------|----------------|---------|----------|
| `$(pwd)/MyApp.cpp` | `/app.cpp` | Vehicle application source | ✅ |
| Persistent volume | `/quickbuild/build` | Compiled executables | Recommended |
| Persistent volume | `/home/vscode/.conan2` | C++ dependencies cache | Recommended |
| Persistent volume | `/quickbuild/app/vehicle_model` | VSS models cache | Recommended |

#### **Configuration Mounts**
| Host Path | Container Path | Purpose | Required |
|-----------|----------------|---------|----------|
| `$(pwd)/conanfile.txt` | `/quickbuild/conanfile.txt` | Custom dependencies | Optional |
| `$(pwd)/custom-vss.json` | `/vss.json` | Custom VSS specification | Optional |
| `$(pwd)/AppManifest.json` | `/quickbuild/app/AppManifest.json` | App metadata | Optional |

### **Complete Mount Examples**

#### **Minimal Development Setup**
```bash
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

#### **Production Development Setup**
```bash
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps
```

#### **Custom Dependencies Setup**
```bash
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  -v $(pwd)/conanfile.txt:/quickbuild/conanfile.txt \
  velocitas-quick build
```

#### **Corporate Environment Setup**
```bash
docker run --rm --network host \
  -e HTTP_PROXY=http://proxy.company.com:8080 \
  -e HTTPS_PROXY=http://proxy.company.com:8080 \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  velocitas-quick build
```

---

## 🌍 Environment Variables

Configure the CLI behavior using environment variables.

### **Vehicle Data Broker Configuration**
```bash
# KUKSA databroker address (default: 127.0.0.1:55555)
-e SDV_VEHICLEDATABROKER_ADDRESS=192.168.1.100:55555

# MQTT broker address (default: 127.0.0.1:1883)
-e SDV_MQTT_ADDRESS=mqtt.company.com:1883
```

### **Vehicle Signal Specification (VSS)**
```bash
# Use custom VSS file
-e VSS_SPEC_FILE=/path/to/custom-vss.json

# Download VSS from URL
-e VSS_SPEC_URL=https://company.com/vss-spec.json

# Override VSS version (default: v4.0)
-e VSS_VERSION=v4.1
```

### **Build Configuration**
```bash
# Enable verbose output
-e VERBOSE_BUILD=1

# Custom Conan remote
-e CONAN_REMOTE_URL=https://conan.company.com

# Build type (default: Release)
-e CMAKE_BUILD_TYPE=Debug
```

### **Proxy Configuration**
```bash
# HTTP/HTTPS proxy for dependencies
-e HTTP_PROXY=http://proxy.company.com:8080
-e HTTPS_PROXY=http://proxy.company.com:8080
-e NO_PROXY=localhost,127.0.0.1,*.company.com
```

### **Complete Environment Example**
```bash
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=kuksa.company.com:55555 \
  -e SDV_MQTT_ADDRESS=mqtt.company.com:1883 \
  -e VSS_SPEC_URL=https://standards.company.com/vss.json \
  -e HTTP_PROXY=http://proxy.company.com:8080 \
  -e HTTPS_PROXY=http://proxy.company.com:8080 \
  -e VERBOSE_BUILD=1 \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build --verbose
```

---

## 🧪 Testing

The CLI provides comprehensive testing capabilities for vehicle applications.

### **Signal Validation Testing**
Tests vehicle signal processing and KUKSA databroker integration.

```bash
# Run signal validation tests
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test signal-validation
```

**What it tests:**
- ✅ Vehicle signal injection using KUKSA client
- ✅ Signal processing by vehicle application
- ✅ Response validation and logging
- ✅ Subscription functionality
- ✅ Error handling

**Test Sequence:**
1. Builds application if needed
2. Starts application in background
3. Injects test signals (speed, location, etc.)
4. Validates application responses
5. Checks for expected log outputs
6. Reports test results

### **Build Validation Testing**
Validates the build process and compilation environment.

```bash
# Run build validation tests
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test build-validation
```

**What it tests:**
- ✅ Source code compilation
- ✅ Dependency resolution
- ✅ VSS model generation
- ✅ Executable creation and permissions
- ✅ Build optimization flags
- ✅ Error handling and reporting

### **Full Test Suite**
Comprehensive testing including all test types.

```bash
# Run all tests
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test full-suite
```

### **Custom Testing**
You can also run manual tests alongside automated testing:

```bash
# Start application in background
docker run -d --network host --name test-app \
  -v vehicle-build:/quickbuild/build \
  velocitas-quick run 300

# Inject custom signals
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check application logs
docker logs test-app --tail 10

# Clean up
docker stop test-app && docker rm test-app
```

---

## 🏢 Corporate Environment

Special considerations for enterprise and corporate environments.

### **Proxy Configuration**
```bash
# Corporate proxy setup
docker run --rm --network host \
  -e HTTP_PROXY=http://proxy.corp.com:8080 \
  -e HTTPS_PROXY=http://proxy.corp.com:8080 \
  -e NO_PROXY=localhost,127.0.0.1,*.corp.com \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

### **Custom Certificates**
```bash
# Mount custom CA certificates
docker run --rm --network host \
  -v /etc/ssl/certs:/etc/ssl/certs:ro \
  -v $(pwd)/corp-ca.crt:/usr/local/share/ca-certificates/corp-ca.crt:ro \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

### **Private Dependency Repositories**
```bash
# Custom Conan remote
docker run --rm --network host \
  -e CONAN_REMOTE_URL=https://conan.corp.com \
  -e CONAN_LOGIN_USERNAME=your-username \
  -e CONAN_PASSWORD=your-token \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

### **Corporate VSS Specifications**
```bash
# Use company-specific VSS
docker run --rm --network host \
  -e VSS_SPEC_URL=https://standards.corp.com/vehicle-signals.json \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

### **Network Restrictions**
```bash
# Limited network access
docker run --rm \
  --add-host=conan.io:1.2.3.4 \
  --add-host=github.com:5.6.7.8 \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

---

## 🎯 Examples by Use Case

### **🎓 Learning & Tutorials**

#### **Step-by-Step SDV Tutorials**
```bash
# Step 1: Basic Speed Monitor
docker run --rm --network host \
  -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
  velocitas-quick build --skip-deps

# Step 2: Multi-Signal Processor
docker run --rm --network host \
  -v $(pwd)/examples/Step2_MultiSignalProcessor.cpp:/app.cpp \
  velocitas-quick build --skip-deps

# Step 3: Data Analysis & Alerts
docker run --rm --network host \
  -v $(pwd)/examples/Step3_DataAnalysisAlerts.cpp:/app.cpp \
  velocitas-quick build --skip-deps

# Step 4: Advanced Fleet Manager
docker run --rm --network host \
  -v $(pwd)/examples/Step4_AdvancedFleetManager.cpp:/app.cpp \
  velocitas-quick build --skip-deps
```

**See Also:** [Tutorial Guide](examples/TUTORIAL_GUIDE.md) for complete learning path.

### **🔬 Development & Prototyping**

#### **Rapid Prototyping**
```bash
# Fast iteration cycle (15-30 second builds)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/MyPrototype.cpp:/app.cpp \
  velocitas-quick build --skip-deps --skip-vss
```

#### **Development with Custom Dependencies**
```bash
# Add custom libraries to conanfile.txt:
# [requires]
# fmt/10.2.1
# nlohmann_json/3.11.3
# spdlog/1.13.0          # Added logging library
# boost/1.82.0           # Added Boost libraries
# vehicle-model/generated
# vehicle-app-sdk/0.7.0

docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  -v $(pwd)/conanfile.txt:/quickbuild/conanfile.txt \
  velocitas-quick build --clean
```

### **🏭 Production Development**

#### **CI/CD Pipeline Integration**
```bash
# Dockerfile for CI/CD
FROM ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest

# Copy source code
COPY MyApp.cpp /app.cpp
COPY conanfile.txt /quickbuild/conanfile.txt

# Build application
RUN /scripts/velocitas-cli.sh build --clean --verbose

# Runtime configuration
ENV SDV_VEHICLEDATABROKER_ADDRESS=production-kuksa:55555
CMD ["/scripts/velocitas-cli.sh", "run", "0"]
```

#### **Production Deployment**
```bash
# Production-ready container
docker run -d --name vehicle-app-prod \
  --restart=unless-stopped \
  --network=vehicle-network \
  -e SDV_VEHICLEDATABROKER_ADDRESS=kuksa-prod:55555 \
  -e SDV_MQTT_ADDRESS=mqtt-prod:1883 \
  -v vehicle-app-logs:/var/log \
  -v $(pwd)/ProductionApp.cpp:/app.cpp \
  velocitas-quick run 0
```

### **🧪 Testing & Quality Assurance**

#### **Automated Testing Pipeline**
```bash
#!/bin/bash
# test-pipeline.sh

echo "🧪 Starting Vehicle App Test Pipeline"

# Build validation
docker run --rm -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test build-validation

# Signal processing tests
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test signal-validation

# Integration tests
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick test full-suite

echo "✅ Test Pipeline Complete"
```

#### **Performance Testing**
```bash
# Build with debug information
docker run --rm --network host \
  -e CMAKE_BUILD_TYPE=Debug \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build --clean

# Memory leak testing with Valgrind
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  --cap-add=SYS_PTRACE \
  velocitas-quick run 60 --memory-check
```

---

## 🐛 Troubleshooting

### **Common Issues & Solutions**

#### **Build Failures**

**Problem: Compilation Errors**
```bash
❌ [ERROR] Build compilation failed
```
**Solution:**
```bash
# Check build log for details
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --verbose

# Common fixes:
# 1. Check C++ syntax errors
# 2. Verify #include directives
# 3. Ensure VSS signal names are correct
# 4. Check for missing dependencies in conanfile.txt
```

**Problem: Dependency Resolution Failed**
```bash
❌ [ERROR] Conan dependency resolution failed
```
**Solution:**
```bash
# Clean dependency cache and rebuild
docker volume rm vehicle-deps
docker volume create vehicle-deps
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --clean
```

**Problem: VSS Model Generation Failed**
```bash
❌ [ERROR] Vehicle model generation failed
```
**Solution:**
```bash
# Force VSS regeneration
docker volume rm vehicle-vss
docker volume create vehicle-vss
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --force
```

#### **Runtime Issues**

**Problem: Cannot Connect to KUKSA Databroker**
```bash
❌ [ERROR] Failed to connect to databroker at 127.0.0.1:55555
```
**Solution:**
```bash
# 1. Ensure databroker is running
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker

# 2. Check databroker logs
docker logs $(docker-compose -f docker-compose.dev.yml ps -q vehicledatabroker)

# 3. Verify network connectivity
docker run --rm --network host busybox nc -zv 127.0.0.1 55555

# 4. Use custom databroker address
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=your-host:55555 \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick run
```

**Problem: Application Crashes on Startup**
```bash
❌ [ERROR] Application exited with code 1
```
**Solution:**
```bash
# Run with verbose logging
docker run --rm --network host \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick run --verbose

# Check application logs
docker logs vehicle-app --tail 50

# Validate source code
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick validate
```

#### **Performance Issues**

**Problem: Slow Build Times**
```bash
# Builds taking >2 minutes consistently
```
**Solution:**
```bash
# Use persistent volumes for caching
docker volume create vehicle-build
docker volume create vehicle-deps
docker volume create vehicle-vss

# Use --skip-deps for subsequent builds
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps

# Use pre-built image for instant setup
docker run -v $(pwd)/MyApp.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build
```

**Problem: High Memory Usage**
```bash
# Docker consuming >4GB RAM
```
**Solution:**
```bash
# Clean unused Docker resources
docker system prune -f
docker volume prune -f

# Use --quiet flag to reduce memory usage
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --quiet
```

### **Debugging Commands**

#### **Inspect Build Environment**
```bash
# Check environment variables
docker run --rm velocitas-quick env

# Inspect mounted files
docker run --rm -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick ls -la /

# Check available space
docker run --rm -v vehicle-build:/quickbuild/build velocitas-quick df -h
```

#### **Network Connectivity**
```bash
# Test KUKSA connection
docker run --rm --network host busybox nc -zv 127.0.0.1 55555

# Test MQTT connection  
docker run --rm --network host busybox nc -zv 127.0.0.1 1883

# Check DNS resolution
docker run --rm --network host busybox nslookup github.com
```

#### **Application Debugging**
```bash
# Run application with debugger
docker run --rm --network host \
  --cap-add=SYS_PTRACE \
  -v vehicle-build:/quickbuild/build \
  velocitas-quick gdb /quickbuild/build/bin/app

# Memory analysis
docker run --rm --network host \
  --cap-add=SYS_PTRACE \
  -v vehicle-build:/quickbuild/build \
  velocitas-quick valgrind --leak-check=full /quickbuild/build/bin/app
```

---

## 📚 Advanced Usage

### **Custom VSS Specifications**

#### **Using Company-Specific Signals**
```bash
# Create custom VSS specification
cat > custom-vss.json << 'EOF'
{
  "Vehicle": {
    "type": "branch",
    "description": "Vehicle root node",
    "children": {
      "Speed": {
        "type": "sensor",
        "datatype": "float",
        "unit": "m/s",
        "description": "Vehicle speed"
      },
      "Custom": {
        "type": "branch",
        "description": "Company-specific signals",
        "children": {
          "FleetId": {
            "type": "attribute",
            "datatype": "string",
            "description": "Fleet identifier"
          },
          "MaintenanceStatus": {
            "type": "sensor", 
            "datatype": "uint8",
            "description": "Maintenance status code"
          }
        }
      }
    }
  }
}
EOF

# Build with custom VSS
docker run --rm --network host \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build
```

### **Multi-Application Development**

#### **Building Multiple Applications**
```bash
#!/bin/bash
# build-fleet.sh - Build multiple vehicle applications

apps=("SpeedMonitor" "FleetManager" "DiagnosticTool" "NavigationAssist")

for app in "${apps[@]}"; do
  echo "🏗️ Building $app..."
  docker run --rm --network host \
    -v vehicle-build-$app:/quickbuild/build \
    -v vehicle-deps:/home/vscode/.conan2 \
    -v vehicle-vss:/quickbuild/app/vehicle_model \
    -v $(pwd)/apps/${app}.cpp:/app.cpp \
    velocitas-quick build --skip-deps
  
  echo "✅ $app build complete"
done
```

### **Performance Optimization**

#### **Build Optimization Techniques**
```bash
# 1. Use ccache for faster compilation
docker run --rm --network host \
  -v ccache-cache:/root/.ccache \
  -e CCACHE_DIR=/root/.ccache \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build

# 2. Parallel builds
docker run --rm --network host \
  -e MAKEFLAGS="-j$(nproc)" \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build

# 3. Optimized persistent volumes
docker volume create vehicle-build --driver local \
  --opt type=tmpfs --opt device=tmpfs --opt o=size=1g
```

#### **Runtime Optimization**
```bash
# Production build with optimizations
docker run --rm --network host \
  -e CMAKE_BUILD_TYPE=Release \
  -e CMAKE_CXX_FLAGS="-O3 -march=native" \
  -v $(pwd)/MyApp.cpp:/app.cpp \
  velocitas-quick build --clean
```

### **Integration Patterns**

#### **Kubernetes Deployment**
```yaml
# vehicle-app-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: vehicle-app
spec:
  replicas: 1
  selector:
    matchLabels:
      app: vehicle-app
  template:
    metadata:
      labels:
        app: vehicle-app
    spec:
      containers:
      - name: vehicle-app
        image: velocitas-quick:latest
        command: ["/scripts/velocitas-cli.sh", "run", "0"]
        env:
        - name: SDV_VEHICLEDATABROKER_ADDRESS
          value: "kuksa-databroker:55555"
        volumeMounts:
        - name: app-source
          mountPath: /app.cpp
          subPath: VehicleApp.cpp
      volumes:
      - name: app-source
        configMap:
          name: vehicle-app-source
```

#### **Docker Compose Stack**
```yaml
# docker-compose.vehicle-stack.yml
version: '3.8'
services:
  kuksa-databroker:
    image: ghcr.io/eclipse-kuksa/kuksa-databroker:main
    ports:
      - "55555:55555"
    networks:
      - vehicle-network
  
  vehicle-app:
    image: velocitas-quick:latest
    command: ["/scripts/velocitas-cli.sh", "run", "0"]
    depends_on:
      - kuksa-databroker
    environment:
      - SDV_VEHICLEDATABROKER_ADDRESS=kuksa-databroker:55555
    volumes:
      - ./MyApp.cpp:/app.cpp
    networks:
      - vehicle-network
  
  mqtt-broker:
    image: eclipse-mosquitto:latest
    ports:
      - "1883:1883"
    networks:
      - vehicle-network

networks:
  vehicle-network:
    driver: bridge
```

---

## 📖 Additional Resources

### **Documentation Links**
- **[Tutorial Guide](examples/TUTORIAL_GUIDE.md)** - Step-by-step SDV learning path
- **[Test Instructions](TEST_INSTRUCTIONS.md)** - Comprehensive testing guide
- **[CLAUDE.md](CLAUDE.md)** - Detailed CLI workflow guide
- **[Eclipse Velocitas Docs](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation

### **Example Applications**
- **[examples/Step1_BasicSpeedMonitor.cpp](examples/Step1_BasicSpeedMonitor.cpp)** - Basic speed monitoring
- **[examples/Step2_MultiSignalProcessor.cpp](examples/Step2_MultiSignalProcessor.cpp)** - Multi-signal processing
- **[examples/Step3_DataAnalysisAlerts.cpp](examples/Step3_DataAnalysisAlerts.cpp)** - Advanced analytics
- **[examples/Step4_AdvancedFleetManager.cpp](examples/Step4_AdvancedFleetManager.cpp)** - Fleet management

### **Community & Support**
- **[GitHub Issues](https://github.com/eclipse-velocitas/vehicle-app-cpp-template/issues)** - Bug reports and feature requests
- **[Eclipse Velocitas Community](https://github.com/eclipse-velocitas)** - Project community
- **[KUKSA Documentation](https://eclipse-kuksa.github.io/)** - Vehicle Data Broker docs

---

## 🏁 Quick Reference

### **Essential Commands**
```bash
# Build application
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build

# Run application  
docker run -d --name vehicle-app -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run

# Test application
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test signal-validation

# Clean build
docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --clean

# Get help
docker run velocitas-quick help
```

### **Performance Tips**
- ✅ Use persistent volumes for 3-4x faster builds
- ✅ Use `--skip-deps` after first build
- ✅ Use pre-built image for instant setup
- ✅ Clean volumes periodically to free space

### **Troubleshooting Checklist**
- ☑️ KUKSA databroker running on 127.0.0.1:55555?
- ☑️ Source file mounted correctly?
- ☑️ Network connectivity working?
- ☑️ Docker daemon running with sufficient resources?
- ☑️ Persistent volumes created and accessible?

---

*🚗💨 Happy Vehicle App Development with Velocitas CLI!*