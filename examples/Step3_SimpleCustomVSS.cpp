// ============================================================================
// 🎓 STEP 3: SIMPLE CUSTOM VSS - Real Custom Signal Testing
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this tutorial step, you will learn:
// - How to create and load custom VSS specifications into KUKSA
// - Subscribe to custom VSS signals using raw paths
// - Process real custom signals injected via KUKSA client
// - Build end-to-end custom VSS applications
//
// 🎯 WHAT YOU'LL BUILD:
// A simple custom VSS application that:
// - Subscribes to Vehicle.MyCustom.Temperature (custom signal)
// - Subscribes to Vehicle.MyCustom.Message (custom signal)
// - Subscribes to Vehicle.MyCustom.Counter (custom signal)
// - Processes real custom signals from KUKSA databroker
//
// 💡 DIFFICULTY: ⭐⭐⭐ Advanced (45 minutes)
// ============================================================================

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include <fmt/format.h>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

/**
 * @brief Step 3: Simple Custom VSS Application
 * 
 * 🎓 TUTORIAL: This application demonstrates real custom VSS signals:
 * - Direct subscription to custom VSS signals
 * - Processing custom signal data from KUKSA
 * - End-to-end custom VSS testing
 */
class SimpleCustomVSS : public velocitas::VehicleApp {
public:
    SimpleCustomVSS();

protected:
    void onStart() override;

private:
    void onSignalChanged(const velocitas::DataPointReply& reply);
    
    // Custom signal state
    struct CustomSignalState {
        double temperature = 0.0;
        std::string message = "";
        uint32_t counter = 0;
        bool temperatureValid = false;
        bool messageValid = false;
        bool counterValid = false;
    };
    
    CustomSignalState m_customState;
    
    void processCustomSignals();
    void logCustomState();
};

// ============================================================================
// 🎓 STEP 3A: Simple Custom VSS Initialization
// ============================================================================
SimpleCustomVSS::SimpleCustomVSS()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 3: Simple Custom VSS starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("🚗 Learning objective: Process real custom VSS signals");
    velocitas::logger().info("📊 Custom Signals: Vehicle.MyCustom.Temperature, Message, Counter");
    velocitas::logger().info("✅ Simple Custom VSS initialized");
}

// ============================================================================
// 🎓 STEP 3B: Custom VSS Signal Subscriptions
// ============================================================================
void SimpleCustomVSS::onStart() {
    velocitas::logger().info("🚀 Step 3: Starting Simple Custom VSS!");
    velocitas::logger().info("📊 Setting up custom VSS signal subscriptions...");
    
    // Give the databroker connection time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🎓 LEARNING POINT: Custom VSS Signal Subscription
    // Subscribe to custom signals using raw signal paths
    // Note: We use raw signal paths since we don't have generated Vehicle model for custom signals
    
    // Subscribe to Vehicle.Speed (standard signal for reference)
    try {
        velocitas::logger().info("📊 Subscribing to Vehicle.Speed (standard reference)...");
        
        // Use raw subscription approach for custom signals
        getDataBrokerClient()->subscribeDatapoints(
            {"Vehicle.Speed"},
            [this](auto&& response) {
                velocitas::logger().info("📡 Received Vehicle.Speed update");
                // Process standard signal
            }
        );
        
    } catch (const std::exception& e) {
        velocitas::logger().error("❌ Standard signal subscription error: {}", e.what());
    }
    
    // 🎓 LEARNING POINT: Raw Custom Signal Subscription
    // For custom VSS signals, we need to use raw subscription paths
    try {
        velocitas::logger().info("📊 Subscribing to custom VSS signals...");
        
        // Subscribe to custom signals using raw paths
        getDataBrokerClient()->subscribeDatapoints(
            {
                "Vehicle.MyCustom.Temperature",
                "Vehicle.MyCustom.Message", 
                "Vehicle.MyCustom.Counter"
            },
            [this](auto&& response) {
                velocitas::logger().info("📡 Received custom VSS signal update");
                
                // Process custom signals from response
                for (const auto& datapoint : response.datapoints()) {
                    if (datapoint.name() == "Vehicle.MyCustom.Temperature" && datapoint.has_value()) {
                        if (datapoint.value().has_float_value()) {
                            m_customState.temperature = datapoint.value().float_value();
                            m_customState.temperatureValid = true;
                            velocitas::logger().info("🌡️  Custom Temperature: {:.1f}°C", m_customState.temperature);
                        }
                    }
                    else if (datapoint.name() == "Vehicle.MyCustom.Message" && datapoint.has_value()) {
                        if (datapoint.value().has_string_value()) {
                            m_customState.message = datapoint.value().string_value();
                            m_customState.messageValid = true;
                            velocitas::logger().info("💬 Custom Message: '{}'", m_customState.message);
                        }
                    }
                    else if (datapoint.name() == "Vehicle.MyCustom.Counter" && datapoint.has_value()) {
                        if (datapoint.value().has_uint32_value()) {
                            m_customState.counter = datapoint.value().uint32_value();
                            m_customState.counterValid = true;
                            velocitas::logger().info("🔢 Custom Counter: {}", m_customState.counter);
                        }
                    }
                }
                
                processCustomSignals();
            }
        );
        
        velocitas::logger().info("✅ Custom VSS signal subscriptions completed");
        
    } catch (const std::exception& e) {
        velocitas::logger().error("❌ Custom VSS subscription error: {}", e.what());
        velocitas::logger().info("💡 Make sure KUKSA is running with custom VSS file:");
        velocitas::logger().info("   docker run --name kuksa-custom -v $(pwd)/examples/simple_custom_vss.json:/vss.json");
        velocitas::logger().info("   ghcr.io/eclipse-kuksa/kuksa-databroker:main --vss /vss.json");
    }
    
    velocitas::logger().info("🔄 Waiting for custom VSS signals...");
    velocitas::logger().info("💡 Test with custom signals:");
    velocitas::logger().info("   echo 'setValue Vehicle.MyCustom.Temperature 25.5' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.MyCustom.Message \"Hello Custom VSS!\"' | kuksa-client");
    velocitas::logger().info("   echo 'setValue Vehicle.MyCustom.Counter 42' | kuksa-client");
}

