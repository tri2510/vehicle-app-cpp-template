#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include "vehicle/Vehicle.hpp"
#include <fmt/format.h>
#include <csignal>
#include <memory>

::vehicle::Vehicle Vehicle;

class SimpleTestApp : public velocitas::VehicleApp {
public:
    SimpleTestApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
        velocitas::logger().info("🧪 SIMPLE TEST APP STARTING - This should be different from template!");
    }

protected:
    void onStart() override {
        velocitas::logger().info("🧪 TEST APP: Setting up signal subscriptions");
        
        subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
            ->onItem([this](auto&& item) { onTestDataChanged(std::forward<decltype(item)>(item)); })
            ->onError([this](auto&& status) { 
                velocitas::logger().error("🧪 TEST APP error: {}", status.errorMessage());
            });
        
        velocitas::logger().info("✅ TEST APP: Signal subscription completed");
    }

private:
    void onTestDataChanged(const velocitas::DataPointReply& reply) {
        try {
            auto speed = reply.get(Vehicle.Speed)->value() * 3.6f;
            velocitas::logger().info("🧪 TEST APP SPEED: {:.1f} km/h - THIS IS THE CUSTOM APP!", speed);
        } catch (const std::exception& e) {
            velocitas::logger().debug("🧪 TEST APP waiting for data...");
        }
    }
};

std::unique_ptr<SimpleTestApp> testApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 TEST APP terminated");
    if (testApp) {
        testApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🧪 STARTING SIMPLE TEST APP - NOT THE TEMPLATE!");
    
    testApp = std::make_unique<SimpleTestApp>();
    try {
        testApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Test app error: {}", e.what());
        return 1;
    }
    
    velocitas::logger().info("👋 TEST APP stopped");
    return 0;
}