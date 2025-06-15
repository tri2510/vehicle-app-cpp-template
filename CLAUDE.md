# Velocitas C++ Vehicle App Development Examples

## Repository Status Overview
This repository contains **3 fully implemented vehicle applications** plus **2 tutorial examples** using the Eclipse Velocitas framework with a Docker-only development environment.

**✅ Production-Ready Applications:** 3 complete implementations with source code, tests, and documentation  
**📚 Tutorial Examples:** 2 detailed architectural guides and design documentation  

## Development Environment
- **Docker-based development** - No devcontainer setup required
- **Complete Velocitas SDK** integration (v0.7.0)
- **Vehicle Data Broker** and **MQTT** communication
- **Full development toolchain** with build, test, and debugging capabilities
- **Comprehensive testing** with GoogleTest framework

## ✅ Fully Implemented Vehicle Applications

### 1. Speed Monitor & Alert System (`examples/speed-monitor/`)
**Status**: ✅ **PRODUCTION READY**  
**Purpose**: Monitor vehicle speed and provide alerts when speed limits are exceeded or sudden speed changes occur.

**Features**:
- Real-time speed monitoring from Vehicle.Speed signal
- Configurable speed limit alerts
- Sudden acceleration/deceleration detection
- MQTT notifications for speed events
- Dashboard integration for speed statistics

**Vehicle Signals Used**:
- `Vehicle.Speed` (read)
- `Vehicle.Acceleration.Longitudinal` (read)

**MQTT Topics**:
- `speedmonitor/alerts` (publish)
- `speedmonitor/config` (subscribe)
- `speedmonitor/status` (publish)

### 2. Fuel Efficiency Tracker (`examples/fuel-efficiency-tracker/`)
**Status**: ✅ **PRODUCTION READY**  
**Purpose**: Track fuel consumption patterns and provide efficiency recommendations to optimize driving behavior.

**Features**:
- Real-time fuel consumption monitoring
- Trip-based efficiency calculations
- Driving behavior analysis
- Efficiency recommendations
- Historical data tracking

**Vehicle Signals Used**:
- `Vehicle.Powertrain.FuelSystem.Level` (read)
- `Vehicle.Speed` (read)
- `Vehicle.Powertrain.Engine.Speed` (read)
- `Vehicle.Acceleration.Longitudinal` (read)

**MQTT Topics**:
- `fueltracker/efficiency` (publish)
- `fueltracker/recommendations` (publish)
- `fueltracker/trip` (publish)

### 3. Maintenance Reminder System (`examples/maintenance-reminder/`)
**Status**: ✅ **PRODUCTION READY**  
**Purpose**: Monitor vehicle diagnostics and provide proactive maintenance reminders based on usage patterns and diagnostic data.

**Features**:
- Engine diagnostics monitoring
- Mileage-based maintenance scheduling
- Diagnostic trouble code (DTC) detection
- Maintenance history tracking
- Service appointment recommendations

**Vehicle Signals Used**:
- `Vehicle.OBD.EngineLoad` (read)
- `Vehicle.Service.DistanceToService` (read)
- `Vehicle.OBD.DTCs` (read)
- `Vehicle.TraveledDistance` (read)

**MQTT Topics**:
- `maintenance/alerts` (publish)
- `maintenance/schedule` (publish)
- `maintenance/diagnostics` (publish)

## 📚 Tutorial Examples

### 4. Parking Assistant (`examples/parking-assistant/`)
**Status**: 📚 **TUTORIAL DOCUMENTATION**  
**Purpose**: Assist drivers with parking using proximity sensors and provide guidance for optimal parking positioning.

**Features**:
- Proximity sensor monitoring
- Parking space detection
- Visual and audio guidance
- Collision avoidance alerts
- Parking completion confirmation

**Vehicle Signals Used**:
- `Vehicle.ADAS.ObstacleDetection.Front.Distance` (read)
- `Vehicle.ADAS.ObstacleDetection.Rear.Distance` (read)
- `Vehicle.ADAS.ObstacleDetection.Left.Distance` (read)
- `Vehicle.ADAS.ObstacleDetection.Right.Distance` (read)
- `Vehicle.Speed` (read)

**MQTT Topics**:
- `parking/guidance` (publish)
- `parking/alerts` (publish)
- `parking/status` (publish)

### 5. Climate Control Optimizer (`examples/climate-control/`)
**Status**: 📚 **TUTORIAL DOCUMENTATION**  
**Purpose**: Automatically optimize climate control settings based on passenger preferences, external conditions, and energy efficiency considerations.

**Features**:
- Automatic temperature adjustment
- Passenger presence detection
- External weather integration
- Energy efficiency optimization
- User preference learning

**Vehicle Signals Used**:
- `Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature` (read/write)
- `Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature` (read/write)
- `Vehicle.Exterior.AirTemperature` (read)
- `Vehicle.Cabin.Seat.Row1.Pos1.IsOccupied` (read)
- `Vehicle.Cabin.Seat.Row1.Pos2.IsOccupied` (read)

