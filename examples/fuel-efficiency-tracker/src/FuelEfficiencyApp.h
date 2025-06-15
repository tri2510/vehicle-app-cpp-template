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

#ifndef FUELEFFICIENCYAPP_H
#define FUELEFFICIENCYAPP_H

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Status.h"
#include "vehicle_model/Vehicle.hpp"

#include <chrono>
#include <memory>
#include <deque>

namespace fueltracker {

/**
 * @brief Configuration for fuel efficiency tracking
 */
struct FuelConfig {
    double targetEfficiencyKmL = 15.0;    // Target fuel efficiency in km/L
    double lowEfficiencyThreshold = 8.0;  // Alert threshold for low efficiency
    double highConsumptionThreshold = 12.0; // Alert threshold for high consumption (L/100km)
    int efficiencyWindowSize = 50;        // Number of samples for rolling efficiency
    int alertCooldownMs = 30000;          // 30 seconds cooldown between alerts
    bool enableEfficiencyAlerts = true;
    bool enableTripTracking = true;
    bool enableEcoTips = true;
};

/**
 * @brief Trip statistics and data
 */
struct TripData {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    double startDistance = 0.0;          // km
    double endDistance = 0.0;            // km
    double totalDistance = 0.0;          // km
    double totalFuelConsumed = 0.0;      // L
    double averageSpeed = 0.0;           // km/h
    double maxSpeed = 0.0;               // km/h
    double efficiency = 0.0;             // km/L
    bool isActive = false;
    
    TripData() : startTime(std::chrono::system_clock::now()) {}
};

/**
 * @brief Fuel efficiency statistics
 */
struct EfficiencyStats {
    double currentEfficiencyKmL = 0.0;
    double averageEfficiencyKmL = 0.0;
    double bestEfficiencyKmL = 0.0;
    double worstEfficiencyKmL = 999.0;
    double totalDistanceKm = 0.0;
    double totalFuelUsedL = 0.0;
    int tripCount = 0;
    std::chrono::system_clock::time_point sessionStart;
    
    EfficiencyStats() : sessionStart(std::chrono::system_clock::now()) {}
};

/**
 * @brief Efficiency data point for rolling calculations
 */
struct EfficiencyPoint {
    std::chrono::system_clock::time_point timestamp;
    double instantConsumption;  // L/h
    double speed;              // km/h
    double distance;           // km
    
    EfficiencyPoint(double consumption, double spd, double dist)
        : timestamp(std::chrono::system_clock::now()), 
          instantConsumption(consumption), speed(spd), distance(dist) {}
};

/**
 * @brief Fuel Efficiency Tracker Vehicle Application
 * 
 * This application monitors and optimizes fuel consumption by providing:
 * - Real-time fuel efficiency calculation
 * - Trip-based fuel tracking
 * - Efficiency alerts and recommendations
 * - Historical statistics and trends
 * - Eco-driving tips based on driving patterns
 */
class FuelEfficiencyApp : public velocitas::VehicleApp {
public:
    FuelEfficiencyApp();

protected:
    /**
     * @brief Called when the application starts and VDB connection is ready
     */
    void onStart() override;

private:
    /**
     * @brief Handles instant fuel consumption changes
     * @param reply DataPoint reply containing consumption data
     */
    void onFuelConsumptionChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles speed changes
     * @param reply DataPoint reply containing speed data
     */
    void onSpeedChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles distance changes
     * @param reply DataPoint reply containing distance data
     */
    void onDistanceChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles engine RPM changes (optional)
     * @param reply DataPoint reply containing RPM data
     */
    void onEngineRpmChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles gear changes (optional)
     * @param reply DataPoint reply containing gear data
     */
    void onGearChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles configuration updates via MQTT
     * @param data JSON configuration data
     */
    void onConfigReceived(const std::string& data);

    /**
     * @brief Handles trip start commands
     * @param data Trip start data
     */
    void onTripStartReceived(const std::string& data);

    /**
     * @brief Handles trip end commands
     * @param data Trip end data
     */
    void onTripEndReceived(const std::string& data);

    /**
     * @brief Handles reset commands
     * @param data Reset command data
     */
    void onResetReceived(const std::string& data);

    /**
     * @brief Handles errors from vehicle data subscriptions
     * @param status Error status
     */
    void onError(const velocitas::Status& status);

    /**
     * @brief Calculates current fuel efficiency
     */
    void calculateEfficiency();

    /**
     * @brief Checks efficiency and sends alerts if needed
     */
    void checkEfficiencyAlerts();

    /**
     * @brief Updates rolling efficiency statistics
     */
    void updateRollingEfficiency();

    /**
     * @brief Starts a new trip
     * @param tripName Optional trip name
     */
    void startTrip(const std::string& tripName = "");

    /**
     * @brief Ends the current trip
     */
    void endTrip();

    /**
     * @brief Publishes current efficiency data
     */
    void publishEfficiency();

    /**
     * @brief Publishes current status
     */
    void publishStatus();

    /**
     * @brief Publishes trip summary
     */
    void publishTripSummary();

    /**
     * @brief Publishes an alert
     * @param alertType Type of alert
     * @param message Alert message
     * @param severity Alert severity
     */
    void publishAlert(const std::string& alertType, const std::string& message, const std::string& severity = "info");

    /**
     * @brief Generates eco-driving tips based on current data
     * @return Vector of eco-driving tips
     */
    std::vector<std::string> generateEcoTips();

    /**
     * @brief Checks if enough time has passed since last alert
     * @return true if alert can be sent
     */
    bool canSendAlert();

    /**
     * @brief Converts L/100km to km/L
     * @param consumptionPer100km Consumption in L/100km
     * @return Efficiency in km/L
     */
    static double consumptionToEfficiency(double consumptionPer100km);

    /**
     * @brief Converts km/L to L/100km
     * @param efficiencyKmL Efficiency in km/L
     * @return Consumption in L/100km
     */
    static double efficiencyToConsumption(double efficiencyKmL);

    /**
     * @brief Parses configuration JSON
     * @param configJson JSON configuration string
     * @return true if parsing successful
     */
    bool parseConfig(const std::string& configJson);

private:
    // Configuration
    FuelConfig m_config;
    
    // Statistics and data
    EfficiencyStats m_stats;
    TripData m_currentTrip;
    
    // Current sensor data
    double m_currentSpeed = 0.0;           // km/h
    double m_currentConsumption = 0.0;     // L/h
    double m_currentDistance = 0.0;        // km
    double m_currentRpm = 0.0;             // RPM
    int m_currentGear = 0;                 // Gear number
    
    // Rolling efficiency calculation
    std::deque<EfficiencyPoint> m_efficiencyData;
    std::chrono::system_clock::time_point m_lastEfficiencyUpdate;
    std::chrono::system_clock::time_point m_lastAlertTime;
    
    // MQTT topic constants
    static constexpr const char* TOPIC_CONFIG = "fueltracker/config";
    static constexpr const char* TOPIC_RESET = "fueltracker/reset";
    static constexpr const char* TOPIC_TRIP_START = "fueltracker/trip/start";
    static constexpr const char* TOPIC_TRIP_END = "fueltracker/trip/end";
    static constexpr const char* TOPIC_STATUS = "fueltracker/status";
    static constexpr const char* TOPIC_EFFICIENCY = "fueltracker/efficiency";
    static constexpr const char* TOPIC_TRIP_SUMMARY = "fueltracker/trip/summary";
    static constexpr const char* TOPIC_ALERTS = "fueltracker/alerts";
};

} // namespace fueltracker

#endif // FUELEFFICIENCYAPP_H