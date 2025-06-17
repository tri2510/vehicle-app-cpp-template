// ============================================================================
// QUICK BUILD TEMPLATE - Placeholder File
// ============================================================================
// This file will be replaced by user input during quick build process.
// The actual VehicleApp.cpp content comes from stdin or mounted file.
// ============================================================================

#include "sdk/VehicleApp.h"
#include "sdk/Logger.h"
#include "vehicle/Vehicle.hpp"

class QuickBuildPlaceholder : public velocitas::VehicleApp {
public:
    QuickBuildPlaceholder() : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {}

protected:
    void onStart() override {
        velocitas::logger().info("Quick Build Placeholder - This should not run");
        velocitas::logger().warn("If you see this message, the user input replacement failed");
    }
};

DEFINE_VEHICLE_APP(QuickBuildPlaceholder)