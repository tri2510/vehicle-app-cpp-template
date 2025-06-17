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

// Create global Vehicle instance for accessing signals
::vehicle::Vehicle Vehicle;

// ============================================================================
// VEHICLE APP CLASS DEFINITION
// ============================================================================

// ============================================================================
// 🔧 STEP 1: UNDERSTAND THE VEHICLE APP CLASS
// ============================================================================
// This class handles all vehicle signal communication. You don't need to modify
// the class structure, just the methods marked with 🔧 STEP indicators.

/**
 * @brief Vehicle Application Template Class
 * 
 * 🎯 PURPOSE: This class connects to the Vehicle Data Broker and processes signals
 * 
 * 📝 KEY METHODS TO CUSTOMIZE:
 * - onStart(): Choose which vehicle signals to subscribe to
 * - onSignalChanged(): Process the signal data when it changes
 * 
 * 💡 COMMON SIGNALS YOU CAN USE:
 * - Vehicle.Speed (vehicle speed in m/s)
 * - Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature (cabin temp in °C)
 * - Vehicle.Powertrain.Engine.Speed (engine RPM)
 * - Vehicle.Powertrain.FuelSystem.Level (fuel level in %)
 * - Vehicle.CurrentLocation.Latitude/Longitude (GPS coordinates)
 */
class VehicleAppTemplate : public velocitas::VehicleApp {
public:
    VehicleAppTemplate();

protected:
    // ========================================================================
    // 🔧 STEP 2: CHOOSE YOUR VEHICLE SIGNALS (Customize this method)
    // ========================================================================
    /**
     * @brief Called when the app starts - THIS IS WHERE YOU CHOOSE SIGNALS
     * 
     * 🎯 YOUR TASK: Uncomment the signals you want to monitor
     * 
     * 📖 HOW TO USE:
     * 1. Look at the examples below (lines 100-120)  
     * 2. Uncomment the signals you want to use
     * 3. Comment out signals you don't need
     * 
     * 💡 SIGNAL EXAMPLES:
     * - Vehicle.Speed                           → Current speed in m/s
     * - Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature → Cabin temperature
     * - Vehicle.Powertrain.Engine.Speed         → Engine RPM
     * - Vehicle.Powertrain.FuelSystem.Level     → Fuel level percentage
     */
    void onStart() override;

private:
    // ========================================================================
    // 🔧 STEP 3: PROCESS YOUR SIGNAL DATA (Customize this method)
    // ========================================================================
    /**
     * @brief Called when signal data changes - ADD YOUR LOGIC HERE
     * 
     * 🎯 YOUR TASK: Process the incoming signal data
     * 
     * 📖 HOW TO PROCESS SIGNALS:
     * 1. Use reply.get(Vehicle.SignalName)->value() to get signal values
     * 2. Add if/else logic to react to different values
     * 3. Use logger().info() to print messages
     * 
     * 💡 EXAMPLE ACTIONS:
     * - Speed monitoring: Warn if speed > 120 km/h
     * - Temperature alerts: Alert if cabin too hot/cold  
     * - Fuel warnings: Alert when fuel < 20%
     * - Data logging: Save values to file or database
     */
    void onSignalChanged(const velocitas::DataPointReply& reply);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

VehicleAppTemplate::VehicleAppTemplate()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    velocitas::logger().info("🚗 Vehicle App Template starting...");
}

