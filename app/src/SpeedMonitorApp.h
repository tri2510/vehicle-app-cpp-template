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

#ifndef SPEEDMONITORAPP_H
#define SPEEDMONITORAPP_H

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Status.h"
#include "vehicle/Vehicle.hpp"

#include <chrono>
#include <memory>

namespace speedmonitor {

/**
 * @brief Simple configuration for speed monitoring
 */
struct SpeedConfig {
    double speedLimitKmh = 22.0;  // Default speed limit in m/s (approx 80 km/h)
};

/**
 * @brief Simple Speed Monitor Vehicle Application
 * 
 * Basic speed monitoring with:
 * - Speed limit checking (default: 80 km/h)
 * - MQTT alerts when speed limit exceeded
 */
class SpeedMonitorApp : public velocitas::VehicleApp {
public:
    SpeedMonitorApp();

protected:
    /**
     * @brief Called when the application starts and VDB connection is ready
     */
    void onStart() override;

private:
    void onSpeedChanged(const velocitas::DataPointReply& reply);
    void publishAlert(const std::string& message);
    void publishStatus();
    
    static double msToKmh(double speedMs) { return speedMs * 3.6; }

private:
    SpeedConfig m_config;
    double m_currentSpeedKmh = 0.0;
};

} // namespace speedmonitor

#endif // SPEEDMONITORAPP_H