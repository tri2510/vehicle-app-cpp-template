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

#ifndef VEHICLE_APP_SDK_SEATADJUSTER_EXAMPLE_H
#define VEHICLE_APP_SDK_SEATADJUSTER_EXAMPLE_H

#include "sdk/Status.h"
#include "sdk/VehicleApp.h"
#include "vehicle/Vehicle.hpp"

#include <memory>
#include <string>

namespace example {

/**
 * @brief Speed Monitor vehicle app.
 * @details Monitors vehicle speed and sends alerts when speed exceeds 80 km/h.
 *      Subscribes to Vehicle.Speed from VehicleDataBroker and publishes
 *      speed alerts via MQTT topics.
 */
class SampleApp : public velocitas::VehicleApp {
public:
    SampleApp();

    /**
     * @brief Run when the vehicle app starts
     *
     */
    void onStart() override;

    /**
     * @brief Handle speed changed events from the VDB.
     *
     * @param dataPoints  The affected data points.
     */
    void onSpeedChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handle set position request from PubSub topic
     *
     * @param data  The JSON string received from PubSub topic.
     */
    void onGetSpeedRequestReceived(const std::string& data);

    /**
     * @brief Handle errors which occurred during async invocation.
     *
     * @param status  The status which contains the error.
     */
    void onError(const velocitas::Status& status);

private:
    vehicle::Vehicle Vehicle;
};

} // namespace example

#endif // VEHICLE_APP_SDK_SEATADJUSTER_EXAMPLE_H
