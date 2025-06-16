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

#include "SpeedMonitorApp.h"
#include "sdk/IPubSubClient.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <numeric>
#include <algorithm>

// Create global Vehicle instance
::vehicle::Vehicle Vehicle;

namespace speedmonitor {

SpeedMonitorApp::SpeedMonitorApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("SpeedMonitorApp")) {
    m_lastAlertTime = std::chrono::system_clock::now() - std::chrono::milliseconds(m_config.alertCooldownMs);
}

void SpeedMonitorApp::onStart() {
    velocitas::logger().info("Speed Monitor App starting...");

    // Subscribe to vehicle speed changes
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to acceleration changes if available
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Acceleration.Longitudinal).build())
        ->onItem([this](auto&& item) { onAccelerationChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().debug("Acceleration data not available: {}", status.errorMessage()); 
        });

    // Subscribe to configuration changes via MQTT
    subscribeToTopic(TOPIC_CONFIG)
        ->onItem([this](auto&& data) { onConfigReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to reset commands via MQTT
    subscribeToTopic(TOPIC_RESET)
        ->onItem([this](auto&& data) { onResetReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    velocitas::logger().info("Speed Monitor App started successfully");
    publishStatus();
}

void SpeedMonitorApp::onSpeedChanged(const velocitas::DataPointReply& reply) {
    try {
        auto speedValue = reply.get(Vehicle.Speed)->value();
        m_currentSpeedKmh = msToKmh(speedValue);
        
        velocitas::logger().debug("Speed changed: {:.2f} km/h", m_currentSpeedKmh);
        
        // Update statistics
        updateStatistics(m_currentSpeedKmh);
        
        // Check for speed limit violations
        if (m_config.enableSpeedLimitAlerts) {
            checkSpeedLimit(m_currentSpeedKmh);
        }
        
        // Publish status periodically (every 10th sample)
        static int sampleCount = 0;
        if (++sampleCount >= 10) {
            publishStatus();
            sampleCount = 0;
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing speed data: {}", e.what());
    }
}

void SpeedMonitorApp::onAccelerationChanged(const velocitas::DataPointReply& reply) {
    try {
        auto accelValue = reply.get(Vehicle.Acceleration.Longitudinal)->value();
        m_currentAcceleration = accelValue;
        
        velocitas::logger().debug("Acceleration changed: {:.2f} m/s²", m_currentAcceleration);
        
        // Check for hard braking or rapid acceleration
        if (m_config.enableAccelerationAlerts) {
            checkAccelerationEvents(m_currentAcceleration);
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing acceleration data: {}", e.what());
    }
}

void SpeedMonitorApp::onConfigReceived(const std::string& data) {
    velocitas::logger().info("Configuration update received: {}", data);
    
    if (parseConfig(data)) {
        velocitas::logger().info("Configuration updated successfully");
        publishStatus();
    } else {
        velocitas::logger().error("Failed to parse configuration");
        publishAlert("config_error", "Invalid configuration received", "error");
    }
}

void SpeedMonitorApp::onResetReceived(const std::string& data) {
    velocitas::logger().info("Reset command received: {}", data);
    
    try {
        auto json = nlohmann::json::parse(data);
        std::string resetType = json.value("type", "statistics");
        
        if (resetType == "statistics") {
            m_stats = SpeedStatistics();
            m_speedSamples.clear();
            velocitas::logger().info("Statistics reset");
            publishAlert("reset", "Statistics have been reset", "info");
        } else if (resetType == "config") {
            m_config = SpeedConfig();
            velocitas::logger().info("Configuration reset to defaults");
            publishAlert("reset", "Configuration reset to defaults", "info");
        }
        
        publishStatus();
        publishStatistics();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing reset command: {}", e.what());
        publishAlert("reset_error", "Failed to process reset command", "error");
    }
}

void SpeedMonitorApp::onError(const velocitas::Status& status) {
    velocitas::logger().error("Error in Speed Monitor App: {}", status.errorMessage());
    publishAlert("system_error", fmt::format("System error: {}", status.errorMessage()), "error");
}

void SpeedMonitorApp::checkSpeedLimit(double currentSpeed) {
    if (currentSpeed > m_config.speedLimitKmh && canSendAlert()) {
        m_stats.speedLimitViolations++;
        
        std::string message = fmt::format(
            "Speed limit exceeded: {:.1f} km/h (limit: {:.1f} km/h)",
            currentSpeed, m_config.speedLimitKmh
        );
        
        publishAlert("speed_limit", message, "warning");
        m_lastAlertTime = std::chrono::system_clock::now();
        
        velocitas::logger().warn(message);
    }
}

void SpeedMonitorApp::checkAccelerationEvents(double acceleration) {
    bool alertSent = false;
    
    // Check for hard braking (negative acceleration)
    if (acceleration <= m_config.hardBrakingThreshold && canSendAlert()) {
        m_stats.hardBrakingEvents++;
        
        std::string message = fmt::format(
            "Hard braking detected: {:.2f} m/s² (threshold: {:.2f} m/s²)",
            acceleration, m_config.hardBrakingThreshold
        );
        
        publishAlert("hard_braking", message, "warning");
        alertSent = true;
        
        velocitas::logger().warn(message);
    }
    
    // Check for rapid acceleration (positive acceleration)
    if (acceleration >= m_config.rapidAccelThreshold && canSendAlert() && !alertSent) {
        m_stats.rapidAccelEvents++;
        
        std::string message = fmt::format(
            "Rapid acceleration detected: {:.2f} m/s² (threshold: {:.2f} m/s²)",
            acceleration, m_config.rapidAccelThreshold
        );
        
        publishAlert("rapid_acceleration", message, "warning");
        alertSent = true;
        
        velocitas::logger().warn(message);
    }
    
    if (alertSent) {
        m_lastAlertTime = std::chrono::system_clock::now();
    }
}

void SpeedMonitorApp::publishAlert(const std::string& alertType, const std::string& message, const std::string& severity) {
    try {
        nlohmann::json alertJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"type", alertType},
            {"message", message},
            {"severity", severity},
            {"current_speed", m_currentSpeedKmh},
            {"current_acceleration", m_currentAcceleration}
        };
        
        publishToTopic(TOPIC_ALERTS, alertJson.dump());
        velocitas::logger().info("Alert published: {} - {}", alertType, message);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing alert: {}", e.what());
    }
}

void SpeedMonitorApp::publishStatus() {
    try {
        nlohmann::json statusJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"current_speed_kmh", m_currentSpeedKmh},
            {"current_acceleration", m_currentAcceleration},
            {"config", {
                {"speed_limit_kmh", m_config.speedLimitKmh},
                {"hard_braking_threshold", m_config.hardBrakingThreshold},
                {"rapid_accel_threshold", m_config.rapidAccelThreshold},
                {"alert_cooldown_ms", m_config.alertCooldownMs},
                {"speed_limit_alerts_enabled", m_config.enableSpeedLimitAlerts},
                {"acceleration_alerts_enabled", m_config.enableAccelerationAlerts},
                {"location_tracking_enabled", m_config.enableLocationTracking}
            }},
            {"status", "active"}
        };
        
        publishToTopic(TOPIC_STATUS, statusJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing status: {}", e.what());
    }
}

void SpeedMonitorApp::publishStatistics() {
    try {
        auto now = std::chrono::system_clock::now();
        auto sessionDuration = std::chrono::duration_cast<std::chrono::seconds>(now - m_stats.sessionStart).count();
        
        nlohmann::json statsJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()},
            {"session_duration_seconds", sessionDuration},
            {"max_speed_kmh", m_stats.maxSpeedKmh},
            {"average_speed_kmh", m_stats.avgSpeedKmh},
            {"speed_limit_violations", m_stats.speedLimitViolations},
            {"hard_braking_events", m_stats.hardBrakingEvents},
            {"rapid_acceleration_events", m_stats.rapidAccelEvents},
            {"total_samples", m_speedSamples.size()}
        };
        
        publishToTopic(TOPIC_STATISTICS, statsJson.dump());
        velocitas::logger().info("Statistics published");
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing statistics: {}", e.what());
    }
}

