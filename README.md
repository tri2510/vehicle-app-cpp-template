# Velocitas C++ Vehicle App Template

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

🚀 **Zero-setup Vehicle App development** - Build [Eclipse Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) C++ vehicle applications with Docker in minutes.

## 🎯 Quick Start Options

Choose your preferred approach:

### 🏗️ **Option A: Self-Build (3-5 minutes setup)**

**Step 1: Build Container**
```bash
# Build the container (3-5 minutes, no proxy needed)
docker build -f Dockerfile.quick -t velocitas-quick .
```

**Step 2: Start Services**
```bash
# Start KUKSA databroker with docker-compose
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker
```

**Step 3: Create Persistent Volumes**
```bash
# Clean any existing volumes and create fresh persistent volumes
docker volume rm vehicle-build vehicle-deps vehicle-vss 2>/dev/null || true
docker volume create vehicle-build      # For executables
docker volume create vehicle-deps       # For Conan dependencies
docker volume create vehicle-vss        # For VSS models
```

**Step 4: Build Vehicle App**
```bash
# Build with persistent volumes (60-90s first time, 15-30s subsequent)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --verbose
```

**Step 5: Run Vehicle App**
```bash
# Run vehicle application using persistent volume
docker run -d --network host --name vehicle-app \
  -v vehicle-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  velocitas-quick run 60
```

### ⚡ **Option B: Pre-built Image (instant start)**

**Step 1: Start Services**
```bash
# Start KUKSA databroker
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker
```

**Step 2: Create Persistent Volumes**
```bash
# Clean any existing volumes and create fresh persistent volumes
docker volume rm vehicle-build vehicle-deps vehicle-vss 2>/dev/null || true
docker volume create vehicle-build      # For executables
docker volume create vehicle-deps       # For Conan dependencies
docker volume create vehicle-vss        # For VSS models
```

**Step 3: Build Vehicle App**
```bash
# Build with pre-built image and persistent volumes (60-90s first time, 15-30s subsequent)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --verbose
```

**Step 4: Run Vehicle App**
```bash
# Run vehicle application using persistent volume
docker run -d --network host --name vehicle-app \
  -v vehicle-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
```

## 🧪 **Test Signal Processing**
```bash
# Inject speed signal
echo "setValue Vehicle.Speed 65.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Check response
docker logs vehicle-app --tail 10
```

**🎉 Expected Result:**
```
INFO: 🚗 Vehicle Speed: 234.0 km/h (65.00 m/s)
WARN: ⚠️ HIGH SPEED WARNING: 234.0 km/h
```

## 📁 **File Mapping & Mount System**

The build system supports mounting external files into the container to customize your vehicle application. Here's the complete mapping:

### **Source Code Mapping**
| Host File | Container Path | Purpose | Required |
|-----------|----------------|---------|----------|
| `$(pwd)/templates/app/src/VehicleApp.cpp` | `/app.cpp` | Your vehicle application source code | ✅ Yes |
| `$(pwd)/examples/FleetSpeedMonitor.cpp` | `/app.cpp` | Alternative example application | Optional |
| `$(pwd)/custom/MyApp.cpp` | `/app.cpp` | Your custom application | Optional |

### **Configuration Mapping**
| Host File | Container Path | Purpose | Required |
|-----------|----------------|---------|----------|
| `$(pwd)/templates/conanfile.txt` | `/quickbuild/conanfile.txt` | C++ dependencies configuration | Optional |
| `$(pwd)/custom-vss.json` | `/vss.json` | Custom vehicle signal specification | Optional |
| `$(pwd)/custom/AppManifest.json` | `/quickbuild/app/AppManifest.json` | Application metadata | Optional |

### **Persistent Storage Mapping**
| Docker Volume | Container Path | Purpose | Benefit |
|---------------|----------------|---------|---------|
| `vehicle-build` | `/quickbuild/build` | Compiled executables & build artifacts | Reuse executables between runs |
| `vehicle-deps` | `/home/vscode/.conan2` | Conan C++ dependencies cache | Skip dependency downloads |
| `vehicle-vss` | `/quickbuild/app/vehicle_model` | Generated VSS models | Skip VSS generation |

