/**
 * Copyright (c) 2023-2025 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SampleApp.h"
#include "sdk/IPubSubClient.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <utility>
#include <ctime>
#include <thread>
#include <chrono>

namespace example {

const auto RESET_REQUEST_TOPIC     = "vehiclecontrol/reset";
const auto RESET_RESPONSE_TOPIC    = "vehiclecontrol/reset/response";
const auto STATUS_REQUEST_TOPIC    = "vehiclecontrol/status";
const auto STATUS_RESPONSE_TOPIC   = "vehiclecontrol/status/response";
const auto SEQUENCE_STARTED_TOPIC  = "vehiclecontrol/sequence/started";
const auto SEQUENCE_COMPLETED_TOPIC = "vehiclecontrol/sequence/completed";

TestApp::TestApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("VehicleControlApp")) {}

void TestApp::onStart() {
    velocitas::logger().info("TestApp starting - Vehicle Control Sequence");
    
    // Subscribe to reset requests
    subscribeToTopic(RESET_REQUEST_TOPIC)
        ->onItem([this](auto&& data) { onResetRequestReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to status requests  
    subscribeToTopic(STATUS_REQUEST_TOPIC)
        ->onItem([this](auto&& data) { onStatusRequestReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Start the automated sequence
    executeSequence();
}

void TestApp::resetAll() {
    velocitas::logger().info("Resetting all vehicle components to default state");
    
    try {
        // Reset doors to closed
        Vehicle.Cabin.Door.Row1.DriverSide.IsOpen.set(false)->await();
        Vehicle.Cabin.Door.Row1.PassengerSide.IsOpen.set(false)->await();
        Vehicle.Body.Trunk.Rear.IsOpen.set(false)->await();
        
        // Reset seat positions to 0
        Vehicle.Cabin.Seat.Row1.DriverSide.Position.set(0)->await();
        Vehicle.Cabin.Seat.Row1.PassengerSide.Position.set(0)->await();
        
        velocitas::logger().info("All components reset successfully");
    } catch (const std::exception& e) {
        velocitas::logger().error("Error during reset: {}", e.what());
    }
}

void TestApp::executeSequence() {
    velocitas::logger().info("Starting vehicle control sequence");
    
    // Publish sequence started event
    nlohmann::json startedJson({{"event", "SEQUENCE_STARTED"}, {"timestamp", std::time(nullptr)}});
    publishToTopic(SEQUENCE_STARTED_TOPIC, startedJson.dump());
    
    try {
        // Step 1: Reset all components
        resetAll();
        
        // Step 2: Wait 2 seconds (like Python asyncio.sleep(2))
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Step 3: Open driver door
        velocitas::logger().info("Opening driver door");
        Vehicle.Cabin.Door.Row1.DriverSide.IsOpen.set(true)->await();
        
        // Step 4: Wait 3 seconds  
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        // Step 5: Adjust driver seat position
        velocitas::logger().info("Adjusting driver seat position to 10");
        Vehicle.Cabin.Seat.Row1.DriverSide.Position.set(10)->await();
        
        velocitas::logger().info("Vehicle control sequence completed successfully");
        
        // Publish sequence completed event
        nlohmann::json completedJson({{"event", "SEQUENCE_COMPLETED"}, {"timestamp", std::time(nullptr)}});
        publishToTopic(SEQUENCE_COMPLETED_TOPIC, completedJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error during sequence execution: {}", e.what());
    }
}

void TestApp::onResetRequestReceived(const std::string& data) {
    velocitas::logger().info("Reset request received: {}", data);
    
    resetAll();
    
    nlohmann::json response({{"result", {{"status", 0}, {"message", "Vehicle reset completed"}}}});
    publishToTopic(RESET_RESPONSE_TOPIC, response.dump());
}

void TestApp::onStatusRequestReceived(const std::string& data) {
    velocitas::logger().info("Status request received: {}", data);
    
    try {
        // Get current state of all components
        auto driverDoorOpen = Vehicle.Cabin.Door.Row1.DriverSide.IsOpen.get()->await().value();
        auto passengerDoorOpen = Vehicle.Cabin.Door.Row1.PassengerSide.IsOpen.get()->await().value();
        auto trunkOpen = Vehicle.Body.Trunk.Rear.IsOpen.get()->await().value();
        auto driverSeatPos = Vehicle.Cabin.Seat.Row1.DriverSide.Position.get()->await().value();
        auto passengerSeatPos = Vehicle.Cabin.Seat.Row1.PassengerSide.Position.get()->await().value();
        
        nlohmann::json status({
            {"driver_door_open", driverDoorOpen},
            {"passenger_door_open", passengerDoorOpen}, 
            {"trunk_open", trunkOpen},
            {"driver_seat_position", driverSeatPos},
            {"passenger_seat_position", passengerSeatPos},
            {"timestamp", std::time(nullptr)}
        });
        
        nlohmann::json response({{"result", {{"status", 0}, {"data", status}}}});
        publishToTopic(STATUS_RESPONSE_TOPIC, response.dump());
        
    } catch (const std::exception& e) {
        nlohmann::json response({{"result", {{"status", -1}, {"error", e.what()}}}});
        publishToTopic(STATUS_RESPONSE_TOPIC, response.dump());
    }
}

void TestApp::onError(const velocitas::Status& status) {
    velocitas::logger().error("Error occurred during async invocation: {}", status.errorMessage());
}

} // namespace example
