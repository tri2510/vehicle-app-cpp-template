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

#ifndef VEHICLE_APP_SDK_TESTAPP_EXAMPLE_H
#define VEHICLE_APP_SDK_TESTAPP_EXAMPLE_H

#include "sdk/Status.h"
#include "sdk/VehicleApp.h"
#include "vehicle/Vehicle.hpp"

#include <memory>
#include <string>

namespace example {

/**
 * @brief Vehicle Control Test App.
 * @details Controls vehicle components (doors, seats, trunk) in sequence.
 *      Equivalent to Python TestApp that resets vehicle state and 
 *      performs automated door/seat operations.
 */
class TestApp : public velocitas::VehicleApp {
public:
    TestApp();

    /**
     * @brief Run when the vehicle app starts
     *
     */
    void onStart() override;

    /**
     * @brief Reset all vehicle components to default state
     *
     */
    void resetAll();

    /**
     * @brief Execute the vehicle control sequence
     *
     */
    void executeSequence();

    /**
     * @brief Handle reset request from PubSub topic
     *
     * @param data  The JSON string received from PubSub topic.
     */
    void onResetRequestReceived(const std::string& data);

    /**
     * @brief Handle status request from PubSub topic
     *
     * @param data  The JSON string received from PubSub topic.
     */
    void onStatusRequestReceived(const std::string& data);

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

#endif // VEHICLE_APP_SDK_TESTAPP_EXAMPLE_H