void VehicleAppTemplate::onStart() {
    velocitas::logger().info("🚀 Vehicle App Template starting - setting up signal subscriptions");
    
    // ========================================================================
    // 🔧 STEP 2: SIGNAL SUBSCRIPTION - CHOOSE YOUR SIGNALS HERE
    // ========================================================================
    // 
    // 📖 INSTRUCTIONS:
    // 1. Pick ONE of the examples below (A, B, or C)
    // 2. Uncomment the example you want to use
    // 3. Comment out the examples you don't need
    // 4. Modify the signals in your chosen example
    //
    // ========================================================================
    
    // ------------------------------------------------------------------------
    // 📊 OPTION A: SINGLE SIGNAL MONITORING (Easy - Start Here!)
    // ------------------------------------------------------------------------
    // Subscribe to just one signal - perfect for beginners
    
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSignalChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
        });
    
    // 💡 SINGLE SIGNAL ALTERNATIVES - Replace Vehicle.Speed with any of these:
    // Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature  // Cabin temperature
    // Vehicle.Powertrain.Engine.Speed                    // Engine RPM
    // Vehicle.Powertrain.FuelSystem.Level                // Fuel level %
    // Vehicle.Acceleration.Longitudinal                  // Acceleration
    // Vehicle.CurrentLocation.Latitude                   // GPS latitude
    // Vehicle.CurrentLocation.Longitude                  // GPS longitude
    
    // ------------------------------------------------------------------------
    // 📊 OPTION B: MULTIPLE SIGNALS (Intermediate)
    // ------------------------------------------------------------------------
    // Subscribe to multiple signals at once - more complex but powerful
    
    // UNCOMMENT THE BLOCK BELOW TO USE MULTIPLE SIGNALS:
    /*
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed)
                                               .select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)
                                               .select(Vehicle.Powertrain.FuelSystem.Level)
                                               .build())
        ->onItem([this](auto&& item) { onSignalChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
        });
    */
    
    // ------------------------------------------------------------------------
    // 📊 OPTION C: CUSTOM SIGNAL COMBINATION (Advanced)
    // ------------------------------------------------------------------------
    // Create your own signal combination - customize as needed
    
    // UNCOMMENT AND MODIFY THE BLOCK BELOW FOR CUSTOM SIGNALS:
    /*
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.YourSignalHere)
                                               .select(Vehicle.AnotherSignal)
                                               .build())
        ->onItem([this](auto&& item) { onSignalChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
        });
    */
    
    // ========================================================================
    // 🔧 STEP 2 COMPLETE: Now go to onSignalChanged() method below (line 170)
    // ========================================================================
    
    velocitas::logger().info("✅ Signal subscription completed - waiting for vehicle data...");
}

