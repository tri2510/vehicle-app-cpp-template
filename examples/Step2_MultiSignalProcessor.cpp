// ============================================================================
// 🎓 STEP 2: MULTI-SIGNAL PROCESSOR - Handle Multiple Vehicle Data Streams
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this tutorial step, you will learn:
// - How to subscribe to multiple vehicle signals simultaneously
// - Process different data types (speed, RPM, fuel level)
// - Correlate data from multiple sources for insights
// - Implement efficient signal processing patterns
//
// 🎯 WHAT YOU'LL BUILD:
// A multi-signal processing application that:
// - Monitors speed, engine RPM, and fuel level together
// - Calculates fuel efficiency based on speed and consumption
// - Detects driving patterns from combined signals
// - Provides comprehensive vehicle status reporting
//
// 💡 DIFFICULTY: ⭐⭐ Intermediate (30 minutes)
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
#include <cmath>

// Global Vehicle instance for accessing vehicle signals
::vehicle::Vehicle Vehicle;

/**
 * @brief Step 2: Multi-Signal Processor Application
 * 
 * 🎓 TUTORIAL: This application demonstrates processing multiple
 * vehicle signals together. You'll learn:
 * - Simultaneous signal subscriptions
 * - Data correlation and analysis
 * - Efficient multi-signal handling
 * - Composite vehicle state tracking
 */
class MultiSignalProcessor : public velocitas::VehicleApp {
public:
    MultiSignalProcessor();

protected:
    void onStart() override;

private:
    // Signal processing methods
    void onSignalChanged(const velocitas::DataPointReply& reply);
    void updateVehicleStatus();
    void analyzeDrivingPattern();
    void calculateFuelEfficiency();
    std::string getDrivingMode();
    
    // 🎓 LEARNING POINT: Vehicle State Structure
    // Track multiple signals in a structured way
    struct VehicleState {
        double speed = 0.0;           // Vehicle speed in m/s
        double latitude = 0.0;        // GPS Latitude
        double longitude = 0.0;       // GPS Longitude
        double engineRpm = 0.0;       // Simulated Engine RPM (calculated)
        double fuelLevel = 75.0;      // Simulated Fuel level percentage
        bool dataValid = false;       // Are all signals valid?
        std::chrono::steady_clock::time_point lastUpdate;
    };
    
    // 🎓 LEARNING POINT: Driving Statistics
    // Calculate derived metrics from multiple signals
    struct DrivingStats {
        double avgSpeed = 0.0;
        double avgRpm = 0.0;
        double fuelEfficiency = 0.0;  // L/100km
        int dataPoints = 0;
        std::string drivingMode = "UNKNOWN";
    };
    
    VehicleState m_vehicleState;
    DrivingStats m_drivingStats;
    
    // Helper methods
    double calculateInstantFuelConsumption(double speed, double rpm);
    void logVehicleStatus();
};

// ============================================================================
// 🎓 STEP 2A: Multi-Signal Application Initialization
// ============================================================================
MultiSignalProcessor::MultiSignalProcessor()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 2: Multi-Signal Processor starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("🚗 Learning objective: Process multiple vehicle signals");
    velocitas::logger().info("📊 Signals: Speed, GPS Location + Simulated RPM/Fuel");
    velocitas::logger().info("✅ Multi-Signal Processor initialized");
    
    // Initialize state
    m_vehicleState.lastUpdate = std::chrono::steady_clock::now();
}

