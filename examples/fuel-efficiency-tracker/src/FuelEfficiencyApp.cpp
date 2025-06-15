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

#include "FuelEfficiencyApp.h"
#include "sdk/IPubSubClient.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <numeric>
#include <algorithm>
#include <cmath>

namespace fueltracker {

FuelEfficiencyApp::FuelEfficiencyApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("FuelEfficiencyApp")) {
    m_lastAlertTime = std::chrono::system_clock::now() - std::chrono::milliseconds(m_config.alertCooldownMs);
    m_lastEfficiencyUpdate = std::chrono::system_clock::now();
}

void FuelEfficiencyApp::onStart() {
    velocitas::logger().info("Fuel Efficiency Tracker starting...");

    // Subscribe to fuel consumption data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Powertrain.FuelSystem.InstantConsumption).build())
        ->onItem([this](auto&& item) { onFuelConsumptionChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to speed data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to distance data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.TraveledDistance).build())
        ->onItem([this](auto&& item) { onDistanceChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to optional engine RPM
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Powertrain.CombustionEngine.Speed).build())
        ->onItem([this](auto&& item) { onEngineRpmChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().debug("Engine RPM data not available: {}", status.errorMessage()); 
        });

    // Subscribe to optional gear data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Powertrain.Transmission.CurrentGear).build())
        ->onItem([this](auto&& item) { onGearChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().debug("Gear data not available: {}", status.errorMessage()); 
        });

    // Subscribe to MQTT topics
    subscribeToTopic(TOPIC_CONFIG)
        ->onItem([this](auto&& data) { onConfigReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    subscribeToTopic(TOPIC_TRIP_START)
        ->onItem([this](auto&& data) { onTripStartReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    subscribeToTopic(TOPIC_TRIP_END)
        ->onItem([this](auto&& data) { onTripEndReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    subscribeToTopic(TOPIC_RESET)
        ->onItem([this](auto&& data) { onResetReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    velocitas::logger().info("Fuel Efficiency Tracker started successfully");
    publishStatus();
}

void FuelEfficiencyApp::onFuelConsumptionChanged(const velocitas::DataPointReply& reply) {
    try {
        auto consumptionValue = reply.get(Vehicle.Powertrain.FuelSystem.InstantConsumption)->value();
        m_currentConsumption = consumptionValue; // L/h
        
        velocitas::logger().debug("Fuel consumption changed: {:.2f} L/h", m_currentConsumption);
        
        calculateEfficiency();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing fuel consumption data: {}", e.what());
    }
}

void FuelEfficiencyApp::onSpeedChanged(const velocitas::DataPointReply& reply) {
    try {
        auto speedValue = reply.get(Vehicle.Speed)->value();
        m_currentSpeed = speedValue * 3.6; // Convert m/s to km/h
        
        velocitas::logger().debug("Speed changed: {:.2f} km/h", m_currentSpeed);
        
        calculateEfficiency();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing speed data: {}", e.what());
    }
}

void FuelEfficiencyApp::onDistanceChanged(const velocitas::DataPointReply& reply) {
    try {
        auto distanceValue = reply.get(Vehicle.TraveledDistance)->value();
        m_currentDistance = distanceValue / 1000.0; // Convert m to km
        
        velocitas::logger().debug("Distance changed: {:.2f} km", m_currentDistance);
        
        // Update trip distance if trip is active
        if (m_currentTrip.isActive && m_currentTrip.startDistance > 0) {
            m_currentTrip.totalDistance = m_currentDistance - m_currentTrip.startDistance;
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing distance data: {}", e.what());
    }
}

void FuelEfficiencyApp::onEngineRpmChanged(const velocitas::DataPointReply& reply) {
    try {
        auto rpmValue = reply.get(Vehicle.Powertrain.CombustionEngine.Speed)->value();
        m_currentRpm = rpmValue;
        
        velocitas::logger().debug("Engine RPM changed: {:.0f} RPM", m_currentRpm);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing RPM data: {}", e.what());
    }
}

void FuelEfficiencyApp::onGearChanged(const velocitas::DataPointReply& reply) {
    try {
        auto gearValue = reply.get(Vehicle.Powertrain.Transmission.CurrentGear)->value();
        m_currentGear = static_cast<int>(gearValue);
        
        velocitas::logger().debug("Gear changed: {}", m_currentGear);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing gear data: {}", e.what());
    }
}

void FuelEfficiencyApp::calculateEfficiency() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastEfficiencyUpdate);
    
    // Only calculate every 2 seconds to avoid too frequent updates
    if (timeSinceLastUpdate.count() < 2000) {
        return;
    }
    
    m_lastEfficiencyUpdate = now;
    
    // Add current data point to rolling window
    if (m_currentSpeed > 0.1 && m_currentConsumption > 0.001) {
        m_efficiencyData.emplace_back(m_currentConsumption, m_currentSpeed, m_currentDistance);
        
        // Limit rolling window size
        while (m_efficiencyData.size() > static_cast<size_t>(m_config.efficiencyWindowSize)) {
            m_efficiencyData.pop_front();
        }
        
        updateRollingEfficiency();
        checkEfficiencyAlerts();
        
        // Publish efficiency data every 5th calculation
        static int calcCount = 0;
        if (++calcCount >= 5) {
            publishEfficiency();
            calcCount = 0;
        }
    }
}

void FuelEfficiencyApp::updateRollingEfficiency() {
    if (m_efficiencyData.size() < 2) {
        return;
    }
    
    // Calculate rolling efficiency over the window
    double totalDistance = 0.0;
    double totalFuelUsed = 0.0;
    
    for (size_t i = 1; i < m_efficiencyData.size(); ++i) {
        const auto& prev = m_efficiencyData[i-1];
        const auto& curr = m_efficiencyData[i];
        
        // Calculate time difference in hours
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(curr.timestamp - prev.timestamp);
        double timeHours = timeDiff.count() / (1000.0 * 3600.0);
        
        // Calculate distance traveled (approximation)
        double avgSpeed = (prev.speed + curr.speed) / 2.0;
        double distanceTraveled = avgSpeed * timeHours;
        
        // Calculate fuel used
        double avgConsumption = (prev.instantConsumption + curr.instantConsumption) / 2.0;
        double fuelUsed = avgConsumption * timeHours;
        
        totalDistance += distanceTraveled;
        totalFuelUsed += fuelUsed;
    }
    
    // Calculate current efficiency
    if (totalFuelUsed > 0.001 && totalDistance > 0.001) {
        m_stats.currentEfficiencyKmL = totalDistance / totalFuelUsed;
        
        // Update overall statistics
        m_stats.totalDistanceKm += totalDistance;
        m_stats.totalFuelUsedL += totalFuelUsed;
        
        if (m_stats.totalFuelUsedL > 0.001) {
            m_stats.averageEfficiencyKmL = m_stats.totalDistanceKm / m_stats.totalFuelUsedL;
        }
        
        // Update best/worst efficiency
        if (m_stats.currentEfficiencyKmL > m_stats.bestEfficiencyKmL) {
            m_stats.bestEfficiencyKmL = m_stats.currentEfficiencyKmL;
        }
        if (m_stats.currentEfficiencyKmL < m_stats.worstEfficiencyKmL) {
            m_stats.worstEfficiencyKmL = m_stats.currentEfficiencyKmL;
        }
        
        velocitas::logger().debug("Current efficiency: {:.2f} km/L", m_stats.currentEfficiencyKmL);
    }
}

void FuelEfficiencyApp::checkEfficiencyAlerts() {
    if (!m_config.enableEfficiencyAlerts || !canSendAlert()) {
        return;
    }
    
    // Check for low efficiency
    if (m_stats.currentEfficiencyKmL > 0 && m_stats.currentEfficiencyKmL < m_config.lowEfficiencyThreshold) {
        std::string message = fmt::format(
            "Low fuel efficiency detected: {:.2f} km/L (target: {:.2f} km/L)",
            m_stats.currentEfficiencyKmL, m_config.targetEfficiencyKmL
        );
        
        publishAlert("low_efficiency", message, "warning");
        
        if (m_config.enableEcoTips) {
            auto tips = generateEcoTips();
            for (const auto& tip : tips) {
                publishAlert("eco_tip", tip, "info");
            }
        }
        
        m_lastAlertTime = std::chrono::system_clock::now();
    }
    
    // Check for high consumption
    double consumptionPer100km = efficiencyToConsumption(m_stats.currentEfficiencyKmL);
    if (consumptionPer100km > m_config.highConsumptionThreshold) {
        std::string message = fmt::format(
            "High fuel consumption: {:.2f} L/100km",
            consumptionPer100km
        );
        
        publishAlert("high_consumption", message, "warning");
        m_lastAlertTime = std::chrono::system_clock::now();
    }
}

std::vector<std::string> FuelEfficiencyApp::generateEcoTips() {
    std::vector<std::string> tips;
    
    // Speed-based tips
    if (m_currentSpeed > 100) {
        tips.push_back("Reduce speed - driving over 100 km/h significantly increases fuel consumption");
    } else if (m_currentSpeed < 50 && m_currentSpeed > 10) {
        tips.push_back("Maintain steady speed around 80-90 km/h for optimal fuel efficiency");
    }
    
    // RPM-based tips
    if (m_currentRpm > 3000) {
        tips.push_back("Shift to higher gear - high RPM increases fuel consumption");
    } else if (m_currentRpm > 0 && m_currentRpm < 1500 && m_currentSpeed > 50) {
        tips.push_back("Consider downshifting - very low RPM at high speed reduces efficiency");
    }
    
    // Gear-based tips
    if (m_currentGear > 0 && m_currentSpeed > 0) {
        double speedPerGear = m_currentSpeed / m_currentGear;
        if (speedPerGear < 15) {
            tips.push_back("Shift to higher gear for better fuel efficiency");
        }
    }
    
    // General tips
    if (tips.empty()) {
        tips.push_back("Maintain steady acceleration and avoid aggressive driving");
        tips.push_back("Plan routes to avoid traffic congestion");
        tips.push_back("Remove excess weight from vehicle");
    }
    
    return tips;
}

void FuelEfficiencyApp::startTrip(const std::string& tripName) {
    if (m_currentTrip.isActive) {
        endTrip(); // End current trip before starting new one
    }
    
    m_currentTrip = TripData();
    m_currentTrip.isActive = true;
    m_currentTrip.startTime = std::chrono::system_clock::now();
    m_currentTrip.startDistance = m_currentDistance;
    
    velocitas::logger().info("Trip started: {}", tripName.empty() ? "Unnamed Trip" : tripName);
    publishAlert("trip_started", fmt::format("Trip started: {}", tripName.empty() ? "Unnamed Trip" : tripName), "info");
}

void FuelEfficiencyApp::endTrip() {
    if (!m_currentTrip.isActive) {
        velocitas::logger().warn("No active trip to end");
        return;
    }
    
    m_currentTrip.isActive = false;
    m_currentTrip.endTime = std::chrono::system_clock::now();
    m_currentTrip.endDistance = m_currentDistance;
    m_currentTrip.totalDistance = m_currentTrip.endDistance - m_currentTrip.startDistance;
    
    // Calculate trip statistics
    auto tripDuration = std::chrono::duration_cast<std::chrono::seconds>(
        m_currentTrip.endTime - m_currentTrip.startTime).count();
    
    if (tripDuration > 0) {
        m_currentTrip.averageSpeed = (m_currentTrip.totalDistance / tripDuration) * 3600.0; // km/h
    }
    
    // Calculate fuel efficiency for trip (simplified)
    if (!m_efficiencyData.empty()) {
        m_currentTrip.efficiency = m_stats.currentEfficiencyKmL;
    }
    
    m_stats.tripCount++;
    
    velocitas::logger().info("Trip ended - Distance: {:.2f} km, Efficiency: {:.2f} km/L", 
                           m_currentTrip.totalDistance, m_currentTrip.efficiency);
    
    publishTripSummary();
    publishAlert("trip_ended", fmt::format("Trip completed - {:.2f} km, {:.2f} km/L", 
                                         m_currentTrip.totalDistance, m_currentTrip.efficiency), "info");
}

void FuelEfficiencyApp::onConfigReceived(const std::string& data) {
    velocitas::logger().info("Configuration update received: {}", data);
    
    if (parseConfig(data)) {
        velocitas::logger().info("Configuration updated successfully");
        publishStatus();
    } else {
        velocitas::logger().error("Failed to parse configuration");
        publishAlert("config_error", "Invalid configuration received", "error");
    }
}

void FuelEfficiencyApp::onTripStartReceived(const std::string& data) {
    try {
        auto json = nlohmann::json::parse(data);
        std::string tripName = json.value("name", "");
        startTrip(tripName);
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing trip start: {}", e.what());
        startTrip(); // Start with default name
    }
}

void FuelEfficiencyApp::onTripEndReceived(const std::string& data) {
    endTrip();
}

void FuelEfficiencyApp::onResetReceived(const std::string& data) {
    try {
        auto json = nlohmann::json::parse(data);
        std::string resetType = json.value("type", "statistics");
        
        if (resetType == "statistics") {
            m_stats = EfficiencyStats();
            m_efficiencyData.clear();
            velocitas::logger().info("Statistics reset");
            publishAlert("reset", "Statistics have been reset", "info");
        } else if (resetType == "config") {
            m_config = FuelConfig();
            velocitas::logger().info("Configuration reset to defaults");
            publishAlert("reset", "Configuration reset to defaults", "info");
        } else if (resetType == "trip") {
            if (m_currentTrip.isActive) {
                endTrip();
            }
            velocitas::logger().info("Current trip ended");
        }
        
        publishStatus();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing reset command: {}", e.what());
        publishAlert("reset_error", "Failed to process reset command", "error");
    }
}

void FuelEfficiencyApp::onError(const velocitas::Status& status) {
    velocitas::logger().error("Error in Fuel Efficiency App: {}", status.errorMessage());
    publishAlert("system_error", fmt::format("System error: {}", status.errorMessage()), "error");
}

void FuelEfficiencyApp::publishEfficiency() {
    try {
        nlohmann::json efficiencyJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"current_efficiency_kmL", m_stats.currentEfficiencyKmL},
            {"current_consumption_L100km", efficiencyToConsumption(m_stats.currentEfficiencyKmL)},
            {"average_efficiency_kmL", m_stats.averageEfficiencyKmL},
            {"best_efficiency_kmL", m_stats.bestEfficiencyKmL},
            {"instant_consumption_Lh", m_currentConsumption},
            {"current_speed_kmh", m_currentSpeed},
            {"target_efficiency_kmL", m_config.targetEfficiencyKmL}
        };
        
        publishToTopic(TOPIC_EFFICIENCY, efficiencyJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing efficiency data: {}", e.what());
    }
}

void FuelEfficiencyApp::publishStatus() {
    try {
        nlohmann::json statusJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"current_speed_kmh", m_currentSpeed},
            {"current_consumption_Lh", m_currentConsumption},
            {"current_efficiency_kmL", m_stats.currentEfficiencyKmL},
            {"total_distance_km", m_stats.totalDistanceKm},
            {"total_fuel_used_L", m_stats.totalFuelUsedL},
            {"trip_active", m_currentTrip.isActive},
            {"config", {
                {"target_efficiency_kmL", m_config.targetEfficiencyKmL},
                {"low_efficiency_threshold", m_config.lowEfficiencyThreshold},
                {"high_consumption_threshold", m_config.highConsumptionThreshold},
                {"efficiency_alerts_enabled", m_config.enableEfficiencyAlerts},
                {"trip_tracking_enabled", m_config.enableTripTracking},
                {"eco_tips_enabled", m_config.enableEcoTips}
            }},
            {"status", "active"}
        };
        
        publishToTopic(TOPIC_STATUS, statusJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing status: {}", e.what());
    }
}

void FuelEfficiencyApp::publishTripSummary() {
    try {
        auto tripDuration = std::chrono::duration_cast<std::chrono::seconds>(
            m_currentTrip.endTime - m_currentTrip.startTime).count();
        
        nlohmann::json tripJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                m_currentTrip.endTime.time_since_epoch()).count()},
            {"trip_duration_seconds", tripDuration},
            {"total_distance_km", m_currentTrip.totalDistance},
            {"average_speed_kmh", m_currentTrip.averageSpeed},
            {"max_speed_kmh", m_currentTrip.maxSpeed},
            {"trip_efficiency_kmL", m_currentTrip.efficiency},
            {"fuel_consumed_L", m_currentTrip.totalFuelConsumed}
        };
        
        publishToTopic(TOPIC_TRIP_SUMMARY, tripJson.dump());
        velocitas::logger().info("Trip summary published");
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing trip summary: {}", e.what());
    }
}

void FuelEfficiencyApp::publishAlert(const std::string& alertType, const std::string& message, const std::string& severity) {
    try {
        nlohmann::json alertJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"type", alertType},
            {"message", message},
            {"severity", severity},
            {"current_efficiency_kmL", m_stats.currentEfficiencyKmL},
            {"current_speed_kmh", m_currentSpeed}
        };
        
        publishToTopic(TOPIC_ALERTS, alertJson.dump());
        velocitas::logger().info("Alert published: {} - {}", alertType, message);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing alert: {}", e.what());
    }
}

