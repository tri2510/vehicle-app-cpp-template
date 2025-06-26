// ============================================================================
// 🚗 VELOCITAS VEHICLE APP TEMPLATE - STEP BY STEP GUIDE
// ============================================================================
// 
// 📋 WHAT THIS TEMPLATE DOES:
// This template shows you how to create a vehicle application that:
// - Connects to Vehicle Data Broker (VDB) to read vehicle signals
// - Processes real-time vehicle data (speed, temperature, fuel, etc.)
// - Logs information and performs custom actions based on signal values
//
// 🎯 QUICK START (3 Steps):
// 1. Choose your signals in the onStart() method (lines 90-120)
// 2. Add your custom logic in onSignalChanged() method (lines 130-180)
// 3. Build: cat VehicleApp.cpp | docker run --rm -i velocitas-quick
//
// 💡 TIP: Look for 🔧 STEP markers throughout this file for guidance
// ============================================================================

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include "vehicle/Vehicle.hpp"
#include <fmt/format.h>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

// Global Vehicle instance for accessing vehicle signals
::vehicle::Vehicle Vehicle;

/**
 * @brief Your Vehicle Application class
 * 
 * 🔧 STEP 1: This is your main application class
 * You can rename it to match your specific use case (e.g., SpeedMonitorApp, FleetManagerApp)
 */
class VehicleApp : public velocitas::VehicleApp {
public:
    VehicleApp();

protected:
    void onStart() override;

private:
    void onSignalChanged(const velocitas::DataPointReply& reply);
    
    // Add your custom methods here
    void processSpeedSignal(double speed);
    void processTemperatureSignal(double temperature);
    void processFuelSignal(double fuelLevel);
    void processLocationSignal(double latitude, double longitude);
};

// ============================================================================
// 🔧 STEP 2: Constructor - Initialize your application
// ============================================================================
VehicleApp::VehicleApp()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🚗 Vehicle Application starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("✅ Application initialized successfully");
}

// ============================================================================
// 🔧 STEP 3: Choose which vehicle signals to monitor
// ============================================================================
void VehicleApp::onStart() {
    velocitas::logger().info("🚀 Vehicle Application started!");
    velocitas::logger().info("📊 Setting up signal subscriptions...");
    
    // Give the databroker connection time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🔧 STEP 3A: Subscribe to vehicle signals
    // Uncomment the signals you want to monitor:
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Speed)                           // Vehicle speed
            //.select(Vehicle.Powertrain.Engine.ECT)                             // Engine temperature
            //.select(Vehicle.Powertrain.FuelSystem.Level)                       // Fuel level
            //.select(Vehicle.CurrentLocation.Latitude)                          // GPS latitude
            //.select(Vehicle.CurrentLocation.Longitude)                         // GPS longitude
            //.select(Vehicle.Powertrain.Engine.Speed)                           // Engine RPM
            //.select(Vehicle.Body.Lights.Hazard.IsSignaling)                    // Hazard lights
            //.select(Vehicle.ADAS.CruiseControl.IsActive)                       // Cruise control
            //.select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)          // Cabin temperature
            //.select(Vehicle.Powertrain.Transmission.CurrentGear)               // Current gear
            .build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Signal subscriptions completed");
    velocitas::logger().info("🔄 Waiting for vehicle signals...");
    velocitas::logger().info("💡 You can test with: echo 'setValue Vehicle.Speed 25.0' | kuksa-client");
}

// ============================================================================
// 🔧 STEP 4: Process incoming vehicle signals
// ============================================================================
void VehicleApp::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        velocitas::logger().info("📡 Received vehicle signal update");
        
        // 🔧 STEP 4A: Process Vehicle.Speed signal
        if (reply.get(Vehicle.Speed)->isValid()) {
            double speed = reply.get(Vehicle.Speed)->value();
            processSpeedSignal(speed);
        }
        
        // 🔧 STEP 4B: Add more signal processing here
        // Uncomment and modify based on your subscribed signals:
        
        /*
        // Process engine temperature
        if (reply.get(Vehicle.Powertrain.Engine.ECT)->isValid()) {
            double temperature = reply.get(Vehicle.Powertrain.Engine.ECT)->value();
            processTemperatureSignal(temperature);
        }
        
        // Process fuel level
        if (reply.get(Vehicle.Powertrain.FuelSystem.Level)->isValid()) {
            double fuelLevel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
            processFuelSignal(fuelLevel);
        }
        
        // Process GPS location
        if (reply.get(Vehicle.CurrentLocation.Latitude)->isValid() && 
            reply.get(Vehicle.CurrentLocation.Longitude)->isValid()) {
            double latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
            double longitude = reply.get(Vehicle.CurrentLocation.Longitude)->value();
            processLocationSignal(latitude, longitude);
        }
        */
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Waiting for complete signal data...");
    }
}

// ============================================================================
// 🔧 STEP 5: Implement your custom signal processing logic
// ============================================================================

