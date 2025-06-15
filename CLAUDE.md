# Velocitas C++ Vehicle App Development Examples

## Overview
This repository contains comprehensive examples of vehicle app development using the Eclipse Velocitas framework with a Docker-only development environment. Each example demonstrates different aspects of automotive application development.

## Development Environment
- **Docker-based development** - No devcontainer setup required
- **Complete Velocitas SDK** integration
- **Vehicle Data Broker** and **MQTT** communication
- **Full development toolchain** with build, test, and debugging capabilities

## Example Vehicle Applications

### 1. Speed Monitor & Alert System (`examples/speed-monitor/`)
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

### 2. Fuel Efficiency Tracker (`examples/fuel-tracker/`)
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

### 4. Parking Assistant (`examples/parking-assistant/`)
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

### 5. Climate Control Optimizer (`examples/climate-optimizer/`)
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

### Quick Start for Any Example
```bash
# 1. Start development environment
docker build -f Dockerfile.dev -t velocitas-dev .
docker run -it --privileged -v $(pwd):/workspace -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

# 2. Build specific example
cd examples/<example-name>
install-deps
build-app

# 3. Test the application
runtime-up        # Start MQTT and VDB services
run-app          # Run the example app
```

### Example Structure
Each example follows this structure:
```
examples/<example-name>/
├── src/
│   ├── <ExampleName>App.cpp    # Main application implementation
│   ├── <ExampleName>App.h      # Application header
│   └── main.cpp                # Application entry point
├── tests/
│   └── <ExampleName>_test.cpp  # Unit tests
├── AppManifest.json            # Velocitas app configuration
├── CMakeLists.txt              # Build configuration
├── conanfile.txt               # Dependencies
├── README.md                   # Detailed tutorial
└── docker-compose.test.yml     # Testing environment
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

### 1. Vehicle Signal Integration
- How to read from and write to vehicle data points
- Understanding Vehicle Signal Specification (VSS)
- Working with the Vehicle Data Broker

### 2. MQTT Communication
- Publishing and subscribing to MQTT topics
- Message formatting and parsing
- Asynchronous message handling

### 3. Application Architecture
- Velocitas framework patterns
- Event-driven programming
- Error handling and resilience

### 4. Testing and Debugging
- Unit testing with GoogleTest
- Integration testing with real services
- Debugging techniques for vehicle applications

### 5. Deployment and Configuration
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

## Resources

- [DOCKER_DEVELOPMENT.md](DOCKER_DEVELOPMENT.md) - Complete development environment guide
- [Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)
- [Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)
- [Eclipse KUKSA](https://github.com/eclipse/kuksa.val)

Each example includes detailed tutorials, code explanations, and testing instructions to help developers learn vehicle application development progressively.