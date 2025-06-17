# 🐳 Pre-built Docker Images Guide

**Build and test vehicle applications instantly without any local setup!**

This guide shows you how to use pre-built Docker images to:
- ⚡ **Build** C++ vehicle apps in 60-90 seconds (vs 5+ minutes locally)
- 🏃 **Run** applications with live vehicle signal testing
- 🔧 **Test** with real Vehicle Data Broker and KUKSA client
- 🏢 **Support** corporate proxies and custom VSS specifications

No dependencies, no complex setup - just Docker and your C++ code!

## 📦 Available Images

CI/CD pipeline automatically builds and publishes Docker images to GitHub Container Registry (GHCR):

| Image | Purpose | Size | Platforms |
|-------|---------|------|-----------|
| `ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest` | **Quick Build Utility** (Recommended) | ~2.5GB | linux/amd64, linux/arm64 |

## 🧪 Complete Test Workflow - Get Started Instantly!

Experience the full vehicle app development cycle in minutes:

```bash
# 1. Get the template
git clone https://github.com/tri2510/vehicle-app-cpp-template.git
cd vehicle-app-cpp-template

# 2. Start Vehicle Data Broker
docker compose -f docker-compose.dev.yml up vehicledatabroker -d

# 3. Build and run your app with live output
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest run

# 4. Test with vehicle signals (in another terminal)
docker run -it --rm --network=host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main

# Inside kuksa-client shell, test different speeds:
setValue Vehicle.Speed 30.0    # City speed
setValue Vehicle.Speed 65.0    # Highway speed  
setValue Vehicle.Speed 85.0    # Over limit
setValue Vehicle.Speed 0.0     # Stopped
getValue Vehicle.Speed
subscribe Vehicle.Speed
quit

# 5. Stop services when done
docker compose -f docker-compose.dev.yml down
```

**🎉 You've just built and tested a complete vehicle application!**

---

## 🚀 Quick Commands Reference

### Build Your App
```bash
# Build only
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Rebuild (force new build)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest build
```

### Run Your App
```bash
# Build and run with live output
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest run

# Rerun (same app, no rebuild if executable exists)
docker run --rm -i \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest run
```

## 🔧 Advanced Usage

### With Corporate Proxy
```bash
# Build with proxy
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Run with proxy
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -e HTTP_PROXY=http://127.0.0.1:3128 \
  -e HTTPS_PROXY=http://127.0.0.1:3128 \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest run
```

### Custom VSS Specification
```bash
# With custom VSS URL
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -e VSS_SPEC_URL=https://company.com/vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# With custom VSS file
docker run --rm -i \
  -v $(pwd)/custom-vss.json:/vss.json \
  -e VSS_SPEC_FILE=/vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest < templates/app/src/VehicleApp.template.cpp
```

### Different Input Methods
```bash
# Method 1: Pipe from file (recommended)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Method 2: Mount single file
docker run --rm \
  -v $(pwd)/templates/app/src/VehicleApp.template.cpp:/input \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Method 3: Mount directory
docker run --rm \
  -v $(pwd)/src:/input \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Method 4: Validation only
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest validate
```

### Persistent Builds (Faster Reruns)
```bash
# Create persistent volume
docker volume create my-velocitas-build

# Build with persistent storage
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -v my-velocitas-build:/quickbuild/build \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Rerun without rebuild (uses cached build)
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  -v my-velocitas-build:/quickbuild/build \
  --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest run

# Clean up
docker volume rm my-velocitas-build
```


## 🏷️ Image Versioning

### Available Tags
| Tag | Description | When to Use |
|-----|-------------|-------------|
| `latest` | Latest stable release | **Production** |
| `develop` | Latest development build | Testing new features |
| `v1.0.0` | Specific version | Reproducible builds |

### Version Examples
```bash
# Latest stable (recommended)
docker run --rm -i ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Specific version
docker run --rm -i ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:v1.0.0

# Development version
docker run --rm -i ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:develop
```

## 📊 Performance Tips

### Optimize Build Speed
```bash
# Use persistent volumes for faster rebuilds
docker volume create velocitas-cache
docker run --rm -i -v velocitas-cache:/quickbuild/build velocitas-quick

# Use specific image versions for consistency
docker run --rm -i ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:v1.0.0
```

### Service Endpoints
| Service | Endpoint | Purpose |
|---------|----------|---------|
| **Vehicle Data Broker** | `127.0.0.1:55555` | Vehicle signal access |
| **KUKSA Client** | `--network=host` | Manual signal testing |

## 🐛 Troubleshooting

### Common Issues
```bash
# Image not found
docker pull ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Permission denied (private repo)
echo $GITHUB_TOKEN | docker login ghcr.io -u $GITHUB_USERNAME --password-stdin

# Build failures - check validation first
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest validate

# Network issues
docker run --rm ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest help
```

## 📚 Related Documentation

- **[README.md](README.md)** - Main project documentation  
- **[DEVELOPER_WORKFLOW.md](DEVELOPER_WORKFLOW.md)** - Complete development workflows
- **[GitHub Releases](https://github.com/tri2510/vehicle-app-cpp-template/releases)** - Version history and release notes

---

**🎯 Ready to build?** Start with the Quick Commands section above and get your vehicle app running in seconds!

*Pre-built images save you 3-5 minutes on every build and ensure consistent, tested environments.*