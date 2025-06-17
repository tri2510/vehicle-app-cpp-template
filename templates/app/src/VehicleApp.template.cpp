// ============================================================================
// VELOCITAS C++ VEHICLE APP TEMPLATE - Quick Build Guide
// ============================================================================
// This template shows how to create a Velocitas C++ vehicle application.
// Copy this template and modify it to create your own vehicle app.
//
// QUICK BUILD USAGE:
//   cat YourVehicleApp.cpp | docker run --rm -i velocitas-quick
//   docker run --rm -v $(pwd)/YourApp.cpp:/input velocitas-quick
// ============================================================================

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include "vehicle/Vehicle.hpp"
#include <fmt/format.h>
#include <memory>

// ============================================================================
// VEHICLE SIGNALS REFERENCE - Available signals you can use:
// ============================================================================
//
// BASIC VEHICLE DATA:
//   Vehicle.Speed                           - Current vehicle speed (m/s)
//   Vehicle.TraveledDistance               - Total distance traveled (m)
//   Vehicle.Acceleration.Longitudinal      - Forward/backward acceleration (m/s²)
//   Vehicle.Acceleration.Lateral           - Left/right acceleration (m/s²)
//   Vehicle.Acceleration.Vertical          - Up/down acceleration (m/s²)
//
// ENGINE & POWERTRAIN:
//   Vehicle.Powertrain.Engine.Speed        - Engine RPM
//   Vehicle.Powertrain.Engine.Power        - Engine power (kW)
//   Vehicle.Powertrain.FuelSystem.Level    - Fuel level (%)
//   Vehicle.Powertrain.FuelSystem.Range    - Remaining range (m)
//   Vehicle.Powertrain.Transmission.Gear   - Current gear
//
// BODY & COMFORT:
//   Vehicle.Body.Lights.Hazard.IsOn        - Hazard lights status
//   Vehicle.Body.Horn.IsActive             - Horn activation
//   Vehicle.Cabin.Door.Row1.Left.IsOpen    - Door status
//   Vehicle.Cabin.Seat.Row1.Pos1.IsOccupied - Seat occupancy
//   Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature - Cabin temperature
//
// ADAS & SAFETY:
//   Vehicle.ADAS.ObstacleDetection.Front.Distance - Front obstacle distance (m)
//   Vehicle.ADAS.ObstacleDetection.Rear.Distance  - Rear obstacle distance (m)
//   Vehicle.ADAS.ABS.IsActive              - ABS system status
//   Vehicle.ADAS.ESC.IsActive              - Electronic stability control
//
// LOCATION & NAVIGATION:
//   Vehicle.CurrentLocation.Latitude       - GPS latitude
//   Vehicle.CurrentLocation.Longitude      - GPS longitude
//   Vehicle.CurrentLocation.Altitude       - GPS altitude (m)
//
// DIAGNOSTICS:
//   Vehicle.OBD.EngineLoad                 - Engine load percentage
//   Vehicle.OBD.DTCs                       - Diagnostic trouble codes
//   Vehicle.Service.DistanceToService      - Distance until next service (m)
//
// ============================================================================

// Create global Vehicle instance for accessing signals
::vehicle::Vehicle Vehicle;

// ============================================================================
// YOUR VEHICLE APP CLASS - Customize this for your application
// ============================================================================

class YourVehicleApp : public velocitas::VehicleApp {
public:
    YourVehicleApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {}

protected:
    void onStart() override {
        velocitas::logger().info("🚗 Starting Your Vehicle App");
        
        // ====================================================================
        // STEP 1: SUBSCRIBE TO VEHICLE SIGNALS
        // ====================================================================
        // Subscribe to signals you want to monitor
        // Replace these with the signals your app needs:
        
        Vehicle.Speed.subscribe(velocitas::QueryBuilder::select(Vehicle.Speed).build());
        Vehicle.Acceleration.Longitudinal.subscribe(velocitas::QueryBuilder::select(Vehicle.Acceleration.Longitudinal).build());
        Vehicle.Powertrain.FuelSystem.Level.subscribe(velocitas::QueryBuilder::select(Vehicle.Powertrain.FuelSystem.Level).build());
        
        // More subscription examples:
        // Vehicle.Powertrain.Engine.Speed.subscribe(velocitas::QueryBuilder::select(Vehicle.Powertrain.Engine.Speed).build());
        // Vehicle.ADAS.ObstacleDetection.Front.Distance.subscribe(velocitas::QueryBuilder::select(Vehicle.ADAS.ObstacleDetection.Front.Distance).build());
        // Vehicle.CurrentLocation.Latitude.subscribe(velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Latitude).build());
        
        velocitas::logger().info("✅ Signal subscriptions configured");
        
        // ====================================================================
        // STEP 2: SET UP MQTT COMMUNICATION (Optional)
        // ====================================================================
        // If you need MQTT communication, initialize it here:
        // subscribeToTopic("yourapp/config");
        // publishToTopic("yourapp/status", "started");
        
        velocitas::logger().info("🚀 Your Vehicle App is ready!");
    }