void VehicleAppTemplate::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        // ====================================================================
        // 🔧 STEP 3: SIGNAL PROCESSING - ADD YOUR LOGIC HERE
        // ====================================================================
        // 
        // 📖 INSTRUCTIONS:
        // 1. Choose the processing example that matches your Step 2 choice
        // 2. Uncomment the example you want to use
        // 3. Modify the logic to fit your needs
        // 4. Add your own custom actions
        //
        // ====================================================================
        
        // --------------------------------------------------------------------
        // 📊 OPTION A: PROCESS SINGLE SIGNAL (matches Step 2 Option A)
        // --------------------------------------------------------------------
        // Process the speed signal (or whatever single signal you chose)
        
        auto speedValue = reply.get(Vehicle.Speed)->value();
        velocitas::logger().info("📊 Vehicle Speed: {:.2f} m/s ({:.1f} km/h)", 
                                speedValue, speedValue * 3.6);
        
        // 🎯 ADD YOUR SPEED-BASED LOGIC HERE:
        // Example: Speed monitoring with alerts
        if (speedValue > 30.0) {  // 30 m/s = 108 km/h
            velocitas::logger().warn("⚠️  HIGH SPEED ALERT: {:.1f} km/h - Slow down!", speedValue * 3.6);
        } else if (speedValue > 20.0) {  // 20 m/s = 72 km/h
            velocitas::logger().info("🚗 Normal highway speed: {:.1f} km/h", speedValue * 3.6);
        } else if (speedValue > 5.0) {  // 5 m/s = 18 km/h
            velocitas::logger().info("🏘️  City driving speed: {:.1f} km/h", speedValue * 3.6);
        } else if (speedValue > 0.1) {
            velocitas::logger().info("🚶 Very slow: {:.1f} km/h", speedValue * 3.6);
        } else {
            velocitas::logger().info("🛑 Vehicle stopped");
        }
        
        // 💡 REPLACE THE ABOVE WITH YOUR OWN LOGIC:
        // - Send alerts to a mobile app
        // - Log data to a database
        // - Control other vehicle systems
        // - Calculate fuel efficiency
        
        // --------------------------------------------------------------------
        // 📊 OPTION B: PROCESS MULTIPLE SIGNALS (matches Step 2 Option B)
        // --------------------------------------------------------------------
        // UNCOMMENT THE BLOCK BELOW IF YOU CHOSE MULTIPLE SIGNALS IN STEP 2:
        
        /*
        // Process speed signal
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            auto speed = reply.get(Vehicle.Speed)->value();
            velocitas::logger().info("🚗 Speed: {:.1f} km/h", speed * 3.6);
        }
        
        // Process cabin temperature signal
        if (reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->isAvailable()) {
            auto temp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->value();
            velocitas::logger().info("🌡️  Cabin Temp: {:.1f}°C", temp);
            
            // 🎯 ADD YOUR TEMPERATURE LOGIC HERE:
            if (temp > 28.0) {
                velocitas::logger().warn("🔥 Cabin too hot! Consider turning on AC");
            } else if (temp < 16.0) {
                velocitas::logger().warn("🧊 Cabin too cold! Consider turning on heater");
            } else {
                velocitas::logger().info("✅ Cabin temperature is comfortable");
            }
        }
        
        // Process fuel level signal
        if (reply.get(Vehicle.Powertrain.FuelSystem.Level)->isAvailable()) {
            auto fuel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
            velocitas::logger().info("⛽ Fuel Level: {:.1f}%", fuel);
            
            // 🎯 ADD YOUR FUEL LOGIC HERE:
            if (fuel < 15.0) {
                velocitas::logger().warn("⚠️  LOW FUEL WARNING: {:.1f}% - Find a gas station!", fuel);
            } else if (fuel < 30.0) {
                velocitas::logger().info("⚠️  Fuel getting low: {:.1f}%", fuel);
            }
        }
        */
        
        // --------------------------------------------------------------------
        // 📊 OPTION C: CUSTOM SIGNAL PROCESSING (matches Step 2 Option C)
        // --------------------------------------------------------------------
        // UNCOMMENT AND MODIFY THE BLOCK BELOW FOR YOUR CUSTOM SIGNALS:
        
        /*
        if (reply.get(Vehicle.YourSignalHere)->isAvailable()) {
            auto value = reply.get(Vehicle.YourSignalHere)->value();
            velocitas::logger().info("📈 Your Signal: {}", value);
            
            // 🎯 ADD YOUR CUSTOM LOGIC HERE:
            // Process your specific signal data
            // Add conditions, calculations, alerts, etc.
        }
        */
        
        // ====================================================================
        // 🎯 CHALLENGE: ADD YOUR OWN CUSTOM LOGIC BELOW
        // ====================================================================
        // Ideas for custom logic:
        // - Combine multiple signals for complex decisions
        // - Track signal changes over time
        // - Send data to external systems (MQTT, databases, APIs)
        // - Calculate derived metrics (fuel efficiency, trip distance, etc.)
        // - Implement safety features (collision avoidance, driver alerts)
        // ====================================================================
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Waiting for vehicle signal data...");
    }
}

// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================

std::unique_ptr<VehicleAppTemplate> myApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 App terminated due to signal {}", sig);
    if (myApp) {
        myApp->stop();
    }
}

// ============================================================================
// 🔧 STEP 4: OPTIONAL CUSTOMIZATIONS (Advanced users only)
// ============================================================================
/**
 * @brief Main application entry point
 * 
 * 📖 WHAT THIS DOES:
 * 1. Handles Ctrl+C to shut down gracefully
 * 2. Starts your vehicle application
 * 3. Catches and reports any errors
 * 
 * 🎯 MOST USERS DON'T NEED TO MODIFY THIS SECTION
 * Only modify if you need:
 * - Command-line arguments
 * - Environment variables
 * - Configuration files
 */
int main(int argc, char** argv) {
    // Handle Ctrl+C for clean shutdown
    signal(SIGINT, signal_handler);

    // ========================================================================
    // 🔧 STEP 4 (OPTIONAL): ADVANCED INITIALIZATION
    // ========================================================================
    // UNCOMMENT AND MODIFY IF YOU NEED ADVANCED FEATURES:
    
    /*
    // Example: Handle command-line arguments
    if (argc > 1) {
        velocitas::logger().info("📁 Using config file: {}", argv[1]);
        // Load your configuration file here
    }
    
    // Example: Read environment variables
    const char* logLevel = std::getenv("LOG_LEVEL");
    if (logLevel) {
        velocitas::logger().info("📝 Log level set to: {}", logLevel);
        // Set your logging level here
    }
    
    const char* deviceId = std::getenv("DEVICE_ID");
    if (deviceId) {
        velocitas::logger().info("🆔 Device ID: {}", deviceId);
        // Use device ID for identification
    }
    */
    
    // ========================================================================

    velocitas::logger().info("🚀 Starting your Vehicle Application...");
    velocitas::logger().info("💡 Press Ctrl+C to stop the application");

    // Create and run your vehicle application
    myApp = std::make_unique<VehicleAppTemplate>();
    try {
        myApp->run();  // This runs until you press Ctrl+C
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }

    velocitas::logger().info("👋 Vehicle Application stopped");
    return 0;
}

