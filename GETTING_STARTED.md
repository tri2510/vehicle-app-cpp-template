# Getting Started with Velocitas C++ Vehicle App Development

Welcome to the **Velocitas C++ Vehicle App Template** - your comprehensive entry point for developing cutting-edge vehicle applications using the Eclipse Velocitas framework with Docker-only development environment.

## 📋 Table of Contents

1. [What This Repository Offers](#what-this-repository-offers)
2. [Quick Start (5 Minutes)](#quick-start-5-minutes)
3. [Development Environment](#development-environment)
4. [Example Applications](#example-applications)
5. [Development Workflow](#development-workflow)
6. [Next Steps for New Developers](#next-steps-for-new-developers)

## What This Repository Offers

This repository provides:

✅ **Complete Docker-only development environment** - No complex setup required  
✅ **3 fully implemented vehicle app examples** with complete source code  
✅ **Production-ready implementations** with comprehensive testing frameworks  
✅ **Extensive documentation** and tutorials for each example  
✅ **All development tools** integrated (build, test, lint, debug)  
✅ **Real vehicle data integration** via KUKSA.val Vehicle Data Broker  
✅ **MQTT communication** for external system integration  

## Quick Start (5 Minutes)

### Prerequisites
- [Docker](https://docker.com) installed and running
- Git for version control
- **4GB+ RAM** recommended for development

### Get Running in 3 Steps

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

### Inside the Container - Your First Build

```bash
# Install dependencies and build the default app
install-deps && build-app

# Start vehicle services (MQTT + Vehicle Data Broker)
runtime-up

# Run your first vehicle app
run-app
```

**🎉 Congratulations!** You now have a working vehicle application development environment.

## Development Environment

### What's Included

| Component | Purpose | Port |
|-----------|---------|------|
| **Velocitas C++ SDK** | Vehicle app development framework | - |
| **KUKSA.val VDB** | Vehicle signal data broker | 55555 |
| **Eclipse Mosquitto** | MQTT message broker | 1883, 9001 |
| **Build Tools** | GCC, CMake, Ninja, Conan | - |
| **Quality Tools** | clang-format, clang-tidy, cppcheck | - |
| **Testing Tools** | GoogleTest, gcovr | - |

### Available Commands

```bash
# Development
install-deps       # Install C++ dependencies via Conan
build-app         # Build the vehicle application  
build-app -r      # Build in Release mode

# Runtime
runtime-up        # Start MQTT broker and Vehicle Data Broker
runtime-down      # Stop runtime services
run-app           # Run the vehicle app with services

# Code Quality
check-code        # Run linting, formatting, and static analysis
./build/bin/app_utests  # Run unit tests

# Vehicle Tools
gen-model         # Generate vehicle model from VSS
gen-grpc          # Generate gRPC SDKs
vdb-cli           # Access Vehicle Data Broker CLI
```

## Example Applications

This repository includes **3 fully implemented vehicle applications** demonstrating different aspects of automotive software development:

### 1. 🚗 Speed Monitor & Alert System
**Status:** ✅ **FULLY IMPLEMENTED**

**What it does:**
- Real-time speed monitoring with configurable limits
- Hard braking and rapid acceleration detection  
- Configurable speed limit alerts via MQTT
- Speed statistics tracking with rolling averages
- Alert cooldown to prevent notification spam

**Key Learning:** Vehicle signal subscription, data processing, MQTT communication

**Files:** `examples/speed-monitor/`
- `SpeedMonitorApp.cpp` - Complete implementation (340 lines)
- `SpeedMonitorApp.h` - Header with full API
- `SpeedMonitorAppTest.cpp` - Comprehensive unit tests

### 2. ⛽ Fuel Efficiency Tracker  
**Status:** ✅ **FULLY IMPLEMENTED**

**What it does:**
- Real-time fuel efficiency calculation (km/L and L/100km)
- Trip-based fuel tracking with start/stop commands
- Rolling efficiency averages with configurable windows
- Eco-driving tips based on driving patterns
- Fuel consumption alerts and optimization suggestions

**Key Learning:** Complex data processing, trip management, eco-algorithms

**Files:** `examples/fuel-efficiency-tracker/`
- `FuelEfficiencyApp.cpp` - Complete implementation (573 lines)
- `FuelEfficiencyApp.h` - Header with full API
- `FuelEfficiencyAppTest.cpp` - Comprehensive unit tests

### 3. 🔧 Maintenance Reminder System
**Status:** ✅ **FULLY IMPLEMENTED**

**What it does:**
- Multiple service interval tracking (distance, time, engine hours)
- Proactive maintenance reminders with priority levels
- Service history and cost tracking with full records
- Overdue service alerts with escalation
- Configurable reminder thresholds

**Key Learning:** Complex state management, scheduling algorithms, data persistence

**Files:** `examples/maintenance-reminder/`
- `MaintenanceReminderApp.cpp` - Complete implementation (720 lines)
- `MaintenanceReminderApp.h` - Comprehensive header (346 lines)
- `MaintenanceReminderAppTest.cpp` - Full unit test suite

### 4. 🅿️ Parking Assistant *(In Tutorial)*
**Status:** 📚 **TUTORIAL ONLY**

**Tutorial covers:**
- Multi-sensor distance monitoring concepts
- Progressive alert algorithms
- Collision warning system design

### 5. 🌡️ Climate Control Optimizer *(In Tutorial)*
**Status:** 📚 **TUTORIAL ONLY**

**Tutorial covers:**
- Automatic temperature optimization concepts
- Energy-efficient climate control algorithms
- Occupancy-based adjustment strategies

## Development Workflow

### Option 1: Quick Development (Recommended for Beginners)
```bash
# Single container with all tools
docker run -it --privileged -v $(pwd):/workspace \
  -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

# Inside container - full development cycle
install-deps && build-app && runtime-up && run-app
```

### Option 2: Full Development Stack (Advanced)
```bash
# Complete environment with all services
docker-compose -f docker-compose.dev.yml up -d

# Access development container
docker-compose -f docker-compose.dev.yml exec dev bash
```

### Testing Your Applications

```bash
# Build and run unit tests
build-app
./build/bin/app_utests

# Run code quality checks
check-code

# Integration testing with real services
runtime-up
run-app &

# Test MQTT communication
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_pub \
  -h localhost -t "speedmonitor/config" -m '{"speed_limit_kmh": 60}'

docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_sub \
  -h localhost -t "speedmonitor/alerts"
```

## Next Steps for New Developers

### 1. Start with Speed Monitor (Beginner)
```bash
cd examples/speed-monitor
# Read the source code to understand:
# - Vehicle signal subscription patterns
# - MQTT communication
# - Configuration management
# - Alert systems
```

### 2. Explore Fuel Efficiency (Intermediate)
```bash
cd examples/fuel-efficiency-tracker  
# Learn advanced concepts:
# - Complex data processing algorithms
# - Trip state management
# - Rolling calculations
# - Multi-signal coordination
```

### 3. Study Maintenance Reminder (Advanced)
```bash
cd examples/maintenance-reminder
# Master complex patterns:
# - Multi-service scheduling
# - Time-based calculations
# - Priority management
# - Historical data tracking
```

### 4. Create Your Own Application

Follow the established patterns to create new vehicle apps:

1. **Create project structure** following existing examples
2. **Inherit from VehicleApp** base class
3. **Subscribe to vehicle signals** using QueryBuilder
4. **Implement MQTT communication** for external integration  
5. **Add comprehensive testing** using GoogleTest
6. **Document your implementation** with clear examples

### 5. Development Resources

**Essential Documentation:**
- [DOCKER_DEVELOPMENT.md](DOCKER_DEVELOPMENT.md) - Complete development guide
- [examples/EXAMPLES_TUTORIAL.md](examples/EXAMPLES_TUTORIAL.md) - Detailed tutorials
- [Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/) - Framework docs
- [Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/) - VSS reference

**Code References:**
All implementations follow consistent patterns - use existing examples as templates for:
- Vehicle data subscription: `examples/speed-monitor/src/SpeedMonitorApp.cpp:39-49`
- MQTT communication: `examples/speed-monitor/src/SpeedMonitorApp.cpp:207-225`
- Configuration handling: `examples/fuel-efficiency-tracker/src/FuelEfficiencyApp.cpp:365-374`
- Error handling: `examples/maintenance-reminder/src/MaintenanceReminderApp.cpp:185-188`

## Repository Status Summary

**What's Ready for Production:**
- ✅ 3 fully implemented vehicle applications with complete source code
- ✅ Comprehensive testing frameworks and unit tests
- ✅ Complete Docker development environment
- ✅ Full build and deployment pipeline
- ✅ Extensive documentation and tutorials

**What's in Tutorial Form:**
- 📚 2 additional example concepts with detailed documentation
- 📚 Architecture guidance for implementing additional features
- 📚 Best practices and design patterns

This repository provides everything you need to understand and develop professional vehicle applications using the Eclipse Velocitas framework.

## Support and Community

- **Issues:** Report bugs at [GitHub Issues](https://github.com/tri2510/vehicle-app-cpp-template/issues)
- **Discussions:** Join the [Eclipse Velocitas Community](https://github.com/eclipse-velocitas)
- **Documentation:** Comprehensive guides in each example directory

---

**Happy Vehicle App Development! 🚗💨**

Ready to build the future of automotive software? Start with the Speed Monitor example and work your way through our comprehensive tutorial series.