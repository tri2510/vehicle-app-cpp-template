// ============================================================================
// 🎓 STEP 1: BASIC SPEED MONITOR - Learn SDV Fundamentals
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this tutorial step, you will learn:
// - How to subscribe to vehicle signals using Velocitas SDK
// - Process Vehicle.Speed signal data in real-time
// - Convert speed units (m/s to km/h) for practical use
// - Build and test your first vehicle application
//
// 🎯 WHAT YOU'LL BUILD:
// A simple speed monitoring application that:
// - Connects to KUKSA Vehicle Data Broker
// - Subscribes to Vehicle.Speed signal
// - Displays speed in both m/s and km/h formats
// - Categorizes speed (stopped, slow, normal, fast)
//
// 💡 DIFFICULTY: ⭐ Beginner (15 minutes)
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
 * @brief Step 1: Basic Speed Monitor Application
 * 
 * 🎓 TUTORIAL: This is your first vehicle application class.
 * It demonstrates the fundamental concepts of SDV development:
 * - Signal subscription
 * - Data processing
 * - Unit conversion
 * - Logging and feedback
 */
class BasicSpeedMonitor : public velocitas::VehicleApp {
public:
    BasicSpeedMonitor();

protected:
    void onStart() override;

private:
    void onSignalChanged(const velocitas::DataPointReply& reply);
    void processSpeedSignal(double speedMs);
    std::string categorizeSpeed(double speedKmh);
};

// ============================================================================
// 🎓 STEP 1A: Application Initialization
// ============================================================================
BasicSpeedMonitor::BasicSpeedMonitor()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 1: Basic Speed Monitor starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("🚗 Learning objective: Process Vehicle.Speed signal");
    velocitas::logger().info("✅ Basic Speed Monitor initialized");
}

// ============================================================================
// 🎓 STEP 1B: Signal Subscription Setup
// ============================================================================
void BasicSpeedMonitor::onStart() {
    velocitas::logger().info("🚀 Step 1: Starting Basic Speed Monitor!");
    velocitas::logger().info("📊 Setting up Vehicle.Speed signal subscription...");
    
    // Give the databroker connection time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🎓 LEARNING POINT: Signal Subscription
    // This is how you subscribe to vehicle signals in Velocitas:
    // 1. Use QueryBuilder to specify which signals you want
    // 2. Build the query
    // 3. Set up callbacks for data and errors
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Speed)  // Subscribe to Vehicle.Speed signal
            .build()                                     // Build the subscription query
    )
    ->onItem([this](auto&& item) { 
        // 🎓 LEARNING POINT: Lambda Callback
        // This lambda function is called every time new signal data arrives
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        // 🎓 LEARNING POINT: Error Handling
        // Always handle subscription errors gracefully
        velocitas::logger().error("❌ Signal subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Vehicle.Speed subscription completed");
    velocitas::logger().info("🔄 Waiting for speed data from vehicle...");
    velocitas::logger().info("💡 Test with: echo 'setValue Vehicle.Speed 25.0' | kuksa-client");
}

// ============================================================================
// 🎓 STEP 1C: Signal Data Processing
// ============================================================================
void BasicSpeedMonitor::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        velocitas::logger().info("📡 Received vehicle signal data");
        
        // 🎓 LEARNING POINT: Signal Validation
        // Always check if the signal data is valid before using it
        if (reply.get(Vehicle.Speed)->isValid()) {
            double speedMs = reply.get(Vehicle.Speed)->value();  // Speed in meters per second
            processSpeedSignal(speedMs);
        } else {
            velocitas::logger().debug("⏳ Waiting for valid Vehicle.Speed data...");
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Processing signal data...");
    }
}

