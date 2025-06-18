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

class BrokenTestApp : public velocitas::VehicleApp {
public:
    BrokenTestApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
        velocitas::logger().info("🔥 BROKEN TEST APP STARTING");
    }

protected:
    void onStart() override {
        velocitas::logger().info("🔥 BROKEN APP: Setting up signal subscriptions");
        
        // INTENTIONAL SYNTAX ERROR - missing semicolon
        subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
            ->onItem([this](auto&& item) { onTestDataChanged(std::forward<decltype(item)>(item)); })
            ->onError([this](auto&& status) { 
                velocitas::logger().error("🔥 BROKEN APP error: {}", status.errorMessage());
            })  // <-- MISSING SEMICOLON HERE INTENTIONALLY
        
        velocitas::logger().info("✅ BROKEN APP: Signal subscription completed");
    }

private:
    void onTestDataChanged(const velocitas::DataPointReply& reply) {
        try {
            auto speed = reply.get(Vehicle.Speed)->value() * 3.6f;
            velocitas::logger().info("🔥 BROKEN APP SPEED: {:.1f} km/h", speed);
        } catch (const std::exception& e) {
            velocitas::logger().debug("🔥 BROKEN APP waiting for data...");
        }
    }
};

std::unique_ptr<BrokenTestApp> brokenApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 BROKEN APP terminated");
    if (brokenApp) {
        brokenApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🔥 STARTING BROKEN TEST APP - SHOULD FAIL TO COMPILE!");
    
    brokenApp = std::make_unique<BrokenTestApp>();
    try {
        brokenApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Broken app error: {}", e.what());
        return 1;
    }
    
    velocitas::logger().info("👋 BROKEN APP stopped");
    return 0;
}