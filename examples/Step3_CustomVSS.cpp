// ============================================================================
// 🎓 STEP 3: CUSTOM VSS INTEGRATION - Professional Signal Specification
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this tutorial step, you will learn:
// - How to subscribe to custom VSS signals using Velocitas SDK
// - Process Vehicle.Tutorial.* custom signal hierarchy in real-time
// - Handle complex custom signal data with validation
// - Build comprehensive custom VSS applications
//
// 🎯 WHAT YOU'LL BUILD:
// A custom VSS integration application that:
// - Connects to KUKSA Vehicle Data Broker with custom VSS
// - Subscribes to Vehicle.Tutorial.DriverBehavior.Score signal
// - Subscribes to Vehicle.Tutorial.Fleet.Status signal  
// - Subscribes to Vehicle.Tutorial.Diagnostics.SystemHealth signal
// - Processes and analyzes custom signal data
//
// 💡 DIFFICULTY: ⭐⭐⭐⭐ Expert (60 minutes)
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
 * @brief Step 3: Custom VSS Integration Application
 * 
 * 🎓 TUTORIAL: This application demonstrates custom VSS integration:
 * - Custom signal subscription using generated Vehicle model
 * - Processing Vehicle.Tutorial.* hierarchy signals
 * - Advanced signal validation and analytics
 * - Production-ready custom VSS patterns
 */
class CustomVSSIntegration : public velocitas::VehicleApp {
public:
    CustomVSSIntegration();

protected:
    void onStart() override;

private:
    void onSignalChanged(const velocitas::DataPointReply& reply);
    void processDriverBehaviorScore(uint8_t score);
    void processFleetStatus(const std::string& status);
    void processSystemHealth(uint8_t health);
    std::string interpretDriverScore(uint8_t score);
    std::string interpretFleetStatus(const std::string& status);
    std::string interpretSystemHealth(uint8_t health);
    
    // Tutorial signal state tracking
    struct TutorialSignalState {
        uint8_t driverScore = 0;
        std::string fleetStatus = "";
        uint8_t systemHealth = 100;
        bool driverScoreValid = false;
        bool fleetStatusValid = false;
        bool systemHealthValid = false;
        int totalSignalsReceived = 0;
    };
    
    TutorialSignalState m_tutorialState;
};

// ============================================================================
// 🎓 STEP 3A: Custom VSS Application Initialization
// ============================================================================
CustomVSSIntegration::CustomVSSIntegration()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 3: Custom VSS Integration starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("🚗 Learning objective: Process Vehicle.Tutorial.* custom signals");
    velocitas::logger().info("📊 Custom VSS: DriverBehavior, Fleet, Diagnostics branches");
    velocitas::logger().info("✅ Custom VSS Integration initialized");
}

// ============================================================================
// 🎓 STEP 3B: Custom VSS Signal Subscription Setup
// ============================================================================
void CustomVSSIntegration::onStart() {
    velocitas::logger().info("🚀 Step 3: Starting Custom VSS Integration!");
    velocitas::logger().info("📊 Setting up Vehicle.Tutorial.* signal subscriptions...");
    
    // Give the databroker connection time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🎓 LEARNING POINT: Custom VSS Signal Subscription
    // Subscribe to custom signals using the generated Vehicle model
    // The Vehicle model was generated from Step3_CustomVSS.json
    
    // Subscribe to Driver Behavior Score
    velocitas::logger().info("📊 Subscribing to Vehicle.Tutorial.DriverBehavior.Score...");
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Tutorial.DriverBehavior.Score)
            .build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ DriverBehavior.Score subscription error: {}", status.errorMessage());
    });
    
    // Subscribe to Fleet Status
    velocitas::logger().info("📊 Subscribing to Vehicle.Tutorial.Fleet.Status...");
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Tutorial.Fleet.Status)
            .build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Fleet.Status subscription error: {}", status.errorMessage());
    });
    
    // Subscribe to System Health
    velocitas::logger().info("📊 Subscribing to Vehicle.Tutorial.Diagnostics.SystemHealth...");
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Tutorial.Diagnostics.SystemHealth)
            .build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Diagnostics.SystemHealth subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Custom VSS signal subscriptions completed");
    velocitas::logger().info("🔄 Waiting for custom VSS data from vehicle...");
    velocitas::logger().info("💡 Test with custom signal injection:");
    velocitas::logger().info("   echo 'setValue Vehicle.Tutorial.DriverBehavior.Score 85' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.Tutorial.Fleet.Status \"DRIVING\"' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.Tutorial.Diagnostics.SystemHealth 95' | kuksa-client");
}