// ============================================================================
// 🎓 LEARNING EXAMPLES & REFERENCE
// ============================================================================
//
// 🚀 QUICK REFERENCE: Common Vehicle Signals
// ============================================================================
//
// 📊 SPEED & MOVEMENT:
// Vehicle.Speed                           → Speed in m/s (multiply by 3.6 for km/h)
// Vehicle.Acceleration.Longitudinal       → Forward/backward acceleration in m/s²
// Vehicle.Acceleration.Lateral            → Left/right acceleration in m/s²
//
// 🌡️ CLIMATE & COMFORT:
// Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature    → Left front temperature in °C
// Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature   → Right front temperature in °C
// Vehicle.Cabin.HVAC.IsAirConditioningActive          → AC on/off status
//
// ⛽ ENGINE & FUEL:
// Vehicle.Powertrain.Engine.Speed         → Engine RPM
// Vehicle.Powertrain.FuelSystem.Level     → Fuel level in percentage (0-100%)
// Vehicle.Powertrain.Engine.IsRunning     → Engine on/off status
//
// 📍 LOCATION & NAVIGATION:
// Vehicle.CurrentLocation.Latitude        → GPS latitude
// Vehicle.CurrentLocation.Longitude       → GPS longitude
// Vehicle.CurrentLocation.Altitude        → Altitude in meters
//
// 🔧 VEHICLE STATUS:
// Vehicle.Body.Lights.IsHeadlightOn       → Headlight status
// Vehicle.Body.Doors.Row1.Left.IsOpen     → Front left door status
// Vehicle.Chassis.Brake.PedalPosition     → Brake pedal position (0-100%)
//
// ============================================================================
// 🎯 COPY-PASTE EXAMPLES FOR COMMON USE CASES
// ============================================================================
//
// 📖 Example 1: CABIN TEMPERATURE MONITOR
// Step 2: Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature
// Step 3: auto temp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->value();
//         if (temp > 25.0) logger().warn("Too hot!"); 
//
// 📖 Example 2: ENGINE RPM TRACKER
// Step 2: Vehicle.Powertrain.Engine.Speed
// Step 3: auto rpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
//         if (rpm > 3000) logger().warn("High RPM!");
//
// 📖 Example 3: FUEL LEVEL ALERT
// Step 2: Vehicle.Powertrain.FuelSystem.Level
// Step 3: auto fuel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
//         if (fuel < 20.0) logger().warn("Low fuel: {:.1f}%", fuel);
//
// 📖 Example 4: GPS LOCATION LOGGER
// Step 2: .select(Vehicle.CurrentLocation.Latitude).select(Vehicle.CurrentLocation.Longitude)
// Step 3: auto lat = reply.get(Vehicle.CurrentLocation.Latitude)->value();
//         auto lon = reply.get(Vehicle.CurrentLocation.Longitude)->value();
//         logger().info("Location: {:.6f}, {:.6f}", lat, lon);
//
// 📖 Example 5: DOOR STATUS MONITOR
// Step 2: Vehicle.Body.Doors.Row1.Left.IsOpen
// Step 3: auto doorOpen = reply.get(Vehicle.Body.Doors.Row1.Left.IsOpen)->value();
//         if (doorOpen) logger().info("Front left door is open");
//
// ============================================================================
// 🔧 NEXT STEPS:
// 1. Pick one of the examples above
// 2. Copy the Step 2 line to the onStart() method
// 3. Copy the Step 3 lines to the onSignalChanged() method
// 4. Build and test: cat VehicleApp.cpp | docker run --rm -i velocitas-quick
// ============================================================================