// ============================================================================
// 🎓 STEP 2B: Multiple Signal Subscription Setup
// ============================================================================
void MultiSignalProcessor::onStart() {
    velocitas::logger().info("🚀 Step 2: Starting Multi-Signal Processor!");
    velocitas::logger().info("📊 Setting up multiple signal subscriptions...");
    
    // Give the databroker connection time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🎓 LEARNING POINT: Multiple Signal Subscription
    // You can subscribe to multiple signals in a single query:
    // 1. Chain multiple .select() calls
    // 2. Each signal can be different data types
    // 3. All signals arrive in the same callback
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Speed)                      // Speed in m/s - testing single signal like Step 1
            // .select(Vehicle.CurrentLocation.Latitude)                      // GPS Latitude  
            // .select(Vehicle.CurrentLocation.Longitude)                     // GPS Longitude
            .build()
    )
    ->onItem([this](auto&& item) { 
        // 🎓 LEARNING POINT: Unified Callback
        // All subscribed signals arrive here together
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Multi-signal subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Multi-signal subscriptions completed");
    velocitas::logger().info("🔄 Monitoring: Speed + GPS Location (RPM/Fuel simulated)");
    velocitas::logger().info("💡 Test with multiple signals:");
    velocitas::logger().info("   echo 'setValue Vehicle.Speed 25.0' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.CurrentLocation.Latitude 40.7589' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.CurrentLocation.Longitude -73.9851' | kuksa-client");
}

// ============================================================================
// 🎓 STEP 2C: Multi-Signal Data Processing
// ============================================================================
void MultiSignalProcessor::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        velocitas::logger().info("📡 Received multi-signal update");
        
        // 🎓 LEARNING POINT: Processing Multiple Signals
        // Check and update each signal individually
        // Some signals might be valid while others aren't
        
        bool anyUpdate = false;
        
        // Process Vehicle Speed (using Step 1 pattern)
        try {
            if (reply.get(Vehicle.Speed)->isValid()) {
                m_vehicleState.speed = reply.get(Vehicle.Speed)->value();
                anyUpdate = true;
                velocitas::logger().info("   Speed: {:.2f} m/s", m_vehicleState.speed);
            } else {
                velocitas::logger().debug("   ⏳ Waiting for valid Vehicle.Speed data...");
            }
        } catch (const std::exception& e) {
            velocitas::logger().debug("   Speed signal not available: {}", e.what());
        }
        
        // Process GPS Latitude
        try {
            if (reply.get(Vehicle.CurrentLocation.Latitude)->isValid()) {
                m_vehicleState.latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
                anyUpdate = true;
                velocitas::logger().info("   Latitude: {:.6f}", m_vehicleState.latitude);
            } else {
                velocitas::logger().debug("   ⏳ Waiting for valid Latitude data...");
            }
        } catch (const std::exception& e) {
            velocitas::logger().debug("   Latitude signal not available: {}", e.what());
        }
        
        // Process GPS Longitude
        try {
            if (reply.get(Vehicle.CurrentLocation.Longitude)->isValid()) {
                m_vehicleState.longitude = reply.get(Vehicle.CurrentLocation.Longitude)->value();
                anyUpdate = true;
                velocitas::logger().info("   Longitude: {:.6f}", m_vehicleState.longitude);
            } else {
                velocitas::logger().debug("   ⏳ Waiting for valid Longitude data...");
            }
        } catch (const std::exception& e) {
            velocitas::logger().debug("   Longitude signal not available: {}", e.what());
        }
        
        // 🎓 LEARNING POINT: State Management
        // Update state only when we have new data
        if (anyUpdate) {
            m_vehicleState.lastUpdate = std::chrono::steady_clock::now();
            m_vehicleState.dataValid = true;
            
            // Process the combined state
            updateVehicleStatus();
        } else {
            velocitas::logger().debug("📡 No valid signals in this update");
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("📡 Exception processing signals: {}", e.what());
    }
}

// ============================================================================
// 🎓 STEP 2D: Combined Signal Analysis
// ============================================================================
void MultiSignalProcessor::updateVehicleStatus() {
    // Convert units for display
    double speedKmh = m_vehicleState.speed * 3.6;
    
    // 🎓 LEARNING POINT: Simulated Engine Data
    // Since Engine RPM signal isn't available, simulate it based on speed
    m_vehicleState.engineRpm = speedKmh * 40.0 + 800.0;  // Realistic RPM simulation
    
    // Simulate fuel consumption based on speed
    static double totalDistance = 0.0;
    totalDistance += speedKmh / 3600.0;  // Add distance traveled this second
    if (totalDistance > 10.0) {  // Every ~10km
        m_vehicleState.fuelLevel -= 1.0;  // Consume 1% fuel
        totalDistance = 0.0;
    }
    
    // 🎓 LEARNING POINT: Multi-Signal Correlation
    // Use multiple signals together to derive insights
    velocitas::logger().info("🚗 Vehicle Status Update:");
    velocitas::logger().info("   📊 Speed: {:.1f} km/h | RPM: {:.0f} | Fuel: {:.1f}%", 
        speedKmh, m_vehicleState.engineRpm, m_vehicleState.fuelLevel);
    velocitas::logger().info("   📍 Location: ({:.6f}, {:.6f})", 
        m_vehicleState.latitude, m_vehicleState.longitude);
    
    // Analyze driving pattern based on multiple signals
    analyzeDrivingPattern();
    
    // Calculate fuel efficiency
    calculateFuelEfficiency();
    
    // Update statistics
    m_drivingStats.dataPoints++;
    m_drivingStats.avgSpeed = ((m_drivingStats.avgSpeed * (m_drivingStats.dataPoints - 1)) + speedKmh) / m_drivingStats.dataPoints;
    m_drivingStats.avgRpm = ((m_drivingStats.avgRpm * (m_drivingStats.dataPoints - 1)) + m_vehicleState.engineRpm) / m_drivingStats.dataPoints;
    
    // Log comprehensive status
    logVehicleStatus();
}