bool FuelEfficiencyApp::canSendAlert() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastAlertTime);
    return timeSinceLastAlert.count() >= m_config.alertCooldownMs;
}

double FuelEfficiencyApp::consumptionToEfficiency(double consumptionPer100km) {
    if (consumptionPer100km <= 0) return 0.0;
    return 100.0 / consumptionPer100km;
}

double FuelEfficiencyApp::efficiencyToConsumption(double efficiencyKmL) {
    if (efficiencyKmL <= 0) return 999.0;
    return 100.0 / efficiencyKmL;
}

bool FuelEfficiencyApp::parseConfig(const std::string& configJson) {
    try {
        auto json = nlohmann::json::parse(configJson);
        
        if (json.contains("target_efficiency_kmL")) {
            m_config.targetEfficiencyKmL = json["target_efficiency_kmL"].get<double>();
        }
        if (json.contains("low_efficiency_threshold")) {
            m_config.lowEfficiencyThreshold = json["low_efficiency_threshold"].get<double>();
        }
        if (json.contains("high_consumption_threshold")) {
            m_config.highConsumptionThreshold = json["high_consumption_threshold"].get<double>();
        }
        if (json.contains("efficiency_window_size")) {
            m_config.efficiencyWindowSize = json["efficiency_window_size"].get<int>();
        }
        if (json.contains("alert_cooldown_ms")) {
            m_config.alertCooldownMs = json["alert_cooldown_ms"].get<int>();
        }
        if (json.contains("enable_efficiency_alerts")) {
            m_config.enableEfficiencyAlerts = json["enable_efficiency_alerts"].get<bool>();
        }
        if (json.contains("enable_trip_tracking")) {
            m_config.enableTripTracking = json["enable_trip_tracking"].get<bool>();
        }
        if (json.contains("enable_eco_tips")) {
            m_config.enableEcoTips = json["enable_eco_tips"].get<bool>();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error parsing configuration JSON: {}", e.what());
        return false;
    }
}

} // namespace fueltracker