void VehicleApp::processSpeedSignal(double speed) {
    // Convert m/s to km/h for easier understanding
    double speedKmh = speed * 3.6;
    
    velocitas::logger().info("🚗 Vehicle Speed: {:.1f} km/h ({:.2f} m/s)", speedKmh, speed);
    
    // 🔧 STEP 5A: Add your speed-based logic here
    if (speedKmh > 120) {
        velocitas::logger().warn("⚠️  HIGH SPEED WARNING: {:.1f} km/h", speedKmh);
        // Add your high-speed actions here:
        // - Send alert to fleet management
        // - Log safety event
        // - Trigger dashboard warning
        
    } else if (speedKmh > 80) {
        velocitas::logger().info("🛣️  Highway speed: {:.1f} km/h", speedKmh);
        // Add your highway driving logic here
        
    } else if (speedKmh > 30) {
        velocitas::logger().info("🏘️  City driving: {:.1f} km/h", speedKmh);
        // Add your city driving logic here
        
    } else if (speedKmh > 0) {
        velocitas::logger().info("🐌 Slow speed: {:.1f} km/h", speedKmh);
        // Add your slow speed logic here
        
    } else {
        velocitas::logger().info("🛑 Vehicle stopped");
        // Add your stopped vehicle logic here
    }
}

void VehicleApp::processTemperatureSignal(double temperature) {
    velocitas::logger().info("🌡️  Engine Temperature: {:.1f}°C", temperature);
    
    // 🔧 STEP 5B: Add your temperature monitoring logic
    if (temperature > 100) {
        velocitas::logger().error("🔥 ENGINE OVERHEATING: {:.1f}°C", temperature);
        // Add emergency actions here
    } else if (temperature > 90) {
        velocitas::logger().warn("⚠️  Engine running hot: {:.1f}°C", temperature);
        // Add preventive actions here
    } else {
        velocitas::logger().info("✅ Engine temperature normal: {:.1f}°C", temperature);
    }
}

void VehicleApp::processFuelSignal(double fuelLevel) {
    velocitas::logger().info("⛽ Fuel Level: {:.1f}%", fuelLevel);
    
    // 🔧 STEP 5C: Add your fuel monitoring logic
    if (fuelLevel < 10) {
        velocitas::logger().error("🚨 CRITICAL: Low fuel {:.1f}%", fuelLevel);
        // Add critical fuel actions here
    } else if (fuelLevel < 25) {
        velocitas::logger().warn("⚠️  Low fuel warning: {:.1f}%", fuelLevel);
        // Add low fuel actions here
    } else {
        velocitas::logger().info("✅ Fuel level good: {:.1f}%", fuelLevel);
    }
}

void VehicleApp::processLocationSignal(double latitude, double longitude) {
    velocitas::logger().info("📍 Location: {:.6f}, {:.6f}", latitude, longitude);
    
    // 🔧 STEP 5D: Add your location-based logic
    // Examples:
    // - Geofencing alerts
    // - Route optimization
    // - Location-based services
    // - Fleet tracking
}

// ============================================================================
// 🔧 STEP 6: Application entry point and signal handling
// ============================================================================

std::unique_ptr<VehicleApp> vehicleApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down vehicle application (signal: {})", signal);
    if (vehicleApp) {
        vehicleApp->stop();
    }
}

int main(int argc, char** argv) {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🚀 Starting Vehicle Application...");
    velocitas::logger().info("🎯 Ready to process vehicle signals");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        // Create and start the vehicle application
        vehicleApp = std::make_unique<VehicleApp>();
        vehicleApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Vehicle application stopped");
    return 0;
}

// ============================================================================
// 🎓 NEXT STEPS & CUSTOMIZATION IDEAS
// ============================================================================
//
// 🔧 ADVANCED CUSTOMIZATION:
//
// 1. **Add More Signals**: Uncomment more signals in onStart() method
//    - Engine data: RPM, torque, fuel consumption
//    - Safety systems: ABS, airbag status, seatbelt status
//    - Comfort: Climate control, seat positions, entertainment
//    - ADAS: Cruise control, lane assist, collision warning
//
// 2. **Add Data Storage**: Store signals in database or files
//    - SQLite for local storage
//    - InfluxDB for time-series data
//    - JSON files for simple logging
//
// 3. **Add External Communication**:
//    - MQTT for fleet management
//    - REST APIs for cloud services
//    - CAN bus integration
//    - Mobile app notifications
//
// 4. **Add Business Logic**:
//    - Predictive maintenance alerts
//    - Driver behavior scoring
//    - Fuel efficiency optimization
//    - Route planning and optimization
//    - Emergency response automation
//
// 5. **Add Configuration**:
//    - JSON config files
//    - Environment variables
//    - Command line parameters
//    - Remote configuration updates
//
// 6. **Add Testing**:
//    - Unit tests for signal processing
//    - Integration tests with mock data
//    - Performance testing
//    - Signal simulation for development
//
// ============================================================================
// 🚀 BUILD & RUN INSTRUCTIONS:
//
// 🚀 OPTIMIZED BUILD (15-30s - Recommended for development):
//   docker run --rm --network host \
//     -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
//     -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
//     -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
//     velocitas-quick build --skip-deps --skip-vss --verbose
//
// Traditional Build (60-90s):
//   docker run --rm --network host \
//     -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
//     -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
//     velocitas-quick build --verbose
//
// 🔄 BUILD AND RUN (Single Container - Recommended):
//   docker run -d --network host --name vehicle-app \
//     -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
//     -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
//     -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
//     velocitas-quick bash -c 'build --skip-deps --skip-vss && run 300'
//
// 📡 Test with Signals:
//   echo "setValue Vehicle.Speed 65.0" | \
//   docker run --rm -i --network host \
//     ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
//
// 🎯 Performance Benefits:
//   --skip-deps: Skip dependency installation (3-4x faster)
//   --skip-vss: Skip VSS generation (additional time savings)
//   Combined: 60-90s → 15-30s build time
//
// ============================================================================