// ============================================================================
// 🎓 STEP 2E: Driving Pattern Analysis
// ============================================================================
void MultiSignalProcessor::analyzeDrivingPattern() {
    double speedKmh = m_vehicleState.speed * 3.6;
    
    // 🎓 LEARNING POINT: Pattern Detection
    // Combine speed and RPM to determine driving mode
    std::string previousMode = m_drivingStats.drivingMode;
    m_drivingStats.drivingMode = getDrivingMode();
    
    if (m_drivingStats.drivingMode != previousMode) {
        velocitas::logger().info("🎯 Driving Mode Changed: {} → {}", 
            previousMode, m_drivingStats.drivingMode);
    }
    
    // 🎓 LEARNING POINT: Multi-Signal Alerts
    // Generate alerts based on combined signal analysis
    
    // High RPM at low speed (aggressive acceleration)
    if (speedKmh < 30 && m_vehicleState.engineRpm > 4000) {
        velocitas::logger().warn("⚠️  Aggressive acceleration detected! RPM: {:.0f} at {:.1f} km/h", 
            m_vehicleState.engineRpm, speedKmh);
    }
    
    // Low fuel warning with distance estimate
    if (m_vehicleState.fuelLevel < 15.0) {
        double estimatedRange = m_vehicleState.fuelLevel * 5.0; // Simplified calculation
        velocitas::logger().warn("⛽ Low fuel warning! {:.1f}% remaining (~{:.0f} km range)", 
            m_vehicleState.fuelLevel, estimatedRange);
    }
    
    // Optimal driving conditions
    if (speedKmh > 50 && speedKmh < 90 && m_vehicleState.engineRpm < 2500) {
        velocitas::logger().info("✅ Optimal driving conditions for fuel efficiency");
    }
}

// ============================================================================
// 🎓 STEP 2F: Fuel Efficiency Calculation
// ============================================================================
void MultiSignalProcessor::calculateFuelEfficiency() {
    double speedKmh = m_vehicleState.speed * 3.6;
    
    // 🎓 LEARNING POINT: Derived Metrics
    // Calculate fuel consumption from speed and RPM
    // This is a simplified model for learning purposes
    
    if (speedKmh > 0) {
        double instantConsumption = calculateInstantFuelConsumption(
            m_vehicleState.speed, m_vehicleState.engineRpm);
        
        m_drivingStats.fuelEfficiency = instantConsumption;
        
        velocitas::logger().info("⛽ Fuel Efficiency: {:.1f} L/100km", instantConsumption);
        
        // Efficiency feedback
        if (instantConsumption < 6.0) {
            velocitas::logger().info("🌱 Excellent fuel efficiency!");
        } else if (instantConsumption < 8.0) {
            velocitas::logger().info("👍 Good fuel efficiency");
        } else if (instantConsumption < 10.0) {
            velocitas::logger().info("⚡ Average fuel efficiency");
        } else {
            velocitas::logger().warn("💸 Poor fuel efficiency - consider adjusting driving style");
        }
    }
}

// ============================================================================
// 🎓 STEP 2G: Helper Methods
// ============================================================================
std::string MultiSignalProcessor::getDrivingMode() {
    double speedKmh = m_vehicleState.speed * 3.6;
    
    // 🎓 LEARNING POINT: Mode Detection Logic
    // Combine multiple signals to categorize driving behavior
    
    if (speedKmh == 0) {
        return "STOPPED";
    } else if (speedKmh < 10) {
        return "CRAWLING";
    } else if (speedKmh < 50) {
        if (m_vehicleState.engineRpm > 3000) {
            return "CITY_AGGRESSIVE";
        } else {
            return "CITY_NORMAL";
        }
    } else if (speedKmh < 100) {
        if (m_vehicleState.engineRpm > 3500) {
            return "HIGHWAY_SPORT";
        } else {
            return "HIGHWAY_CRUISE";
        }
    } else {
        return "HIGH_SPEED";
    }
}

