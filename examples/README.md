# 🚗 SDV Vehicle Application Examples

This directory contains comprehensive Software-Defined Vehicle (SDV) examples demonstrating real-world automotive applications.

## 📋 Available Examples

### 1. FleetManagementSDV.cpp
**Comprehensive Fleet Management System**

A complete SDV application demonstrating:
- **Multi-signal processing**: Location, speed, fuel, engine, electrical systems
- **Real-time analytics**: Driver behavior scoring, fuel efficiency monitoring
- **Predictive maintenance**: Condition-based alerts and maintenance scheduling
- **Fleet integration**: Dashboard updates and MQTT communication
- **Data aggregation**: Trip logging and performance metrics

**Signals Used:**
- `Vehicle.CurrentLocation.Latitude/Longitude` - GPS tracking
- `Vehicle.Speed` - Speed monitoring and behavior analysis
- `Vehicle.Acceleration.Longitudinal` - Harsh event detection
- `Vehicle.Powertrain.FuelSystem.Level` - Fuel monitoring
- `Vehicle.Powertrain.Engine.Speed/ECT` - Engine diagnostics
- `Vehicle.Service.DistanceToService` - Maintenance scheduling
- `Vehicle.Electrical.Battery.Voltage` - Electrical system health

**SDV Concepts Demonstrated:**
- Vehicle-to-Cloud connectivity
- Data-driven decision making
- Predictive analytics and maintenance
- Fleet optimization and monitoring
- Real-time telemetry processing

## 🚀 How to Use Examples

### Method 1: Copy to Template (Recommended)
```bash
# Copy the example you want to use
cp examples/FleetManagementSDV.cpp templates/app/src/VehicleApp.cpp

# Build and run
docker run -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick build --verbose
docker run -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick run
```

### Method 2: Direct File Mount
```bash
# Build directly from example file
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick build --verbose
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick run
```

### Method 3: Test with Signal Validation
```bash
# Test with multi-container KUKSA setup
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick test signal-validation
```

## 🧪 Testing Examples

The new build system includes comprehensive testing:

```bash
# Test signal processing with KUKSA
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick test signal-validation

# Test build process
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick test build-validation

# Run full test suite
docker run -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick test full-suite
```

## 📊 Expected Output

When running the FleetManagementSDV example, you'll see:

```
🚗 SDV Fleet Management System starting...
📡 Setting up multi-signal monitoring...
✅ Fleet management signals subscribed
📊 Processing fleet telemetry data...
📍 Location Update: 52.520008, 13.404954
🚗 Speed: 65.0 km/h (18.06 m/s)
⛽ Fuel Level: 75.0%
🔧 Engine: 2100 RPM, 87.5°C
📊 Driver Analysis: Score=98.5, AvgSpeed=65.0km/h, Violations=0, HarshEvents=0
📊 Fleet Dashboard Update:
   🚗 Vehicle: SDV-DEMO-001 | Location: 52.5200,13.4050
   📈 Speed: 65.0 km/h | Fuel: 75.0% | Driver Score: 98.5
```

## 🛠️ Customization

### Adding New Signals
To add more vehicle signals to the examples:

1. **Add to subscription in `onStart()`:**
```cpp
.select(Vehicle.YourNewSignal)
```

2. **Process in `onVehicleDataChanged()`:**
```cpp
if (reply.get(Vehicle.YourNewSignal)->isAvailable()) {
    double value = reply.get(Vehicle.YourNewSignal)->value();
    // Process your signal
}
```

### Modifying Analytics
The examples include customizable analytics functions:
- `analyzeDrivingBehavior()` - Modify driver scoring logic
- `predictMaintenance()` - Adjust maintenance prediction algorithms
- `calculateFuelEfficiency()` - Customize efficiency calculations

### Fleet Integration
For production use, modify the fleet integration functions:
- `updateFleetDashboard()` - Send data to your fleet management system
- `sendMaintenanceAlert()` - Integrate with your alerting system

## 📚 Learning Path

1. **Start with the template**: Use the basic `VehicleApp.cpp` template to understand signal subscription
2. **Try FleetManagementSDV**: See comprehensive multi-signal processing
3. **Customize for your use case**: Modify the examples for your specific needs
4. **Test thoroughly**: Use the signal validation testing to ensure reliability

## 🔗 Related Documentation

- [Main README](../README.md) - Complete build system documentation
- [PREBUILT_IMAGES.md](../PREBUILT_IMAGES.md) - Using pre-built Docker images
- [Vehicle Signal Specification](https://covesa.github.io/vehicle_signal_specification/) - VSS reference

## 💡 Tips for Development

- **Use file mounting**: Mount your C++ files instead of piping for better workflow
- **Enable verbose mode**: Use `--verbose` flag to see detailed build output
- **Test with KUKSA**: Always validate signal processing with real KUKSA integration
- **Monitor performance**: Check driver scoring and analytics output
- **Customize thresholds**: Adjust speed limits, fuel warnings, etc. for your use case