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

class CompilationErrorApp : public velocitas::VehicleApp {
public:
    CompilationErrorApp() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
        velocitas::logger().info("🔥 COMPILATION ERROR APP STARTING");
    }

protected:
    void onStart() override {
        velocitas::logger().info("Setting up signal subscriptions");
        
        subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
            ->onItem([this](auto&& item) { onTestDataChanged(std::forward<decltype(item)>(item)); })
            ->onError([this](auto&& status) { 
                velocitas::logger().error("Error: {}", status.errorMessage());
            });
    }

private:
    void onTestDataChanged(const velocitas::DataPointReply& reply) {
        try {
            auto speed = reply.get(Vehicle.Speed)->value() * 3.6f;
            velocitas::logger().info("Speed: {:.1f} km/h", speed);
            
            // INTENTIONAL COMPILATION ERROR - using undefined variable
            undefined_variable = speed;  // This will cause compilation error
            
        } catch (const std::exception& e) {
            velocitas::logger().debug("Waiting for data...");
        }
    }
};

std::unique_ptr<CompilationErrorApp> testApp;

void signal_handler(int sig) {
    velocitas::logger().info("App terminated");
    if (testApp) {
        testApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("Starting app with compilation error");
    
    testApp = std::make_unique<CompilationErrorApp>();
    try {
        testApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("App error: {}", e.what());
        return 1;
    }
    
    velocitas::logger().info("App stopped");
    return 0;
}