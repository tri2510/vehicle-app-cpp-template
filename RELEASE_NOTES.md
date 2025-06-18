# Release Notes

## Version 1.0.0 (2025-06-18)

🎉 **Initial Release - Complete Vehicle App C++ Template with Quick Build System**

### 🚀 **New Features**

#### **Quick Build System**
- **Zero-setup containerized building** - Build C++ vehicle apps without local dependencies
- **Multi-input support** - stdin, file mount, or directory mount input methods
- **Proxy-ready** - Full corporate network and firewall support
- **Custom VSS support** - Use default VSS 4.0 or custom vehicle signal specifications
- **Pre-built Docker images** - Available on GitHub Container Registry (GHCR)

#### **Granular Build Commands**
- `gen-model` / `model` - Generate vehicle signal model from VSS (Step 3)
- `compile` / `build-cpp` - Compile C++ application only (Step 4)  
- `finalize` - Build summary and finalization (Step 5)
- `run` - Build and run application with live output (smart rebuild detection)
- `rerun` - Run pre-built template instantly (no input needed)

#### **Advanced Build Features**
- **Verbose mode** - `VERBOSE_BUILD=1` shows detailed command output
- **Smart rebuild detection** - Only rebuilds when input changes
- **Comprehensive logging** - Unified logging system with success/warning messages
- **Build optimization** - Pre-installed dependencies and cached components
- **Multi-platform support** - linux/amd64 and linux/arm64 architectures

#### **Testing & Validation**
- **Comprehensive test suite** - 17 test cases covering all functionality
- **Automated testing** - `test-mode2.sh` with proxy and verbose mode support
- **CI/CD ready** - GitHub Actions workflows for automated builds and releases
- **Input validation** - VehicleApp.cpp code validation before build

### 📦 **Available Images**

| Image | Purpose | Size | Platforms |
|-------|---------|------|-----------|
| `ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest` | Quick Build Utility | ~2.5GB | linux/amd64, linux/arm64 |

### 🔧 **Usage Examples**

#### **Basic Usage**
```bash
# Build and run instantly
cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest

# Run pre-built template (fastest)
docker run --rm --network=host \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest rerun
```

#### **Corporate Environment**
```bash
# With proxy support
cat VehicleApp.cpp | docker run --rm -i \
  -e HTTP_PROXY=http://proxy:3128 \
  -e HTTPS_PROXY=http://proxy:3128 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest
```

#### **Custom VSS**
```bash
# Custom VSS specification
cat VehicleApp.cpp | docker run --rm -i \
  -e VSS_SPEC_URL=https://company.com/vss.json \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest
```

#### **Granular Commands**
```bash
# Individual build steps
docker run --rm velocitas-quick gen-model    # Generate model
docker run --rm velocitas-quick compile      # Compile only
docker run --rm velocitas-quick finalize     # Finalize build
```

#### **Verbose Mode**
```bash
# See detailed output
cat VehicleApp.cpp | docker run --rm -i \
  -e VERBOSE_BUILD=1 \
  velocitas-quick
```

### 🏗️ **Technical Specifications**

#### **Performance Metrics**
- **Container Build Time**: 3-5 minutes (one-time setup)
- **App Build Time**: 60-90 seconds (cached dependencies)
- **Memory Usage**: ~2GB during build, ~500MB runtime  
- **Executable Size**: ~13-15MB optimized binary
- **Network**: Proxy-friendly with minimal external dependencies

#### **Dependencies**
- **Base Image**: eclipse-velocitas/devcontainer-base-images/cpp:v0.4
- **Velocitas SDK**: >= 0.7.0
- **Conan**: 2.15.1+
- **Build System**: CMake + Ninja with ccache optimization

#### **Supported Platforms**
- **Architecture**: x86_64 (AMD64), ARM64
- **OS**: Linux (Ubuntu-based)
- **Network**: IPv4/IPv6, proxy support
- **Services**: MQTT (port 1883), Vehicle Data Broker (port 55555)

### 📋 **Environment Variables**

| Variable | Purpose | Example |
|----------|---------|---------|
| `VSS_SPEC_URL` | Custom VSS specification URL | `https://company.com/vss.json` |
| `VSS_SPEC_FILE` | Custom VSS file path | `/vss.json` |
| `HTTP_PROXY` | HTTP proxy for corporate networks | `http://proxy:3128` |
| `HTTPS_PROXY` | HTTPS proxy for corporate networks | `http://proxy:3128` |
| `VERBOSE_BUILD` | Show detailed command output | `1` |

### 🧪 **Testing**

#### **Test Coverage**
- **17 comprehensive test cases** covering all functionality
- **Basic build functionality** (8 tests)
- **Granular commands** (5 tests)
- **Run/rerun commands** (2 tests)  
- **Sequential workflow** (1 test)
- **Verbose mode** (1 test)

#### **Test Execution**
```bash
# Run all tests
./test-mode2.sh

# Test with proxy
./test-mode2.sh --proxy

# Test with verbose mode  
./test-mode2.sh --verbose

# Full testing
./test-mode2.sh --proxy --verbose
```

### 📚 **Documentation**

#### **Core Documentation**
- **[README.md](README.md)** - Complete usage guide and examples
- **[PREBUILT_IMAGES.md](PREBUILT_IMAGES.md)** - Pre-built Docker images guide  
- **[BUILD_FLOW.md](BUILD_FLOW.md)** - Build file flow and architecture internals

#### **Key Features Documented**
- Input processing methods (stdin, file mount, directory mount)
- Corporate proxy configuration and troubleshooting
- Custom VSS specification usage
- Granular build command workflows
- CI/CD integration examples
- Performance optimization tips

### 🔒 **Security**

- **No secrets exposure** - Safe for CI/CD pipelines
- **Proxy support** - Corporate firewall compatibility
- **Container isolation** - Sandboxed build environment
- **Minimal attack surface** - Only required dependencies included

### 🐛 **Known Issues**

- **Dependency warnings** - Non-critical warnings may appear with proxy configurations (builds continue normally)
- **Service availability** - MQTT/VDB services must be running separately for full functionality (apps fall back to simulation mode)

### 📈 **Performance Benefits**

- **3-5x faster builds** compared to local development setup
- **Zero local dependencies** - No SDK installation required
- **Consistent environment** - Same results across all platforms
- **Cached components** - Pre-installed dependencies and generated models
- **Optimized binaries** - Release mode compilation with size optimization

### 🚀 **Getting Started**

1. **Try the template instantly:**
   ```bash
   docker run --rm --network=host \
     ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest rerun
   ```

2. **Build your own app:**
   ```bash
   cat YourVehicleApp.cpp | docker run --rm -i \
     ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest
   ```

3. **Corporate environment:**
   ```bash
   cat YourVehicleApp.cpp | docker run --rm -i \
     -e HTTP_PROXY=http://proxy:3128 \
     ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:latest
   ```

### 🎯 **Use Cases**

- **Rapid Prototyping** - Quick iteration and testing
- **CI/CD Pipelines** - Automated building and testing
- **Corporate Environments** - Proxy and firewall compatibility  
- **Educational** - No complex setup for learning
- **Production Builds** - Consistent, reproducible binaries
- **Custom VSS** - Company-specific vehicle specifications

---

**🎉 Ready to build your Velocitas C++ vehicle apps instantly with zero setup complexity!**

*For support and issues, please visit: https://github.com/tri2510/vehicle-app-cpp-template/issues*