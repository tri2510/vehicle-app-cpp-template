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

**Step 3: Create Persistent Volume**
```bash
# Clean any existing volume and create fresh persistent volume
docker volume rm vehicle-build 2>/dev/null || true
docker volume create vehicle-build
```

**Step 4: Build Vehicle App**
```bash
# Build with persistent volume (15-30 seconds)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps --skip-vss --verbose
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

**Step 2: Create Persistent Volume**
```bash
# Clean any existing volume and create fresh persistent volume
docker volume rm vehicle-build 2>/dev/null || true
docker volume create vehicle-build
```

**Step 3: Build Vehicle App**
```bash
# Build with pre-built image (60-90 seconds)
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --skip-vss --verbose
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

## 🛠️ **Advanced Usage**

### Build Options
```bash
# Verbose build output
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --verbose

# Clean rebuild  
docker run --rm --network host \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --clean --force

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
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build
```

### Custom Vehicle Signal Specification
```bash
# Use custom VSS URL
docker run --rm --network host \
  -e VSS_SPEC_URL=https://company.com/vss.json \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build

# Use custom VSS file
docker run --rm --network host \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  -v vehicle-build:/quickbuild/build \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build
```

## 📖 Documentation

- **[TEST_INSTRUCTIONS.md](TEST_INSTRUCTIONS.md)** - Detailed interactive testing guide
- **[examples/](examples/)** - Real-world SDV applications  
- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation

## 🚀 Performance

| Metric | Self-Build | Pre-built Image |
|--------|------------|-----------------|
| **Setup Time** | 3-5 minutes | 0 seconds |
| **Build Time** | 15-30 seconds | 60-90 seconds |
| **Executable Size** | ~14MB optimized | ~14MB optimized |
| **Memory Usage** | ~500MB runtime | ~500MB runtime |
| **Disk Space** | ~2.5GB (local) | ~2.5GB (cached) |

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

# Remove persistent volume
docker volume rm vehicle-build
```

---
*🚗💨 Build Velocitas C++ vehicle apps with zero setup!*