// ============================================================================
// 🎓 STEP 3C: Custom Signal Processing
// ============================================================================
void SimpleCustomVSS::processCustomSignals() {
    velocitas::logger().info("🎯 Processing custom VSS signals...");
    
    // 🎓 LEARNING POINT: Custom Signal Processing Logic
    // Process the custom signals with business logic
    
    if (m_customState.temperatureValid) {
        if (m_customState.temperature > 30.0) {
            velocitas::logger().warn("🔥 HIGH CUSTOM TEMPERATURE: {:.1f}°C", m_customState.temperature);
        } else if (m_customState.temperature < 0.0) {
            velocitas::logger().warn("🧊 LOW CUSTOM TEMPERATURE: {:.1f}°C", m_customState.temperature);
        } else {
            velocitas::logger().info("✅ Custom temperature normal: {:.1f}°C", m_customState.temperature);
        }
    }
    
    if (m_customState.messageValid) {
        if (m_customState.message.find("ALERT") != std::string::npos) {
            velocitas::logger().warn("🚨 ALERT in custom message: '{}'", m_customState.message);
        } else {
            velocitas::logger().info("📝 Custom message received: '{}'", m_customState.message);
        }
    }
    
    if (m_customState.counterValid) {
        if (m_customState.counter > 100) {
            velocitas::logger().warn("📊 HIGH CUSTOM COUNTER: {}", m_customState.counter);
        } else {
            velocitas::logger().info("🔢 Custom counter: {}", m_customState.counter);
        }
    }
    
    // Log combined state
    logCustomState();
}

void SimpleCustomVSS::logCustomState() {
    velocitas::logger().info("📊 === CUSTOM VSS STATE ===");
    if (m_customState.temperatureValid) {
        velocitas::logger().info("🌡️  Temperature: {:.1f}°C", m_customState.temperature);
    }
    if (m_customState.messageValid) {
        velocitas::logger().info("💬 Message: '{}'", m_customState.message);
    }
    if (m_customState.counterValid) {
        velocitas::logger().info("🔢 Counter: {}", m_customState.counter);
    }
    velocitas::logger().info("🎉 Custom VSS signals working!");
    velocitas::logger().info("========================");
}

// ============================================================================
// 🎓 STEP 3D: Application Entry Point
// ============================================================================

std::unique_ptr<SimpleCustomVSS> customApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Simple Custom VSS (signal: {})", signal);
    if (customApp) {
        customApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 3: Starting Simple Custom VSS Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Real custom VSS signal processing");
    velocitas::logger().info("🔧 Custom Signals: Temperature, Message, Counter");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        customApp = std::make_unique<SimpleCustomVSS>();
        customApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 3: Simple Custom VSS completed");
    velocitas::logger().info("🎓 You successfully processed real custom VSS signals!");
    return 0;
}

// ============================================================================
// 🎓 STEP 3: LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Custom VSS specification creation
// ✅ KUKSA databroker custom VSS loading
// ✅ Raw signal path subscription for custom signals
// ✅ Real custom signal processing from KUKSA
// ✅ End-to-end custom VSS testing
//
// 🔧 KEY CODE PATTERNS:
// ✅ Raw signal subscription: getDataBrokerClient()->subscribeDatapoints()
// ✅ Custom signal path format: "Vehicle.MyCustom.SignalName"
// ✅ Signal response processing: datapoint.name() and datapoint.value()
// ✅ Type-specific value access: float_value(), string_value(), uint32_value()
//
// 📊 TESTING COMMANDS:
// Setup KUKSA with Custom VSS:
//   docker run -d --name kuksa-custom --network host \
//     -v $(pwd)/examples/simple_custom_vss.json:/vss.json \
//     ghcr.io/eclipse-kuksa/kuksa-databroker:main \
//     --address 0.0.0.0 --port 55555 --insecure --vss /vss.json
//
// Build and Run:
//   docker run --rm --network host \
//     -v $(pwd)/examples/Step3_SimpleCustomVSS.cpp:/app.cpp \
//     velocitas-quick build --skip-deps --verbose
//   
//   docker run -d --network host --name simple-custom-vss \
//     velocitas-quick run 120
//
// Test Custom Signals:
//   echo "setValue Vehicle.MyCustom.Temperature 35.5" | kuksa-client
//   echo "setValue Vehicle.MyCustom.Message \"ALERT: System Check\"" | kuksa-client  
//   echo "setValue Vehicle.MyCustom.Counter 150" | kuksa-client
//
// 🎓 EXPECTED OUTPUT:
// 🌡️  Custom Temperature: 35.5°C
// 🔥 HIGH CUSTOM TEMPERATURE: 35.5°C
// 💬 Custom Message: 'ALERT: System Check'
// 🚨 ALERT in custom message: 'ALERT: System Check'
// 🔢 Custom Counter: 150
// 📊 HIGH CUSTOM COUNTER: 150
// 🎉 Custom VSS signals working!
//
// 🏆 CONGRATULATIONS! You've built a working custom VSS application!
// ============================================================================