/**
 * Copyright (c) 2023-2025 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// ============================================================================
// VEHICLE APP TEMPLATE - Single File Implementation
// ============================================================================
// This is a unified C++ template for vehicle applications.
// Users can modify the marked TEMPLATE AREAS to create custom vehicle apps.
//
// QUICK START:
// 1. Modify signal subscription in onStart() method
// 2. Add custom logic in onSignalChanged() method  
// 3. Build and run: docker run --rm velocitas-template-dev build-app && run-app
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

/**
 * @brief Generic Vehicle Application Template
 * 
 * This template provides a simple framework for vehicle signal processing:
 * - Subscribe to vehicle signals from the Vehicle Data Broker
 * - Process incoming signal data
 * - Log results and perform custom actions
 * 
 * TEMPLATE AREAS are marked with clear comments for easy customization.
 */
class VehicleAppTemplate : public velocitas::VehicleApp {
public:
    VehicleAppTemplate();

protected:
    /**
     * @brief Called when the application starts and VDB connection is ready
     * 
     * 🔧 TEMPLATE AREA: Modify this method to subscribe to different signals
     * 
     * Examples:
     * - Single signal: QueryBuilder::select(Vehicle.Speed)
     * - Multiple signals: QueryBuilder::select(Vehicle.Speed).select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)
     * - Custom signals: QueryBuilder::select(Vehicle.MyCustomSignal.Value)
     */
    void onStart() override;

private:
    /**
     * @brief Handle signal data changes
     * 
     * 🔧 TEMPLATE AREA: Modify this method to process different signal types
     * 
     * Available operations:
     * - Data validation: Check if signal values are reasonable
     * - Calculations: Perform math operations on signal data
     * - Conditional logic: Take actions based on signal values
     * - State tracking: Store and compare previous values
     * - Logging: Output results to console
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
    velocitas::logger().info("🔧 Vehicle App Template ready - setting up signal subscriptions");
    
    // ========================================================================
    // 🔧 TEMPLATE AREA: SIGNAL SUBSCRIPTION
    // ========================================================================
    // Modify the signals you want to subscribe to:
    //
    // SINGLE SIGNAL EXAMPLE:
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSignalChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
        });
    
    // MULTIPLE SIGNALS EXAMPLE (uncomment to use):
    // subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed)
    //                                            .select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)
    //                                            .select(Vehicle.Powertrain.Engine.Speed)
    //                                            .build())
    //     ->onItem([this](auto&& item) { onSignalChanged(std::forward<decltype(item)>(item)); })
    //     ->onError([this](auto&& status) { 
    //         velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
    //     });
    //
    // AVAILABLE SIGNALS (examples):
    // - Vehicle.Speed                                    (vehicle speed in m/s)
    // - Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature (cabin temperature in °C)
    // - Vehicle.Powertrain.Engine.Speed                  (engine RPM)
    // - Vehicle.Acceleration.Longitudinal                (acceleration in m/s²)
    // - Vehicle.CurrentLocation.Latitude                 (GPS latitude)
    // - Vehicle.CurrentLocation.Longitude                (GPS longitude)
    // - Vehicle.Powertrain.FuelSystem.Level              (fuel level %)
    // ========================================================================
    
    velocitas::logger().info("✅ Signal subscription completed");
}

void VehicleAppTemplate::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        // ====================================================================
        // 🔧 TEMPLATE AREA: SIGNAL PROCESSING
        // ====================================================================
        // Modify this section to process different signals and add custom logic:
        //
        // SINGLE SIGNAL PROCESSING EXAMPLE:
        auto speedValue = reply.get(Vehicle.Speed)->value();
        velocitas::logger().info("📊 Received signal: Vehicle.Speed = {:.2f} m/s ({:.1f} km/h)", 
                                speedValue, speedValue * 3.6);
        
        // CUSTOM LOGIC EXAMPLES:
        // 1. Speed limit checking:
        if (speedValue > 25.0) {  // 25 m/s ≈ 90 km/h
            velocitas::logger().warn("⚠️  High speed detected: {:.2f} m/s", speedValue);
        }
        
        // 2. Speed categories:
        if (speedValue < 0.1) {
            velocitas::logger().info("🛑 Vehicle stopped");
        } else if (speedValue < 5.0) {
            velocitas::logger().info("🚶 Low speed: city driving");
        } else if (speedValue < 15.0) {
            velocitas::logger().info("🏘️  Medium speed: suburban driving");
        } else {
            velocitas::logger().info("🛣️  High speed: highway driving");
        }
        
        // MULTIPLE SIGNALS PROCESSING EXAMPLE (uncomment if using multiple signals):
        // if (reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->isAvailable()) {
        //     auto temp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->value();
        //     velocitas::logger().info("🌡️  Cabin temperature: {:.1f}°C", temp);
        //     
        //     if (temp > 25.0) {
        //         velocitas::logger().warn("🔥 High cabin temperature!");
        //     } else if (temp < 18.0) {
        //         velocitas::logger().info("🧊 Cool cabin temperature");
        //     }
        // }
        //
        // if (reply.get(Vehicle.Powertrain.Engine.Speed)->isAvailable()) {
        //     auto rpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
        //     velocitas::logger().info("🔧 Engine RPM: {:.0f}", rpm);
        // }
        
        // ADD YOUR CUSTOM PROCESSING HERE:
        // - Data validation: if (speedValue >= 0 && speedValue <= 100) { ... }
        // - Calculations: double speedKmh = speedValue * 3.6;
        // - State tracking: store previous values and compare changes
        // - Complex logic: combine multiple signals for advanced features
        // ====================================================================
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Waiting for signal data...");
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

/**
 * @brief Main application entry point
 * 
 * This function:
 * 1. Sets up signal handling for graceful shutdown (Ctrl+C)
 * 2. Creates and starts the vehicle application
 * 3. Handles any runtime errors
 * 
 * 🔧 TEMPLATE AREA: Add command-line arguments or initialization here if needed
 */
int main(int argc, char** argv) {
    // Set up graceful shutdown on Ctrl+C
    signal(SIGINT, signal_handler);

    // ========================================================================
    // 🔧 TEMPLATE AREA: INITIALIZATION
    // ========================================================================
    // Add custom initialization here if needed:
    // - Command line argument processing
    // - Environment variable reading
    // - Configuration file loading
    // 
    // EXAMPLES:
    // if (argc > 1) {
    //     velocitas::logger().info("📁 Config file: {}", argv[1]);
    // }
    // 
    // const char* logLevel = std::getenv("LOG_LEVEL");
    // if (logLevel) {
    //     velocitas::logger().info("📝 Log level: {}", logLevel);
    // }
    // ========================================================================

    velocitas::logger().info("🚀 Starting Vehicle App Template...");

    // Create and run the application
    myApp = std::make_unique<VehicleAppTemplate>();
    try {
        myApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 App terminated due to error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 App terminated due to unknown error");
        return 1;
    }

    velocitas::logger().info("👋 Vehicle App Template terminated normally");
    return 0;
}

// ============================================================================
// TEMPLATE USAGE EXAMPLES
// ============================================================================
//
// 1. MONITOR CABIN TEMPERATURE:
//    - Change: QueryBuilder::select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)
//    - Process: auto temp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->value();
//
// 2. TRACK ENGINE STATUS:
//    - Change: QueryBuilder::select(Vehicle.Powertrain.Engine.Speed)
//    - Process: auto rpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
//
// 3. GPS LOCATION TRACKING:
//    - Change: QueryBuilder::select(Vehicle.CurrentLocation.Latitude).select(Vehicle.CurrentLocation.Longitude)
//    - Process: auto lat = reply.get(Vehicle.CurrentLocation.Latitude)->value();
//              auto lon = reply.get(Vehicle.CurrentLocation.Longitude)->value();
//
// 4. FUEL MONITORING:
//    - Change: QueryBuilder::select(Vehicle.Powertrain.FuelSystem.Level)
//    - Process: auto fuel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
//              if (fuel < 20.0) { logger().warn("Low fuel!"); }
//
// For custom VSS signals, update AppManifest.json and regenerate with: gen-model
// ============================================================================