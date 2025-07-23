#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include <iostream>

using namespace velocitas;

class VehicleApp : public App {
public:
    void onStart() override {
        std::cout << "=== My Vehicle App Started ===" << std::endl;
        
        // Monitor vehicle speed
        Vehicle.Speed->subscribe(DataPointReply::newBuilder()->build())
        ->onResult([](auto&& result) {
            if (result.isOk()) {
                auto speed = result.value().getValue();
                std::cout << "🚗 Current Speed: " << speed << " km/h" << std::endl;
                
                if (speed > 80) {
                    std::cout << "⚠️  High speed detected!" << std::endl;
                }
            }
        });
        
        std::cout << "🚀 Speed monitoring active..." << std::endl;
    }
};

REGISTER_MAIN(VehicleApp)