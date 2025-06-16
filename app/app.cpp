/**
 * Ultra-Minimal Vehicle App
 * 
 * Reads Vehicle.Speed from Vehicle Data Broker and logs the values.
 * Single file implementation with no MQTT, no headers, no complexity.
 */

#include "sdk/VehicleApp.h"
#include "sdk/Logger.h" 
#include "sdk/QueryBuilder.h"
#include "sdk/IPubSubClient.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include "vehicle/Vehicle.hpp"
#include <csignal>
#include <memory>

// Create global Vehicle instance (following working SpeedMonitorApp pattern)
::vehicle::Vehicle Vehicle;

class MyApp : public velocitas::VehicleApp {
public:
    MyApp()
        : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                     nullptr) {  // No MQTT - pass nullptr for PubSub client
        velocitas::logger().info("Minimal Vehicle App created");
    }

protected:
    void onStart() override {
        velocitas::logger().info("Minimal Vehicle App started - monitoring Vehicle.Speed");
        
        // Subscribe to Vehicle.Speed only (using global Vehicle instance)
        subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
            ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
            ->onError([this](auto&& status) { 
                velocitas::logger().error("Error reading Vehicle.Speed: {}", status.errorMessage());
            });
    }

private:
    void onSpeedChanged(const velocitas::DataPointReply& reply) {
        try {
            auto speedValue = reply.get(Vehicle.Speed)->value();
            double speedMs = speedValue;
            velocitas::logger().info("Vehicle Speed: {:.2f} m/s ({:.1f} km/h)", 
                                   speedMs, speedMs * 3.6);
        } catch (const std::exception& e) {
            velocitas::logger().debug("Speed data not available: {}", e.what());
        }
    }
};

// Global app instance for signal handling
std::unique_ptr<MyApp> app;

void signalHandler(int signal) {
    velocitas::logger().info("Received signal {}, shutting down...", signal);
    if (app) {
        app->stop();
    }
}

int main() {
    // Setup signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("Starting Minimal Vehicle App");
    
    try {
        // Create app using working pattern (clients created in constructor)
        app = std::make_unique<MyApp>();
        app->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("App failed: {}", e.what());
        return 1;
    }
    
    velocitas::logger().info("Minimal Vehicle App stopped");
    return 0;
}