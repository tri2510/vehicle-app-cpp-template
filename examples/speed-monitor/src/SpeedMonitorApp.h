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
#include "vehicle_model/Vehicle.hpp"

#include <chrono>
#include <memory>

namespace speedmonitor {

/**
 * @brief Configuration structure for speed monitoring
 */
struct SpeedConfig {
    double speedLimitKmh = 80.0;        // Default speed limit in km/h
    double hardBrakingThreshold = -5.0;  // m/s² - negative for deceleration
    double rapidAccelThreshold = 3.0;    // m/s² - positive for acceleration
    int alertCooldownMs = 5000;          // Cooldown between alerts in milliseconds
    bool enableSpeedLimitAlerts = true;
    bool enableAccelerationAlerts = true;
    bool enableLocationTracking = false;
};

/**
 * @brief Statistics for speed monitoring
 */
struct SpeedStatistics {
    double maxSpeedKmh = 0.0;
    double avgSpeedKmh = 0.0;
    int speedLimitViolations = 0;
    int hardBrakingEvents = 0;
    int rapidAccelEvents = 0;
    std::chrono::system_clock::time_point sessionStart;
    
    SpeedStatistics() : sessionStart(std::chrono::system_clock::now()) {}
};

/**
 * @brief Speed Monitor Vehicle Application
 * 
 * This application monitors vehicle speed and acceleration to provide:
 * - Speed limit violation alerts
 * - Hard braking detection
 * - Rapid acceleration detection
 * - Speed statistics tracking
 * - Configurable thresholds via MQTT
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
    /**
     * @brief Handles speed changes from Vehicle.Speed signal
     * @param reply DataPoint reply containing speed data
     */
    void onSpeedChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles acceleration changes from Vehicle.Acceleration.Longitudinal signal
     * @param reply DataPoint reply containing acceleration data
     */
    void onAccelerationChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles configuration updates via MQTT
     * @param data JSON configuration data
     */
    void onConfigReceived(const std::string& data);

    /**
     * @brief Handles reset commands via MQTT
     * @param data Reset command data
     */
    void onResetReceived(const std::string& data);

    /**
     * @brief Handles errors from vehicle data subscriptions
     * @param status Error status
     */
    void onError(const velocitas::Status& status);

    /**
     * @brief Checks if speed limit is violated and sends alert if needed
     * @param currentSpeed Current vehicle speed in km/h
     */
    void checkSpeedLimit(double currentSpeed);

    /**
     * @brief Checks for hard braking or rapid acceleration events
     * @param acceleration Current longitudinal acceleration in m/s²
     */
    void checkAccelerationEvents(double acceleration);

    /**
     * @brief Publishes an alert message to MQTT
     * @param alertType Type of alert (e.g., "speed_limit", "hard_braking")
     * @param message Alert message
     * @param severity Alert severity level
     */
    void publishAlert(const std::string& alertType, const std::string& message, const std::string& severity = "warning");

    /**
     * @brief Publishes current status to MQTT
     */
    void publishStatus();

    /**
     * @brief Publishes current statistics to MQTT
     */
    void publishStatistics();

    /**
     * @brief Updates speed statistics
     * @param speed Current speed in km/h
     */
    void updateStatistics(double speed);

    /**
     * @brief Checks if enough time has passed since last alert
     * @return true if alert can be sent, false if still in cooldown
     */
    bool canSendAlert();

    /**
     * @brief Converts m/s to km/h
     * @param speedMs Speed in m/s
     * @return Speed in km/h
     */
    static double msToKmh(double speedMs);

    /**
     * @brief Parses configuration JSON string
     * @param configJson JSON configuration string
     * @return true if parsing successful, false otherwise
     */
    bool parseConfig(const std::string& configJson);

private:
    // Configuration
    SpeedConfig m_config;
    
    // Statistics
    SpeedStatistics m_stats;
    
    // State tracking
    double m_currentSpeedKmh = 0.0;
    double m_currentAcceleration = 0.0;
    std::chrono::system_clock::time_point m_lastAlertTime;
    
    // Speed tracking for average calculation
    std::vector<double> m_speedSamples;
    static constexpr size_t MAX_SPEED_SAMPLES = 100;
    
    // MQTT topic constants
    static constexpr const char* TOPIC_CONFIG = "speedmonitor/config";
    static constexpr const char* TOPIC_RESET = "speedmonitor/reset";
    static constexpr const char* TOPIC_ALERTS = "speedmonitor/alerts";
    static constexpr const char* TOPIC_STATUS = "speedmonitor/status";
    static constexpr const char* TOPIC_STATISTICS = "speedmonitor/statistics";
};

} // namespace speedmonitor

#endif // SPEEDMONITORAPP_H