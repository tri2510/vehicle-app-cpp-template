# Velocitas C++ Vehicle App Template

[![License: Apache](https://img.shields.io/badge/License-Apache-yellow.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Docker](https://img.shields.io/badge/Docker-Enabled-blue.svg)](https://docker.com)
[![Velocitas](https://img.shields.io/badge/Velocitas-C++-green.svg)](https://github.com/eclipse-velocitas/velocitas-docs)

🚀 **Zero-setup Vehicle App development** - Build [Eclipse Velocitas](https://github.com/eclipse-velocitas/velocitas-docs) C++ vehicle applications with Docker in minutes.

## 🎯 Quick Start: 5 Steps to Working Vehicle App

### **Step 1: Build Container**
```bash
# Build the container (3-5 minutes, no proxy needed)
docker build -f Dockerfile.quick -t velocitas-quick .
```

### **Step 2: Start Services**
```bash
# Start KUKSA databroker with docker-compose
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker
```

### **Step 3: Build Vehicle App**
```bash
# Fast build with optimization (15-30 seconds)
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick build --skip-deps --skip-vss --verbose
```

### **Step 4: Run Vehicle App**
```bash
# Run vehicle application  
docker run -d --network host --name vehicle-app \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick run 60
```

### **Step 5: Test Signal Processing**
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

## 📖 Documentation

- **[TEST_INSTRUCTIONS.md](TEST_INSTRUCTIONS.md)** - Detailed interactive testing guide
- **[PREBUILT_IMAGES.md](PREBUILT_IMAGES.md)** - Pre-built Docker images guide  
- **[examples/](examples/)** - Real-world SDV applications
- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation

## 🚀 Performance
- **Build Time:** 3-5 minutes (container) + 15-30 seconds (app)
- **Executable Size:** ~14MB optimized
- **Memory:** ~500MB runtime

## 🛠️ Features
✅ Zero dependencies ✅ Docker-compose ready ✅ Signal validation ✅ VSS 4.0 support

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
```

---
*🚗💨 Build Velocitas C++ vehicle apps with zero setup!*