**MQTT Topics**:
- `climate/settings` (subscribe)
- `climate/status` (publish)
- `climate/efficiency` (publish)

## Development Workflow

### Quick Start for Implemented Examples
```bash
# 1. Start development environment
docker build -f Dockerfile.dev -t velocitas-dev .
docker run -it --privileged -v $(pwd):/workspace -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

# 2. Build and test the main application (includes all implemented examples)
install-deps
build-app

# 3. Test the application
runtime-up        # Start MQTT and VDB services  
run-app          # Run the vehicle app
```

### Working with Individual Examples
The 3 implemented examples are integrated into the main application. To understand each:

1. **Speed Monitor**: Study `examples/speed-monitor/src/SpeedMonitorApp.cpp`
2. **Fuel Efficiency**: Study `examples/fuel-efficiency-tracker/src/FuelEfficiencyApp.cpp` 
3. **Maintenance Reminder**: Study `examples/maintenance-reminder/src/MaintenanceReminderApp.cpp`

### Implemented Example Structure
Each implemented example includes:
```
examples/<example-name>/
├── src/
│   ├── <ExampleName>App.cpp    # ✅ Complete implementation
│   └── <ExampleName>App.h      # ✅ Full header file
├── tests/
│   └── <ExampleName>Test.cpp   # ✅ Comprehensive unit tests
└── AppManifest.json            # ✅ App configuration
```

### Tutorial Example Structure  
Tutorial examples provide:
```
examples/<example-name>/
├── src/                        # 📚 Empty (tutorial only)
├── tests/                      # 📚 Empty (tutorial only)
└── Detailed documentation in EXAMPLES_TUTORIAL.md
```

## Building and Testing Examples

### Build All Examples
```bash
# Build all examples
for example in examples/*/; do
    echo "Building $(basename $example)..."
    cd $example
    install-deps && build-app
    cd ../..
done
```

### Run Integration Tests
```bash
# Test with full runtime stack
docker-compose -f docker-compose.dev.yml up -d

# Run specific example
docker-compose -f examples/<example>/docker-compose.test.yml run test
```

## Key Learning Objectives

### 1. Vehicle Signal Integration ✅
**Learn from implemented examples:**
- How to read from and write to vehicle data points
- Understanding Vehicle Signal Specification (VSS)
- Working with the Vehicle Data Broker
- **Reference**: `SpeedMonitorApp.cpp:39-49` for subscription patterns

### 2. MQTT Communication ✅  
**Learn from implemented examples:**
- Publishing and subscribing to MQTT topics
- Message formatting and parsing
- Asynchronous message handling
- **Reference**: `FuelEfficiencyApp.cpp:500-518` for alert publishing

### 3. Application Architecture ✅
**Learn from implemented examples:**
- Velocitas framework patterns
- Event-driven programming
- Error handling and resilience
- **Reference**: `MaintenanceReminderApp.cpp:185-188` for error handling

### 4. Testing and Debugging ✅
**Fully implemented:**
- Unit testing with GoogleTest (all examples have comprehensive test suites)
- Integration testing with real services
- Debugging techniques for vehicle applications

### 5. Deployment and Configuration ✅
**Production-ready:**
- Docker containerization
- Environment configuration  
- Service orchestration

## Best Practices Demonstrated

1. **Signal Management**: Efficient subscription and data handling
2. **Error Handling**: Robust error handling for unreliable vehicle data
3. **Performance**: Optimized for real-time vehicle environments
4. **Testing**: Comprehensive unit and integration testing
5. **Documentation**: Clear documentation and tutorials
6. **Configuration**: Flexible configuration management

## Development Tips

### Performance Considerations
- Minimize signal subscriptions to only required data points
- Use appropriate data types for vehicle signals
- Implement efficient message parsing and handling
- Consider memory usage in embedded environments

### Testing Strategies
- Mock vehicle signals for unit testing
- Use simulation data for integration testing
- Test error conditions and edge cases
- Validate MQTT message formats

### Debugging Techniques
- Use structured logging for troubleshooting
- Monitor MQTT traffic for message flow analysis
- Analyze Vehicle Data Broker connections
- Profile application performance

## Entry Points for New Developers

### 🚀 **Start Here**
1. **[GETTING_STARTED.md](GETTING_STARTED.md)** - Your comprehensive entry point
2. **[DOCKER_DEVELOPMENT.md](DOCKER_DEVELOPMENT.md)** - Complete development environment guide
3. **[examples/EXAMPLES_TUTORIAL.md](examples/EXAMPLES_TUTORIAL.md)** - Detailed tutorials for all examples

### 📚 **External Resources**
- [Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)
- [Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)
- [Eclipse KUKSA](https://github.com/eclipse/kuksa.val)

### ✅ **What You Get**
- **3 production-ready vehicle applications** with complete source code
- **Comprehensive unit tests** for all implemented features
- **Extensive documentation** and tutorials
- **Working Docker development environment** with zero setup complexity

### 📚 **Tutorial Coverage**
- **2 additional example concepts** with detailed architectural documentation
- **Design patterns** and best practices for vehicle application development
- **Implementation guidance** for extending the existing examples