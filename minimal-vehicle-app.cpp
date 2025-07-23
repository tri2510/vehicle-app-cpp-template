#include "sdk/VehicleApp.h"
#include "sdk/IPubSubClient.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/DataPointReply.h"
#include "vehicle/Vehicle.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>

using namespace velocitas;

class MinimalVehicleApp : public VehicleApp {
private:
    std::atomic<bool> connected{false};
    std::atomic<float> currentSpeed{0.0f};
    vehicle::Vehicle Vehicle;
    
public:
    MinimalVehicleApp(std::shared_ptr<IVehicleDataBrokerClient> vdbClient, 
                      std::shared_ptr<IPubSubClient> pubSubClient)
        : VehicleApp(vdbClient, pubSubClient) {
    }
    void onStart() override {
        velocitas::logger().info("=== Minimal Vehicle App Started ===");
        std::cout << "🚗 Minimal Vehicle App Started" << std::endl;
        std::cout << "🔌 Connecting to KUKSA.val Databroker..." << std::endl;
        
        // Subscribe to Vehicle.Speed signal
        auto subscription = subscribeDataPoints("SELECT Vehicle.Speed");
        subscription->onError([this](const velocitas::Status& status) {
            std::cout << "❌ Subscription error: " << status.errorMessage() << std::endl;
            velocitas::logger().error("Subscription error: {}", status.errorMessage());
        });
        subscription->onItem([this](const DataPointReply& reply) {
            onSpeedUpdate(reply);
        });
        
        std::cout << "✅ Subscription to Vehicle.Speed created" << std::endl;
        std::cout << "📊 Waiting for vehicle signals..." << std::endl;
        
        // Start a demo thread that shows the app is running
        std::thread statusThread([this]() {
            int counter = 0;
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                counter++;
                std::cout << "💓 App heartbeat #" << counter 
                          << " - Current speed: " << currentSpeed.load() << " km/h" << std::endl;
            }
        });
        statusThread.detach();
    }
    
private:
    void onSpeedUpdate(const DataPointReply& reply) {
        try {
            // Get speed data point
            auto speedDataPoint = reply.get(Vehicle.Speed);
            
            if (speedDataPoint && speedDataPoint->isValid()) {
                float speed = speedDataPoint->value();
                currentSpeed = speed;
                
                std::cout << "📈 Speed Update: " << speed << " km/h";
                
                // Positive test case: High speed alert
                if (speed > 80.0f) {
                    std::cout << " ⚠️  HIGH SPEED ALERT!";
                    velocitas::logger().warn("High speed detected: {} km/h", speed);
                }
                
                // Negative test case: Invalid speed
                if (speed < 0.0f || speed > 300.0f) {
                    std::cout << " ❌ INVALID SPEED VALUE!";
                    velocitas::logger().error("Invalid speed value: {} km/h", speed);
                }
                
                std::cout << std::endl;
                velocitas::logger().info("Speed updated: {} km/h", speed);
                
            } else {
                std::cout << "⚠️  No valid speed data in reply" << std::endl;
                velocitas::logger().warn("No valid speed data received");
            }
            
        } catch (const std::exception& e) {
            std::cout << "❌ Error processing speed: " << e.what() << std::endl;
            velocitas::logger().error("Speed processing error: {}", e.what());
        }
    }
};

int main(int argc, char** argv) {
    // Create the VehicleApp with null clients - this will use default KUKSA client
    auto app = std::make_unique<MinimalVehicleApp>(nullptr, nullptr);
    app->run();
    return 0;
}