// ============================================================================
// 🎓 STEP 3C: Custom VSS Signal Data Processing
// ============================================================================
void CustomVSSIntegration::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        velocitas::logger().info("📡 Received custom VSS signal data");
        m_tutorialState.totalSignalsReceived++;
        
        // 🎓 LEARNING POINT: Custom Signal Validation and Processing
        // Process Driver Behavior Score
        if (reply.get(Vehicle.Tutorial.DriverBehavior.Score)->isValid()) {
            uint8_t score = reply.get(Vehicle.Tutorial.DriverBehavior.Score)->value();
            m_tutorialState.driverScore = score;
            m_tutorialState.driverScoreValid = true;
            processDriverBehaviorScore(score);
        }
        
        // Process Fleet Status
        if (reply.get(Vehicle.Tutorial.Fleet.Status)->isValid()) {
            std::string status = reply.get(Vehicle.Tutorial.Fleet.Status)->value();
            m_tutorialState.fleetStatus = status;
            m_tutorialState.fleetStatusValid = true;
            processFleetStatus(status);
        }
        
        // Process System Health
        if (reply.get(Vehicle.Tutorial.Diagnostics.SystemHealth)->isValid()) {
            uint8_t health = reply.get(Vehicle.Tutorial.Diagnostics.SystemHealth)->value();
            m_tutorialState.systemHealth = health;
            m_tutorialState.systemHealthValid = true;
            processSystemHealth(health);
        }
        
        // Log tutorial analytics
        velocitas::logger().info("📊 Tutorial Analytics: {} signals received", m_tutorialState.totalSignalsReceived);
        velocitas::logger().info("✅ Valid Signals - Driver: {}, Fleet: {}, Diagnostics: {}", 
            m_tutorialState.driverScoreValid ? "✓" : "✗",
            m_tutorialState.fleetStatusValid ? "✓" : "✗", 
            m_tutorialState.systemHealthValid ? "✓" : "✗");
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Processing custom VSS signal data...");
    }
}

// ============================================================================
// 🎓 STEP 3D: Driver Behavior Signal Processing
// ============================================================================
void CustomVSSIntegration::processDriverBehaviorScore(uint8_t score) {
    // 🎓 LEARNING POINT: Custom Signal Business Logic
    std::string interpretation = interpretDriverScore(score);
    velocitas::logger().info("🎯 Driver Behavior Score: {} - {}", score, interpretation);
    
    // 🎓 LEARNING POINT: Conditional Processing for Custom Signals
    if (score >= 90) {
        velocitas::logger().info("🏆 EXCELLENT Driver Performance: {} - Top tier driver!", score);
    } else if (score >= 80) {
        velocitas::logger().info("✅ GOOD Driver Performance: {} - Above average driving", score);
    } else if (score >= 70) {
        velocitas::logger().info("📊 AVERAGE Driver Performance: {} - Standard driving behavior", score);
    } else if (score >= 60) {
        velocitas::logger().warn("⚠️  NEEDS IMPROVEMENT: {} - Driver coaching recommended", score);
    } else {
        velocitas::logger().warn("🚨 HIGH RISK DRIVER: {} - Immediate intervention required!", score);
    }
}

// ============================================================================
// 🎓 STEP 3E: Fleet Management Signal Processing
// ============================================================================
void CustomVSSIntegration::processFleetStatus(const std::string& status) {
    // 🎓 LEARNING POINT: String Signal Processing
    std::string interpretation = interpretFleetStatus(status);
    velocitas::logger().info("🚛 Fleet Status: {} - {}", status, interpretation);
    
    // 🎓 LEARNING POINT: String-based Conditional Logic
    if (status == "IDLE") {
        velocitas::logger().info("🟢 Vehicle available for dispatch");
    } else if (status == "DRIVING") {
        velocitas::logger().info("🚗 Vehicle in active use - monitoring performance");
    } else if (status == "MAINTENANCE") {
        velocitas::logger().warn("🔧 Vehicle undergoing service - unavailable for dispatch");
    } else if (status == "EMERGENCY") {
        velocitas::logger().error("🚨 EMERGENCY STATUS: Vehicle requires immediate attention!");
    } else {
        velocitas::logger().warn("❓ Unknown fleet status: {}", status);
    }
}

// ============================================================================
// 🎓 STEP 3F: Diagnostic Signal Processing
// ============================================================================
void CustomVSSIntegration::processSystemHealth(uint8_t health) {
    // 🎓 LEARNING POINT: Health Percentage Processing
    std::string interpretation = interpretSystemHealth(health);
    velocitas::logger().info("💊 System Health: {}% - {}", health, interpretation);
    
    // 🎓 LEARNING POINT: Health Threshold Analysis
    if (health >= 95) {
        velocitas::logger().info("🟢 EXCELLENT System Health: {}% - All systems optimal", health);
    } else if (health >= 85) {
        velocitas::logger().info("✅ GOOD System Health: {}% - Minor optimization possible", health);
    } else if (health >= 70) {
        velocitas::logger().warn("🟡 MODERATE System Health: {}% - Monitor closely", health);
    } else if (health >= 50) {
        velocitas::logger().warn("🟠 LOW System Health: {}% - Maintenance recommended", health);
    } else {
        velocitas::logger().error("🔴 CRITICAL System Health: {}% - Immediate inspection required!", health);
    }
}

