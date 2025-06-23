# 🚗 SDV C++ Examples - Production-Ready Vehicle Applications

This directory contains **6 comprehensive Software-Defined Vehicle (SDV) examples** demonstrating real-world automotive use cases with production-quality code.

## 🎯 Quick Start - Copy & Run Any Example

Each example is a complete, working application that you can instantly try:

```bash
# 1. Choose any example and copy it over the template
cp examples/CollisionWarningSystem.cpp templates/app/src/VehicleApp.cpp

# 2. Build and run instantly
cat templates/app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick
```

## 📋 Available Examples

### 1. 🛡️ ADAS Collision Warning System
**File**: `CollisionWarningSystem.cpp`  
**Copy Command**: `cp examples/CollisionWarningSystem.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Real-time collision risk assessment
- Multi-threshold alert system (warning, critical, emergency)
- Emergency braking detection and assistance
- Time-to-collision estimation
- Safety-critical error handling with fail-safe defaults

**VSS Signals Used**:
- `Vehicle.Speed` - Current vehicle speed
- `Vehicle.Acceleration.Longitudinal` - Forward/backward acceleration  
- `Vehicle.ADAS.ABS.IsActive` - Anti-lock braking system status
- `Vehicle.Chassis.Brake.PedalPosition` - Brake pedal position

**Key Features**:
- Forward collision warning based on speed and acceleration
- Emergency braking assistance logic
- Driver behavior analysis for safety coaching
- Production-ready safety thresholds and responses

---

### 2. 📊 Smart Fleet Telematics
**File**: `FleetManagementApp.cpp`  
**Copy Command**: `cp examples/FleetManagementApp.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Real-time GPS tracking and route optimization
- Fuel efficiency monitoring and reporting
- Driver behavior analysis and scoring
- Maintenance scheduling based on usage patterns
- Fleet dashboard integration with comprehensive analytics

**VSS Signals Used**:
- `Vehicle.CurrentLocation.Latitude/Longitude` - GPS coordinates
- `Vehicle.Powertrain.FuelSystem.Level` - Fuel level percentage
- `Vehicle.Service.DistanceToService` - Maintenance scheduling
- `Vehicle.Powertrain.Engine.Speed` - Engine RPM for diagnostics

**Key Features**:
- Comprehensive driver performance scoring
- Predictive fuel efficiency analysis
- Idle time monitoring and optimization
- Integration-ready for fleet management platforms

---

### 3. 🌡️ Intelligent Climate Control
**File**: `SmartClimateApp.cpp`  
**Copy Command**: `cp examples/SmartClimateApp.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Multi-zone temperature control based on occupancy detection
- Predictive climate adjustment using external conditions
- Energy-efficient HVAC operation with battery optimization
- Comfort preference learning and personalization
- Integration with vehicle energy management systems

**VSS Signals Used**:
- `Vehicle.Cabin.HVAC.Station.Row1.Left/Right.Temperature` - Zone temperatures
- `Vehicle.Cabin.Seat.Row*.*.IsOccupied` - Occupancy detection
- `Vehicle.Exterior.AirTemperature` - Outside temperature
- `Vehicle.Cabin.HVAC.IsAirConditioningActive` - AC system status

**Key Features**:
- Adaptive machine learning for comfort preferences
- Energy optimization for unoccupied zones
- Predictive pre-conditioning based on weather
- Comprehensive comfort scoring and analytics

---

### 4. 🔋 EV Energy Management System
**File**: `EVEnergyManager.cpp`  
**Copy Command**: `cp examples/EVEnergyManager.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Intelligent battery optimization and health monitoring
- Smart charging strategy based on grid conditions and usage patterns
- Advanced range prediction with real-time efficiency analysis
- Energy recovery optimization during regenerative braking
- Charging station recommendation with route planning

**VSS Signals Used**:
- `Vehicle.Powertrain.TractionBattery.StateOfCharge.Current` - Battery level
- `Vehicle.Powertrain.TractionBattery.Charging.IsCharging` - Charging status
- `Vehicle.Powertrain.ElectricMotor.Power` - Motor power consumption
- `Vehicle.Powertrain.TractionBattery.Range` - Estimated range

**Key Features**:
- Predictive battery degradation monitoring
- Smart charging optimization for cost and battery health
- Real-time efficiency coaching and recommendations
- Integration with charging networks and grid services

---

### 5. 🔧 Predictive Maintenance System
**File**: `MaintenancePredictor.cpp`  
**Copy Command**: `cp examples/MaintenancePredictor.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Component health monitoring with trend analysis
- Predictive failure detection using machine learning techniques
- Optimal service scheduling and cost optimization
- Integration with service provider APIs and booking systems
- Real-time diagnostic alerts and maintenance recommendations

**VSS Signals Used**:
- `Vehicle.Service.DistanceToService` - Service intervals
- `Vehicle.Powertrain.Engine.ECT` - Engine coolant temperature
- `Vehicle.OBD.EngineLoad` - Engine load for wear analysis
- `Vehicle.Powertrain.Engine.Speed` - Engine RPM patterns

**Key Features**:
- AI-powered failure prediction algorithms
- Cost optimization for maintenance scheduling
- Component degradation tracking and analysis
- Automated service appointment coordination

---

### 6. 📡 V2X Communication Hub
**File**: `V2XCommunicator.cpp`  
**Copy Command**: `cp examples/V2XCommunicator.cpp templates/app/src/VehicleApp.cpp`

**What it does**:
- Vehicle-to-vehicle (V2V) hazard warnings and coordination
- Vehicle-to-infrastructure (V2I) traffic signal optimization
- Cooperative adaptive cruise control with surrounding vehicles
- Emergency vehicle priority coordination and routing
- Real-time traffic condition sharing and crowd-sourced intelligence

**VSS Signals Used**:
- `Vehicle.CurrentLocation.Latitude/Longitude` - Position sharing
- `Vehicle.Speed` - Speed sharing for traffic flow
- `Vehicle.ConnectedServices.V2X.Messages` - V2X message handling
- `Vehicle.ADAS.TIS.IsEnabled` - Traffic information system

**Key Features**:
- Cooperative driving coordination and safety
- Traffic signal phase and timing optimization (SPAT)
- Emergency vehicle detection and priority handling
- Connected autonomous driving support

---

## 🛠️ Usage Instructions

### Basic Usage
```bash
# 1. Copy any example to the template location
cp examples/[EXAMPLE_NAME].cpp templates/app/src/VehicleApp.cpp