double MultiSignalProcessor::calculateInstantFuelConsumption(double speed, double rpm) {
    // 🎓 LEARNING POINT: Simple Fuel Model
    // This is a simplified calculation for demonstration
    // Real calculations would use engine maps and vehicle parameters
    
    if (speed < 0.1) return 0.0; // Avoid division by zero
    
    double speedKmh = speed * 3.6;
    double baseConsumption = 5.0; // Base L/100km
    double rpmFactor = rpm / 2000.0; // RPM influence
    double speedFactor = 1.0;
    
    // Speed efficiency curve (simplified)
    if (speedKmh < 50) {
        speedFactor = 1.5; // City driving less efficient
    } else if (speedKmh < 90) {
        speedFactor = 1.0; // Optimal speed range
    } else {
        speedFactor = 1.0 + (speedKmh - 90) * 0.02; // Increasing with speed
    }
    
    return baseConsumption * rpmFactor * speedFactor;
}

void MultiSignalProcessor::logVehicleStatus() {
    // 🎓 LEARNING POINT: Comprehensive Status Reporting
    // Combine all signals and derived metrics for complete picture
    
    // Periodic comprehensive report
    if (m_drivingStats.dataPoints % 10 == 0) {
        velocitas::logger().info("📊 === VEHICLE STATUS REPORT ===");
        velocitas::logger().info("📈 Average Speed: {:.1f} km/h", m_drivingStats.avgSpeed);
        velocitas::logger().info("🔧 Average RPM: {:.0f}", m_drivingStats.avgRpm);
        velocitas::logger().info("⛽ Fuel Efficiency: {:.1f} L/100km", m_drivingStats.fuelEfficiency);
        velocitas::logger().info("🎯 Driving Mode: {}", m_drivingStats.drivingMode);
        velocitas::logger().info("📊 Data Points: {}", m_drivingStats.dataPoints);
        velocitas::logger().info("==============================");
    }
}

// ============================================================================
// 🎓 STEP 2H: Application Entry Point
// ============================================================================

std::unique_ptr<MultiSignalProcessor> multiSignalApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Multi-Signal Processor (signal: {})", signal);
    if (multiSignalApp) {
        multiSignalApp->stop();
    }
}

int main(int argc, char** argv) {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 2: Starting Multi-Signal Processor Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Master multi-signal correlation");
    velocitas::logger().info("📊 Processing: Speed + RPM + Fuel Level");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        // Create and start the multi-signal processor
        multiSignalApp = std::make_unique<MultiSignalProcessor>();
        multiSignalApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 2: Multi-Signal Processor completed");
    velocitas::logger().info("🎓 Next: Step 3 - Data Analysis & Alerts");
    return 0;
}

// ============================================================================
// 🎓 STEP 2: LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Multiple signal subscription with QueryBuilder
// ✅ Handling different data types (double, int, bool)
// ✅ Signal correlation and analysis
// ✅ State management for multiple signals
// ✅ Derived metrics calculation
// ✅ Pattern detection from combined data
// ✅ Comprehensive status reporting
//
// 🔧 KEY CODE PATTERNS:
// ✅ QueryBuilder with multiple .select() calls
// ✅ Individual signal validation in single callback
// ✅ Structured state tracking (VehicleState struct)
// ✅ Multi-signal correlation logic
// ✅ Derived metrics (fuel efficiency)
// ✅ Mode detection algorithms
// ✅ Periodic reporting patterns
//
// 📊 TESTING COMMANDS:
// Using Pre-built Image (RECOMMENDED):
// Build: docker run --rm --network host \
//        -v $(pwd)/examples/Step2_MultiSignalProcessor.cpp:/app.cpp \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
//
// Run:   docker run -d --network host --name step2-app \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
//
// Test:  echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
//        ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
//        
//        echo "setValue Vehicle.Powertrain.Engine.Speed 2500" | docker run --rm -i --network host \
//        ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
//        
//        echo "setValue Vehicle.Powertrain.FuelSystem.Level 75.5" | docker run --rm -i --network host \
//        ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
//
// 🎓 EXPECTED OUTPUT:
// 📡 Received multi-signal update
// 🚗 Vehicle Status Update:
//    📊 Speed: 90.0 km/h | RPM: 2500 | Fuel: 75.5%
// 🎯 Driving Mode Changed: UNKNOWN → HIGHWAY_CRUISE
// ✅ Optimal driving conditions for fuel efficiency
// ⛽ Fuel Efficiency: 6.2 L/100km
// 👍 Good fuel efficiency
//
// 🚀 NEXT STEP: Step3_DataAnalysisAlerts.cpp
// Learn advanced pattern analysis and alert systems!
// ============================================================================