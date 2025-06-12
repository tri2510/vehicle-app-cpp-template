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

namespace example {

const auto GET_SPEED_REQUEST_TOPIC       = "speedmonitor/getSpeed";
const auto GET_SPEED_RESPONSE_TOPIC      = "speedmonitor/getSpeed/response";
const auto DATABROKER_SUBSCRIPTION_TOPIC = "speedmonitor/currentSpeed";
const auto SPEED_ALERT_TOPIC             = "speedmonitor/alert";
const float SPEED_LIMIT_KMH              = 80.0f;

SampleApp::SampleApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("SpeedMonitorApp")) {}

void SampleApp::onStart() {
    // This method will be called by the SDK when the connection to the
    // Vehicle DataBroker is ready.
    // Here you can subscribe for the Vehicle Signals update (e.g. Vehicle Speed).
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // ... and, unlike Python, you have to manually subscribe to pub/sub topics
    subscribeToTopic(GET_SPEED_REQUEST_TOPIC)
        ->onItem(
            [this](auto&& data) { onGetSpeedRequestReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });
}

void SampleApp::onSpeedChanged(const velocitas::DataPointReply& reply) {
    // Get the current vehicle speed value from the received DatapointReply.
    auto vehicleSpeed = reply.get(Vehicle.Speed)->value();

    // Log current speed
    velocitas::logger().info("Current vehicle speed: {:.1f} km/h", vehicleSpeed);

    // Publish current speed data
    nlohmann::json speedJson({{"speed", vehicleSpeed}, {"timestamp", std::time(nullptr)}});
    publishToTopic(DATABROKER_SUBSCRIPTION_TOPIC, speedJson.dump());

    // Check if speed exceeds limit and send alert
    if (vehicleSpeed > SPEED_LIMIT_KMH) {
        velocitas::logger().warn("SPEED ALERT: Vehicle speed ({:.1f} km/h) exceeds limit ({:.1f} km/h)!", 
                                vehicleSpeed, SPEED_LIMIT_KMH);
        
        nlohmann::json alertJson({
            {"alert", "SPEED_LIMIT_EXCEEDED"},
            {"current_speed", vehicleSpeed},
            {"speed_limit", SPEED_LIMIT_KMH},
            {"timestamp", std::time(nullptr)}
        });
        publishToTopic(SPEED_ALERT_TOPIC, alertJson.dump());
    }
}

void SampleApp::onGetSpeedRequestReceived(const std::string& data) {
    // The subscribe_topic annotation is used to subscribe for incoming
    // PubSub events, e.g. MQTT event for GET_SPEED_REQUEST_TOPIC.

    // Use the logger with the preferred log level (e.g. debug, info, error, etc)
    velocitas::logger().debug("PubSub event for the Topic: {} -> is received with the data: {}",
                              GET_SPEED_REQUEST_TOPIC, data);

    // Getting current speed from VehicleDataBroker using the DataPoint getter.
    auto vehicleSpeed = Vehicle.Speed.get()->await().value();

    // Do anything with the speed value.
    // Example:
    // - Publish the vehicle speed to MQTT topic (i.e. GET_SPEED_RESPONSE_TOPIC).
    nlohmann::json response(
        {{"result",
          {{"status", 0}, {"message", fmt::format("Current Speed = {}", vehicleSpeed)}}}});
    publishToTopic(GET_SPEED_RESPONSE_TOPIC, response.dump());
}

void SampleApp::onError(const velocitas::Status& status) {
    velocitas::logger().error("Error occurred during async invocation: {}", status.errorMessage());
}

} // namespace example