# 2. Build and run with the quick build system
cat templates/app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick

# 3. Watch the application run and process vehicle signals
```

### Advanced Usage
```bash
# Build and run with live services (Vehicle Data Broker + MQTT)
cat templates/app/src/VehicleApp.cpp | docker run --rm -i --network=host velocitas-quick run

# Run validation only (syntax and structure check)
cat templates/app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick validate

# Use with custom VSS specification
docker run --rm -i \
  -e VSS_SPEC_URL=https://your-company.com/custom-vss.json \
  velocitas-quick < templates/app/src/VehicleApp.cpp
```

### Testing with Live Services
```bash
# 1. Start Vehicle Data Broker and MQTT services
docker compose -f docker-compose.dev.yml up vehicledatabroker mosquitto -d

# 2. Run the example with network access
cp examples/CollisionWarningSystem.cpp templates/app/src/VehicleApp.cpp
cat templates/app/src/VehicleApp.cpp | docker run --rm -i --network=host velocitas-quick run

# 3. In another terminal, simulate vehicle signals with KUKSA client
docker run -it --rm --network=host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main

# 4. Inside KUKSA client, set vehicle signals:
# setValue Vehicle.Speed 25.0
# setValue Vehicle.Acceleration.Longitudinal -2.5
# setValue Vehicle.Powertrain.FuelSystem.Level 15.0

# 5. Watch your application respond to the signals in real-time!

# 6. Clean up when done
docker compose -f docker-compose.dev.yml down
```

## 🎓 Learning Path

### For Beginners
1. **Start with**: `CollisionWarningSystem.cpp` - Simple safety monitoring
2. **Then try**: `SmartClimateApp.cpp` - Multi-signal processing
3. **Advanced**: `FleetManagementApp.cpp` - Comprehensive data analytics

### For Fleet Management
1. **Start with**: `FleetManagementApp.cpp` - Driver and vehicle monitoring
2. **Add**: `MaintenancePredictor.cpp` - Predictive maintenance
3. **Optimize**: `EVEnergyManager.cpp` - Energy efficiency (if applicable)

### For Autonomous Driving
1. **Start with**: `V2XCommunicator.cpp` - Vehicle communication
2. **Add**: `CollisionWarningSystem.cpp` - Safety systems
3. **Integrate**: `SmartClimateApp.cpp` - Passenger comfort

## 🔧 Customization

Each example is designed to be easily customizable:

### Adjusting Thresholds
```cpp
// In any example, look for these patterns:
static constexpr double SPEED_WARNING_THRESHOLD = 22.22;    // Modify as needed
static constexpr double FUEL_EFFICIENCY_TARGET = 8.0;       // L/100km
static constexpr double COMFORT_TEMP_MIN = 18.0;            // °C
```

### Adding New Signals
```cpp
// In onStart() method, add more signals:
subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed)
                                           .select(Vehicle.YourNewSignal)  // Add here
                                           .build())

// In the data processing method, handle the new signal:
if (reply.get(Vehicle.YourNewSignal)->isAvailable()) {
    auto value = reply.get(Vehicle.YourNewSignal)->value();
    // Process your signal data
}
```

### Integration with External Systems
Each example includes commented sections showing how to integrate with:
- MQTT brokers for real-time communication
- REST APIs for external service integration
- Databases for data persistence
- Mobile applications for user interfaces
- Cloud platforms for analytics and AI

## 📊 Production Deployment

Each example includes comprehensive production deployment notes covering:

### Infrastructure Integration
- Cloud platform deployment (AWS, Azure, GCP)
- Edge computing integration
- 5G and cellular connectivity
- Enterprise security and compliance

### Analytics and AI
- Machine learning model integration
- Real-time data processing pipelines
- Predictive analytics platforms
- Business intelligence dashboards

### Automotive Standards
- ISO 26262 functional safety compliance
- AUTOSAR integration guidelines
- VSS (Vehicle Signal Specification) best practices
- Cybersecurity frameworks (ISO/SAE 21434)

## 🚀 Next Steps

After trying these examples:

1. **Combine Examples**: Use multiple examples together for comprehensive vehicle applications
2. **Add Custom Logic**: Integrate your own business logic and requirements
3. **Deploy to Production**: Follow the deployment notes for real-world usage
4. **Contribute**: Share your enhancements and improvements

## 📖 Additional Resources

- **[Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/)** - Framework documentation
- **[Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/)** - VSS reference
- **[KUKSA.val](https://github.com/eclipse/kuksa.val)** - Vehicle Data Broker
- **[Eclipse SDV](https://sdv.eclipse.org/)** - Software-Defined Vehicle initiative

---

**Happy SDV Development! 🚗💨**

*These examples demonstrate the power of software-defined vehicles and provide a solid foundation for building production-ready automotive applications.*