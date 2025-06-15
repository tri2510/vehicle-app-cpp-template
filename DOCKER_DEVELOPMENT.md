# Velocitas C++ Vehicle App - Docker Development Environment

This repository provides a complete Docker-based development environment for building and running Velocitas C++ vehicle applications. No devcontainer or complex setup required - just Docker.

## 🚀 Quick Start

### Prerequisites
- Docker installed and running
- Git (for cloning and version control)

### 1. Clone and Start Development
```bash
# Clone the repository
git clone <repository-url>
cd vehicle-app-cpp-template

# Build the development image
docker build -f Dockerfile.dev -t velocitas-dev .

# Start development container
docker run -it --privileged -v $(pwd):/workspace -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev
```

### 2. Build and Run Your App
```bash
# Inside the container:
install-deps  # Install dependencies
build-app     # Build the application
run-app       # Run the vehicle app
```

That's it! Your Velocitas vehicle app development environment is ready.

## 📁 Project Structure

```
vehicle-app-cpp-template/
├── app/                      # Vehicle application source code
│   ├── src/                  # C++ source files
│   │   ├── SampleApp.cpp     # Main vehicle app implementation
│   │   └── SampleApp.h       # App header file
│   ├── tests/                # Unit tests
│   └── AppManifest.json      # App configuration
├── config/                   # Configuration files
│   └── mosquitto.conf        # MQTT broker configuration
├── Dockerfile.dev            # Development Docker image
├── docker-compose.dev.yml    # Full development stack
├── CMakeLists.txt            # Build configuration
├── conanfile.txt            # C++ dependencies
└── requirements.txt         # Python dependencies
```

## 🛠️ Development Commands

The development container provides convenient commands for all development tasks:

### Build & Dependencies
```bash
install-deps    # Install C++ dependencies via Conan
build-app      # Build the vehicle application
build-app -r   # Build in Release mode
```

### Runtime Services
```bash
runtime-up     # Start MQTT broker and Vehicle Data Broker
runtime-down   # Stop runtime services
run-app        # Run the vehicle app with services
```

### Code Quality
```bash
check-code     # Run linting and formatting checks
gen-model      # Generate vehicle model from VSS
gen-grpc       # Generate gRPC SDKs
```

### Vehicle Data Broker
```bash
vdb-cli        # Access Vehicle Data Broker CLI
```

## 🐳 Docker Development Options

### Option 1: Quick Development (Recommended)
```bash
# Build and run development container
docker build -f Dockerfile.dev -t velocitas-dev .
docker run -it --privileged -v $(pwd):/workspace -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

# Inside container - start developing immediately
install-deps && build-app
```

### Option 2: Full Development Stack
```bash
# Start complete development environment with services
docker-compose -f docker-compose.dev.yml up -d

# Access development container
docker-compose -f docker-compose.dev.yml exec dev bash

# Services included:
# - dev: Main development container
# - mosquitto: MQTT broker (port 1883)
# - vehicledatabroker: Vehicle Data Broker (port 55555)
```

### Option 3: Production Build Test
```bash
# Test production build
docker run --rm -it --net="host" \
  -e SDV_MIDDLEWARE_TYPE="native" \
  -e SDV_MQTT_ADDRESS="localhost:1883" \
  -e SDV_VEHICLEDATABROKER_ADDRESS="localhost:55555" \
  localhost:12345/vehicleapp:local
```

## 🚗 Vehicle App Development

### Sample Application Features
The included sample app demonstrates:
- **Vehicle Signal Interface**: Reads `Vehicle.Speed` from Vehicle Data Broker
- **MQTT Pub/Sub**: Listens on `sampleapp/getSpeed` topic
- **Message Processing**: Responds to speed requests
- **Data Broadcasting**: Publishes speed updates to `sampleapp/currentSpeed`

### Key Files to Modify
1. **app/src/SampleApp.cpp**: Main application logic
2. **app/src/SampleApp.h**: Application header
3. **app/AppManifest.json**: Configure vehicle signals and topics
4. **CMakeLists.txt**: Add new source files or dependencies

