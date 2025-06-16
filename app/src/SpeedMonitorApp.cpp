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
#include <fmt/format.h>

// Create global Vehicle instance
::vehicle::Vehicle Vehicle;

using namespace speedmonitor;

SpeedMonitorApp::SpeedMonitorApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("SpeedMonitorApp")) {
    velocitas::logger().info("Creating Speed Monitor App");
}

void SpeedMonitorApp::onStart() {
    velocitas::logger().info("Speed Monitor App started - monitoring Vehicle.Speed");
    
    // Subscribe to vehicle speed changes
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("Error in Speed Monitor App: {}", status.errorMessage());
        });
    
    // Publish initial status
    publishStatus();
}

void SpeedMonitorApp::onSpeedChanged(const velocitas::DataPointReply& reply) {
    try {
        auto speedValue = reply.get(Vehicle.Speed)->value();
        m_currentSpeedKmh = speedValue;  // Keep as m/s, just rename variable later
        
        velocitas::logger().info("Current speed: {:.1f} m/s", m_currentSpeedKmh);
        
        // Check speed limit (now in m/s)
        if (m_currentSpeedKmh > m_config.speedLimitKmh) {
            std::string alertMessage = fmt::format(
                "SPEED ALERT: {:.1f} m/s exceeds limit of {:.1f} m/s", 
                m_currentSpeedKmh, m_config.speedLimitKmh
            );
            velocitas::logger().warn(alertMessage);
            publishAlert(alertMessage);
        }
        
        // Publish current status
        publishStatus();
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("Speed data not available - waiting for vehicle data");
    }
}

void SpeedMonitorApp::publishAlert(const std::string& message) {
    std::string alertData = fmt::format(R"({{
        "timestamp": "{}",
        "type": "speed_limit_exceeded", 
        "message": "{}",
        "current_speed_ms": {:.1f},
        "speed_limit_ms": {:.1f}
    }})", 
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count(),
        message, m_currentSpeedKmh, m_config.speedLimitKmh
    );
    
    publishToTopic("speedmonitor/alerts", alertData);
    velocitas::logger().info("Alert published to MQTT");
}

void SpeedMonitorApp::publishStatus() {
    std::string statusData = fmt::format(R"({{
        "timestamp": "{}",
        "current_speed_ms": {:.1f},
        "speed_limit_ms": {:.1f},
        "status": "{}"
    }})", 
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count(),
        m_currentSpeedKmh, m_config.speedLimitKmh,
        (m_currentSpeedKmh > m_config.speedLimitKmh) ? "OVER_LIMIT" : "OK"
    );
    
    publishToTopic("speedmonitor/status", statusData);
}