// ============================================================================
// 🎓 STEP 1D: Speed Processing and Unit Conversion
// ============================================================================
void BasicSpeedMonitor::processSpeedSignal(double speedMs) {
    // 🎓 LEARNING POINT: Unit Conversion
    // Vehicle signals often use SI units (meters/second for speed)
    // Convert to practical units (kilometers/hour) for human understanding
    double speedKmh = speedMs * 3.6;  // Convert m/s to km/h
    
    // 🎓 LEARNING POINT: Structured Logging
    // Use clear, informative log messages for debugging and monitoring
    velocitas::logger().info("🚗 Vehicle Speed: {:.1f} km/h ({:.2f} m/s)", speedKmh, speedMs);
    
    // 🎓 LEARNING POINT: Business Logic
    // Categorize the speed to provide meaningful feedback
    std::string category = categorizeSpeed(speedKmh);
    velocitas::logger().info("📊 Speed Category: {}", category);
    
    // 🎓 LEARNING POINT: Conditional Processing
    // Different actions based on speed ranges
    if (speedKmh > 100.0) {
        velocitas::logger().warn("⚠️  HIGH SPEED: {:.1f} km/h - Drive safely!", speedKmh);
    } else if (speedKmh > 50.0) {
        velocitas::logger().info("🛣️  Highway Speed: {:.1f} km/h", speedKmh);
    } else if (speedKmh > 20.0) {
        velocitas::logger().info("🏘️  City Speed: {:.1f} km/h", speedKmh);
    } else if (speedKmh > 0.0) {
        velocitas::logger().info("🐌 Low Speed: {:.1f} km/h", speedKmh);
    } else {
        velocitas::logger().info("🛑 Vehicle Stopped");
    }
}

// ============================================================================
// 🎓 STEP 1E: Helper Functions
// ============================================================================
std::string BasicSpeedMonitor::categorizeSpeed(double speedKmh) {
    // 🎓 LEARNING POINT: Categorization Logic
    // Transform numerical data into meaningful categories
    if (speedKmh == 0.0) {
        return "STOPPED";
    } else if (speedKmh <= 20.0) {
        return "VERY_SLOW";
    } else if (speedKmh <= 50.0) {
        return "CITY_SPEED";
    } else if (speedKmh <= 100.0) {
        return "HIGHWAY_SPEED";
    } else {
        return "HIGH_SPEED";
    }
}

// ============================================================================
// 🎓 STEP 1F: Application Entry Point
// ============================================================================

std::unique_ptr<BasicSpeedMonitor> speedMonitor;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Basic Speed Monitor (signal: {})", signal);
    if (speedMonitor) {
        speedMonitor->stop();
    }
}

int main(int argc, char** argv) {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 1: Starting Basic Speed Monitor Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Master vehicle signal processing");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        // Create and start the basic speed monitor
        speedMonitor = std::make_unique<BasicSpeedMonitor>();
        speedMonitor->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 1: Basic Speed Monitor completed");
    velocitas::logger().info("🎓 Next: Step 2 - Multi-Signal Processing");
    return 0;
}

// ============================================================================
// 🎓 STEP 1: LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Vehicle signal subscription using QueryBuilder
// ✅ Signal validation and error handling
// ✅ Unit conversion (m/s to km/h)
// ✅ Structured logging and debugging
// ✅ Basic business logic implementation
// ✅ Application lifecycle management
//
// 🔧 KEY CODE PATTERNS:
// ✅ subscribeDataPoints() - Subscribe to vehicle signals
// ✅ reply.get(Vehicle.Signal)->isValid() - Validate signal data
// ✅ reply.get(Vehicle.Signal)->value() - Extract signal value
// ✅ velocitas::logger().info() - Structured logging
// ✅ Unit conversion formulas
// ✅ Conditional processing based on ranges
//
// 📊 TESTING COMMANDS:
// Using Pre-built Image (RECOMMENDED):
// Build: docker run --rm --network host \
//        -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
//
// Run:   docker run -d --network host --name step1-app \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
//
// Using Local Build:
// Build: docker run --rm --network host \
//        -v $(pwd)/examples/Step1_BasicSpeedMonitor.cpp:/app.cpp \
//        velocitas-quick build --skip-deps --verbose
//
// Test:  echo "setValue Vehicle.Speed 25.0" | \
//        docker run --rm -i --network host \
//        ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
//
// 🎓 EXPECTED OUTPUT:
// 📡 Received vehicle signal data
// 🚗 Vehicle Speed: 90.0 km/h (25.00 m/s)
// 📊 Speed Category: HIGHWAY_SPEED
// 🛣️  Highway Speed: 90.0 km/h
//
// 🚀 NEXT STEP: Step2_MultiSignalProcessor.cpp
// Learn to handle multiple vehicle signals simultaneously!
// ============================================================================