    // ========================================================================
    // STEP 3: HANDLE SIGNAL CHANGES
    // ========================================================================
    // This method is called whenever subscribed signals change
    void onVehicleDataChanged(const velocitas::VehicleDataResponse& response) override {
        
        // EXAMPLE 1: Handle speed changes
        if (response.hasValue(Vehicle.Speed)) {
            float currentSpeed = response.getValue(Vehicle.Speed).value();
            velocitas::logger().info("Speed updated: {:.1f} m/s ({:.1f} km/h)", 
                                   currentSpeed, currentSpeed * 3.6);
            
            // Your custom logic here:
            if (currentSpeed > 33.33) {  // > 120 km/h
                velocitas::logger().warn("⚠️ High speed detected!");
                // Add your high-speed logic here
            }
        }
        
        // EXAMPLE 2: Handle acceleration changes
        if (response.hasValue(Vehicle.Acceleration.Longitudinal)) {
            float acceleration = response.getValue(Vehicle.Acceleration.Longitudinal).value();
            velocitas::logger().info("Acceleration: {:.2f} m/s²", acceleration);
            
            // Your custom logic here:
            if (acceleration < -6.0) {  // Hard braking
                velocitas::logger().warn("🛑 Hard braking detected!");
                // Add your hard braking logic here
            }
        }
        
        // EXAMPLE 3: Handle fuel level changes
        if (response.hasValue(Vehicle.Powertrain.FuelSystem.Level)) {
            float fuelLevel = response.getValue(Vehicle.Powertrain.FuelSystem.Level).value();
            velocitas::logger().info("Fuel level: {:.1f}%", fuelLevel);
            
            // Your custom logic here:
            if (fuelLevel < 10.0) {  // Low fuel warning
                velocitas::logger().warn("⛽ Low fuel warning!");
                // Add your low fuel logic here
            }
        }
        
        // ADD YOUR SIGNAL HANDLING HERE:
        // Copy the pattern above for other signals you subscribed to
    }

    // ========================================================================
    // STEP 4: ADD YOUR CUSTOM METHODS
    // ========================================================================
    
    // Example: Method to set vehicle signals (if needed)
    void setHazardLights(bool enabled) {
        Vehicle.Body.Lights.Hazard.IsOn.set(enabled);
        velocitas::logger().info("Hazard lights: {}", enabled ? "ON" : "OFF");
    }
    
    // Example: Method to publish MQTT messages (if MQTT is configured)
    void sendAlert(const std::string& message) {
        // publishToTopic("yourapp/alerts", message);
        velocitas::logger().info("Alert: {}", message);
    }
    
    // Add your custom methods here...
};

// ============================================================================
// CONFIGURATION EXAMPLES FOR COMMON USE CASES
// ============================================================================
//
// 1. SPEED MONITORING APP:
//    Subscribe to: Vehicle.Speed, Vehicle.Acceleration.Longitudinal
//    Logic: Monitor speed limits, detect hard braking/acceleration
//
// 2. FUEL EFFICIENCY TRACKER:
//    Subscribe to: Vehicle.Speed, Vehicle.Powertrain.FuelSystem.Level, 
//                  Vehicle.Powertrain.Engine.Speed
//    Logic: Calculate fuel consumption, provide efficiency tips
//
// 3. PARKING ASSISTANT:
//    Subscribe to: Vehicle.ADAS.ObstacleDetection.*.Distance, Vehicle.Speed
//    Logic: Provide parking guidance based on proximity sensors
//
// 4. MAINTENANCE REMINDER:
//    Subscribe to: Vehicle.OBD.EngineLoad, Vehicle.Service.DistanceToService,
//                  Vehicle.TraveledDistance
//    Logic: Monitor engine health, track service intervals
//
// 5. CLIMATE CONTROL OPTIMIZER:
//    Subscribe to: Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature,
//                  Vehicle.Exterior.AirTemperature
//    Logic: Automatically adjust cabin temperature for comfort and efficiency
//
// ============================================================================
// VSS CUSTOMIZATION
// ============================================================================
// To use custom VSS specifications:
//
// 1. Mount custom VSS file:
//    docker run -v $(pwd)/custom-vss.json:/vss.json \
//      -e VSS_SPEC_FILE=/vss.json -i velocitas-quick < YourApp.cpp
//
// 2. Use custom VSS URL:
//    docker run -e VSS_SPEC_URL=https://company.com/vss.json \
//      -i velocitas-quick < YourApp.cpp
//
// ============================================================================

// Register your vehicle app with the Velocitas framework
DEFINE_VEHICLE_APP(YourVehicleApp)