// ============================================================================
// 🎓 STEP 3G: Helper Functions for Signal Interpretation
// ============================================================================
std::string CustomVSSIntegration::interpretDriverScore(uint8_t score) {
    // 🎓 LEARNING POINT: Signal Value Interpretation
    if (score >= 90) return "Excellent Driver";
    else if (score >= 80) return "Good Driver";
    else if (score >= 70) return "Average Driver";
    else if (score >= 60) return "Needs Improvement";
    else return "High Risk Driver";
}

std::string CustomVSSIntegration::interpretFleetStatus(const std::string& status) {
    // 🎓 LEARNING POINT: String Signal Mapping
    if (status == "IDLE") return "Vehicle available for dispatch";
    else if (status == "DRIVING") return "Vehicle in active use";
    else if (status == "MAINTENANCE") return "Vehicle undergoing service";
    else if (status == "EMERGENCY") return "Vehicle requires immediate attention";
    else return "Unknown status";
}

std::string CustomVSSIntegration::interpretSystemHealth(uint8_t health) {
    // 🎓 LEARNING POINT: Health Status Categorization
    if (health >= 95) return "Optimal condition";
    else if (health >= 85) return "Good condition";
    else if (health >= 70) return "Acceptable condition";
    else if (health >= 50) return "Poor condition";
    else return "Critical condition";
}

// ============================================================================
// 🎓 STEP 3H: Application Entry Point
// ============================================================================

std::unique_ptr<CustomVSSIntegration> customVssApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Custom VSS Integration (signal: {})", signal);
    if (customVssApp) {
        customVssApp->stop();
    }
}

int main(int argc, char** argv) {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 3: Starting Custom VSS Integration Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Master custom VSS signal processing");
    velocitas::logger().info("📊 Custom Signals: Vehicle.Tutorial.* hierarchy");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        // Create and start the custom VSS integration
        customVssApp = std::make_unique<CustomVSSIntegration>();
        customVssApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 3: Custom VSS Integration completed");
    velocitas::logger().info("🎓 You successfully processed custom VSS signals!");
    return 0;
}

// ============================================================================
// 🎓 STEP 3: LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Custom VSS signal subscription using generated Vehicle model
// ✅ Processing Vehicle.Tutorial.* hierarchical signals
// ✅ Custom signal validation and error handling
// ✅ String and numeric custom signal processing
// ✅ Business logic implementation for custom signals
// ✅ Production-ready custom VSS integration patterns
//
// 🔧 KEY CODE PATTERNS:
// ✅ Vehicle.Tutorial.DriverBehavior.Score subscription
// ✅ Vehicle.Tutorial.Fleet.Status string processing
// ✅ Vehicle.Tutorial.Diagnostics.SystemHealth monitoring
// ✅ Custom signal validation: reply.get(Vehicle.Tutorial.*)->isValid()
// ✅ Type-specific value extraction: ->value() for uint8_t and string
// ✅ Conditional business logic based on custom signal ranges
//
// 📊 TESTING COMMANDS:
// Build with Custom VSS:
// docker run --rm --network host \
//   -v step3-build:/quickbuild/build \
//   -v step3-deps:/home/vscode/.conan2 \
//   -v step3-vss:/quickbuild/app/vehicle_model \
//   -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
//   -e VSS_SPEC_FILE=/custom_vss.json \
//   -v $(pwd)/examples/Step3_CustomVSS.cpp:/app.cpp \
//   -v $(pwd)/examples/Step3_CustomVSS.json:/custom_vss.json \
//   velocitas-quick build --verbose --force
//
// Run Application:
// docker run -d --network host --name step3-custom-vss \
//   -v step3-build:/quickbuild/build \
//   velocitas-quick run 180
//
// Test Custom Signals:
// echo "setValue Vehicle.Tutorial.DriverBehavior.Score 85" | kuksa-client
// echo "setValue Vehicle.Tutorial.Fleet.Status \"DRIVING\"" | kuksa-client  
// echo "setValue Vehicle.Tutorial.Diagnostics.SystemHealth 95" | kuksa-client
//
// 🎓 EXPECTED OUTPUT:
// 📡 Received custom VSS signal data
// 🎯 Driver Behavior Score: 85 - Good Driver
// ✅ GOOD Driver Performance: 85 - Above average driving
// 🚛 Fleet Status: DRIVING - Vehicle in active use
// 🚗 Vehicle in active use - monitoring performance
// 💊 System Health: 95% - Optimal condition
// 🟢 EXCELLENT System Health: 95% - All systems optimal
//
// 🏆 CONGRATULATIONS! You've mastered custom VSS integration!
// ============================================================================