### **Complete Mount Example**
```bash
# Full example with all possible mounts
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/templates/conanfile.txt:/quickbuild/conanfile.txt \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e VSS_SPEC_FILE=/vss.json \
  velocitas-quick build --verbose
```

### **Common Use Cases**

**1. Basic Development (Minimal Mounts):**
```bash
docker run --rm --network host \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build
```

**2. Production Development (With Caching):**
```bash
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps
```

**3. Custom Dependencies:**
```bash
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/templates/conanfile.txt:/quickbuild/conanfile.txt \
  velocitas-quick build
```

**4. Different Application:**
```bash
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/examples/FleetSpeedMonitor.cpp:/app.cpp \
  velocitas-quick build --skip-deps
```

## 🛠️ **Advanced Usage**

### Build Options
```bash
# Verbose build output
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --verbose

# Clean rebuild (forces fresh dependency and VSS installation)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --clean --force

# Fast build (skip deps/VSS, use only if already installed)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps --skip-vss --verbose

# Validate source only
docker run --rm -i \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick validate
```

### Corporate Proxy Support
```bash
# Build with proxy
docker run --rm --network host \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build
```

### Custom Vehicle Signal Specification
```bash
# Use custom VSS URL
docker run --rm --network host \
  -e VSS_SPEC_URL=https://company.com/vss.json \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build

# Use custom VSS file
docker run --rm --network host \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build
```

### Custom Dependencies (conanfile.txt)
```bash
# When modifying templates/conanfile.txt, mount it to use custom dependencies
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/templates/conanfile.txt:/quickbuild/conanfile.txt \
  velocitas-quick build --verbose

# Example: Adding spdlog logging library to conanfile.txt
# [requires]
# fmt/10.2.1
# nlohmann_json/3.11.3
# spdlog/1.13.0
# vehicle-model/generated
# vehicle-app-sdk/0.7.0

# Clean rebuild after dependency changes (recommended)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v vehicle-deps:/home/vscode/.conan2 \
  -v vehicle-vss:/quickbuild/app/vehicle_model \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/templates/conanfile.txt:/quickbuild/conanfile.txt \
  velocitas-quick build --clean --force
```

**⚠️ Dependency Version Conflicts:**
If you encounter version conflicts (e.g., `ERROR: Version conflict: Conflict between fmt/10.2.1 and fmt/11.1.1`), check dependency compatibility:
- Use `conan graph info` to inspect dependency trees
- Adjust versions in conanfile.txt to resolve conflicts
- Some libraries may require specific versions of their dependencies

## 📖 Documentation

- **[CLI_USER_GUIDE.md](CLI_USER_GUIDE.md)** - Complete CLI reference & advanced usage guide
- **[TEST_INSTRUCTIONS.md](TEST_INSTRUCTIONS.md)** - Detailed interactive testing guide
- **[examples/](examples/)** - Real-world SDV applications  
- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation

## 🚀 Performance

| Metric | Self-Build | Pre-built Image |
|--------|------------|-----------------|
| **Setup Time** | 3-5 minutes | 0 seconds |
| **First Build** | 60-90 seconds | 60-90 seconds |
| **Subsequent Builds** | 15-30 seconds | 15-30 seconds |
| **Executable Size** | ~14MB optimized | ~14MB optimized |
| **Memory Usage** | ~500MB runtime | ~500MB runtime |
| **Disk Space** | ~2.5GB + volumes | ~2.5GB + volumes |

## 🛠️ Features
✅ Zero dependencies ✅ Docker-compose ready ✅ Signal validation ✅ VSS 4.0 support ✅ Pre-built images ✅ Corporate proxy support

## 🏆 Success Criteria
When working correctly, you'll see:
```
INFO: 🚗 Vehicle Speed: 234.0 km/h (65.00 m/s)  
WARN: ⚠️ HIGH SPEED WARNING: 234.0 km/h
```

## Cleanup
```bash
# Stop services
docker-compose -f docker-compose.dev.yml down
docker stop vehicle-app && docker rm vehicle-app

# Remove persistent volumes
docker volume rm vehicle-build vehicle-deps vehicle-vss
```

---
*🚗💨 Build Velocitas C++ vehicle apps with zero setup!*