void SpeedMonitorApp::updateStatistics(double speed) {
    // Update maximum speed
    m_stats.maxSpeedKmh = std::max(m_stats.maxSpeedKmh, speed);
    
    // Add to speed samples for average calculation
    m_speedSamples.push_back(speed);
    
    // Limit the number of samples to prevent memory growth
    if (m_speedSamples.size() > MAX_SPEED_SAMPLES) {
        m_speedSamples.erase(m_speedSamples.begin());
    }
    
    // Calculate average speed
    if (!m_speedSamples.empty()) {
        double sum = std::accumulate(m_speedSamples.begin(), m_speedSamples.end(), 0.0);
        m_stats.avgSpeedKmh = sum / m_speedSamples.size();
    }
}

bool SpeedMonitorApp::canSendAlert() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastAlertTime);
    return timeSinceLastAlert.count() >= m_config.alertCooldownMs;
}

double SpeedMonitorApp::msToKmh(double speedMs) {
    return speedMs * 3.6; // Convert m/s to km/h
}

bool SpeedMonitorApp::parseConfig(const std::string& configJson) {
    try {
        auto json = nlohmann::json::parse(configJson);
        
        if (json.contains("speed_limit_kmh")) {
            m_config.speedLimitKmh = json["speed_limit_kmh"].get<double>();
        }
        if (json.contains("hard_braking_threshold")) {
            m_config.hardBrakingThreshold = json["hard_braking_threshold"].get<double>();
        }
        if (json.contains("rapid_accel_threshold")) {
            m_config.rapidAccelThreshold = json["rapid_accel_threshold"].get<double>();
        }
        if (json.contains("alert_cooldown_ms")) {
            m_config.alertCooldownMs = json["alert_cooldown_ms"].get<int>();
        }
        if (json.contains("enable_speed_limit_alerts")) {
            m_config.enableSpeedLimitAlerts = json["enable_speed_limit_alerts"].get<bool>();
        }
        if (json.contains("enable_acceleration_alerts")) {
            m_config.enableAccelerationAlerts = json["enable_acceleration_alerts"].get<bool>();
        }
        if (json.contains("enable_location_tracking")) {
            m_config.enableLocationTracking = json["enable_location_tracking"].get<bool>();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error parsing configuration JSON: {}", e.what());
        return false;
    }
}

} // namespace speedmonitor