### Development Workflow
1. **Modify Code**: Edit source files in `app/src/`
2. **Build**: Run `build-app` to compile
3. **Test**: Run unit tests with the generated `app_utests` binary
4. **Run**: Test with `run-app` or start services manually
5. **Debug**: Use GDB or your preferred debugging tools

## 🔧 Configuration

### Environment Variables
```bash
# MQTT Broker Configuration
SDV_MQTT_ADDRESS=localhost:1883

# Vehicle Data Broker Configuration  
SDV_VEHICLEDATABROKER_ADDRESS=localhost:55555

# Middleware Type
SDV_MIDDLEWARE_TYPE=native
```

### Vehicle Signals Configuration
Edit `app/AppManifest.json` to configure:
- Required vehicle signals (e.g., Vehicle.Speed, Vehicle.RPM)
- MQTT topics for pub/sub
- Signal access permissions (read/write)

### Dependencies
Add C++ dependencies in `conanfile.txt`:
```ini
[requires]
vehicle-app-sdk/0.7.0
fmt/11.1.1
nlohmann_json/3.11.3
# Add your dependencies here

[generators]
CMakeDeps
CMakeToolchain
```

## 🧪 Testing

### Unit Tests
```bash
# Build and run unit tests
build-app
./build/bin/app_utests
```

### Integration Testing
```bash
# Start services
runtime-up

# Run vehicle app
run-app

# In another terminal, test MQTT messaging
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_pub \
  -h localhost -t "sampleapp/getSpeed" -m '{"requestId": "test123"}'
```

### Code Quality Checks
```bash
# Run all code quality checks
check-code

# Individual tools:
# - clang-format: Code formatting
# - clang-tidy: Static analysis
# - cpplint: Style checking
# - cppcheck: Error detection
```

## 🔍 Debugging

### GDB Debugging
```bash
# Build with debug symbols (default in Debug mode)
build-app

# Start GDB
gdb ./build/bin/app

# Set breakpoints and run
(gdb) break main
(gdb) run
```

### Log Analysis
```bash
# View application logs
run-app

# MQTT message debugging
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_sub \
  -h localhost -t "sampleapp/#" -v
```

## 📦 Deployment

### Building Production Image
```bash
# Build app first
build-app

# Create production Dockerfile (example)
cat > Dockerfile.prod << EOF
FROM ghcr.io/eclipse-velocitas/devcontainer-base-images/cpp:v0.4
COPY build/bin/app /usr/local/bin/
COPY app/AppManifest.json /app/
CMD ["/usr/local/bin/app"]
EOF

# Build production image
docker build -f Dockerfile.prod -t vehicle-app:latest .
```

### Running in Production
```bash
docker run --rm -it \
  -e SDV_MQTT_ADDRESS="production-mqtt:1883" \
  -e SDV_VEHICLEDATABROKER_ADDRESS="production-vdb:55555" \
  vehicle-app:latest
```

## 🐛 Troubleshooting

### Common Issues

**Permission Errors**
```bash
# Fix file permissions
docker run --rm -v $(pwd):/workspace --privileged velocitas-dev \
  bash -c "chown -R vscode:vscode /workspace"
```

**Build Failures**
```bash
# Clean and rebuild
rm -rf build-linux-x86_64
install-deps
build-app
```

**Connection Issues**
```bash
# Check services are running
runtime-up
docker ps  # Should show MQTT and VDB containers

# Test connectivity
telnet localhost 1883  # MQTT
telnet localhost 55555 # VDB
```

**Container Issues**
```bash
# Rebuild development image
docker build --no-cache -f Dockerfile.dev -t velocitas-dev .

# Clean up containers
docker system prune -f
```

## 📚 Additional Resources

- [Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)
- [Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)
- [KUKSA.val Project](https://github.com/eclipse/kuksa.val)
- [Eclipse Mosquitto](https://mosquitto.org/)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes and test them
4. Run code quality checks: `check-code`
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.