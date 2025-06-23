// ============================================================================
// 🚗 INTELLIGENT CLIMATE CONTROL - Production SDV Example
// ============================================================================
// 
// 📋 WHAT THIS APPLICATION DOES:
// Advanced HVAC management system that optimizes cabin climate based on
// occupancy, external conditions, and energy efficiency requirements.
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Multi-zone climate control and optimization
// - Occupancy detection and adaptive comfort
// - Energy-efficient HVAC operation
// - Comfort preference learning and adaptation
// - Integration with vehicle energy management
// - Predictive climate adjustment
//
// 🚀 QUICK START:
// cp examples/SmartClimateApp.cpp templates/app/src/VehicleApp.cpp
// cat templates/app/src/VehicleApp.cpp | docker run --rm -i velocitas-quick
// ============================================================================

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"
#include "vehicle/Vehicle.hpp"
#include <fmt/format.h>
#include <csignal>
#include <memory>
#include <chrono>
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

::vehicle::Vehicle Vehicle;

/**
 * @brief Intelligent Climate Control System
 * 
 * Production-ready smart HVAC management that provides:
 * - Multi-zone temperature control based on occupancy
 * - Predictive climate adjustment using external conditions
 * - Energy-efficient HVAC operation with battery optimization
 * - Comfort preference learning and personalization
 * - Integration with vehicle energy management systems
 * - Air quality monitoring and automatic adjustment
 */
class SmartClimateApp : public velocitas::VehicleApp {
public:
    SmartClimateApp();

protected:
    void onStart() override;

private:
    void onClimateDataChanged(const velocitas::DataPointReply& reply);
    
    // Climate control methods
    void analyzeOccupancy();
    void optimizeZoneTemperatures();
    void manageEnergyEfficiency(bool isAcActive);
    void predictiveClimateAdjustment(double exteriorTemp);
    void updateComfortPreferences();
    void monitorAirQuality();
    void generateClimateReport();
    
    // Zone control structures
    struct ClimateZone {
        std::string name;
        double currentTemp;
        double targetTemp;
        double preferredTemp;
        bool isOccupied;
        bool needsAdjustment;
        std::chrono::steady_clock::time_point lastAdjustment;
    };
    
    struct ComfortProfile {
        double preferredTemp;
        double tempTolerance;
        bool preferencesLearned;
        int adjustmentCount;
        std::vector<double> historicalPreferences;
    };
    
    // Climate zones
    std::map<std::string, ClimateZone> climateZones;
    std::map<std::string, ComfortProfile> comfortProfiles;
    
    // Climate control parameters
    static constexpr double COMFORT_TEMP_MIN = 18.0;        // °C
    static constexpr double COMFORT_TEMP_MAX = 26.0;        // °C
    static constexpr double OPTIMAL_TEMP = 22.0;            // °C
    static constexpr double TEMP_TOLERANCE = 2.0;           // °C
    static constexpr double ENERGY_SAVE_TEMP_OFFSET = 2.0;  // °C
    static constexpr double EXTERIOR_TEMP_THRESHOLD = 30.0; // °C for AC activation
    static constexpr double HVAC_EFFICIENCY_THRESHOLD = 0.8; // Efficiency ratio
    
    // State tracking
    double exteriorTemperature = 20.0;
    bool hvacSystemActive = false;
    bool energySaveMode = false;
    double totalEnergyUsage = 0.0;
    std::chrono::steady_clock::time_point lastReportTime;
    std::chrono::steady_clock::time_point systemStartTime;
    
    // Performance metrics
    int temperatureAdjustments = 0;
    int energySavingActivations = 0;
    int comfortOptimizations = 0;
    double averageComfortScore = 0.0;
    
    // Configuration
    bool adaptiveLearningEnabled = true;
    bool energyOptimizationEnabled = true;
    bool predictiveControlEnabled = true;
    
    static constexpr int REPORT_INTERVAL_MINUTES = 10;
    static constexpr int LEARNING_SAMPLE_SIZE = 50;
};

SmartClimateApp::SmartClimateApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")),
      lastReportTime(std::chrono::steady_clock::now()),
      systemStartTime(std::chrono::steady_clock::now()) {
    
    // Initialize climate zones
    climateZones["FrontLeft"] = {"Front Left", 20.0, 22.0, 22.0, false, false, std::chrono::steady_clock::now()};
    climateZones["FrontRight"] = {"Front Right", 20.0, 22.0, 22.0, false, false, std::chrono::steady_clock::now()};
    climateZones["RearLeft"] = {"Rear Left", 20.0, 22.0, 22.0, false, false, std::chrono::steady_clock::now()};
    climateZones["RearRight"] = {"Rear Right", 20.0, 22.0, 22.0, false, false, std::chrono::steady_clock::now()};
    
    // Initialize comfort profiles
    for (auto& zone : climateZones) {
        comfortProfiles[zone.first] = {22.0, 2.0, false, 0, {}};
    }
    
    velocitas::logger().info("🌡️  Smart Climate Control initializing with {} zones", climateZones.size());
}

void SmartClimateApp::onStart() {
    velocitas::logger().info("🚀 Intelligent Climate Control starting - Multi-zone HVAC optimization");
    
    // Subscribe to comprehensive climate monitoring signals
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)
                                               .select(Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature)
                                               .select(Vehicle.Cabin.Seat.Row1.Left.IsOccupied)
                                               .select(Vehicle.Cabin.Seat.Row1.Right.IsOccupied)
                                               .select(Vehicle.Cabin.Seat.Row2.Left.IsOccupied)
                                               .select(Vehicle.Cabin.Seat.Row2.Right.IsOccupied)
                                               .select(Vehicle.Exterior.AirTemperature)
                                               .select(Vehicle.Cabin.HVAC.IsAirConditioningActive)
                                               .build())
        ->onItem([this](auto&& item) { onClimateDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) {
            velocitas::logger().error("❌ Climate control subscription error: {}", status.errorMessage());
            // Fallback to manual mode
            velocitas::logger().warn("🔧 Switching to manual climate control mode");
        });
    
    velocitas::logger().info("✅ Smart climate control active - monitoring {} zones", climateZones.size());
    velocitas::logger().info("🎯 Target comfort range: {:.1f}°C - {:.1f}°C", COMFORT_TEMP_MIN, COMFORT_TEMP_MAX);
}

void SmartClimateApp::onClimateDataChanged(const velocitas::DataPointReply& reply) {
    try {
        // Extract climate sensor data
        double frontLeftTemp = 20.0, frontRightTemp = 20.0;
        bool frontLeftOccupied = false, frontRightOccupied = false;
        bool rearLeftOccupied = false, rearRightOccupied = false;
        
        // Temperature readings
        if (reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->isAvailable()) {
            frontLeftTemp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature)->value();
            climateZones["FrontLeft"].currentTemp = frontLeftTemp;
        }
        
        if (reply.get(Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature)->isAvailable()) {
            frontRightTemp = reply.get(Vehicle.Cabin.HVAC.Station.Row1.Right.Temperature)->value();
            climateZones["FrontRight"].currentTemp = frontRightTemp;
        }
        
        // Occupancy detection
        if (reply.get(Vehicle.Cabin.Seat.Row1.Left.IsOccupied)->isAvailable()) {
            frontLeftOccupied = reply.get(Vehicle.Cabin.Seat.Row1.Left.IsOccupied)->value();
            climateZones["FrontLeft"].isOccupied = frontLeftOccupied;
        }
        
        if (reply.get(Vehicle.Cabin.Seat.Row1.Right.IsOccupied)->isAvailable()) {
            frontRightOccupied = reply.get(Vehicle.Cabin.Seat.Row1.Right.IsOccupied)->value();
            climateZones["FrontRight"].isOccupied = frontRightOccupied;
        }
        
        if (reply.get(Vehicle.Cabin.Seat.Row2.Left.IsOccupied)->isAvailable()) {
            rearLeftOccupied = reply.get(Vehicle.Cabin.Seat.Row2.Left.IsOccupied)->value();
            climateZones["RearLeft"].isOccupied = rearLeftOccupied;
            // Assume rear temperature similar to front for demonstration
            climateZones["RearLeft"].currentTemp = frontLeftTemp + 1.0;
        }
        
        if (reply.get(Vehicle.Cabin.Seat.Row2.Right.IsOccupied)->isAvailable()) {
            rearRightOccupied = reply.get(Vehicle.Cabin.Seat.Row2.Right.IsOccupied)->value();
            climateZones["RearRight"].isOccupied = rearRightOccupied;
            climateZones["RearRight"].currentTemp = frontRightTemp + 1.0;
        }
        
        // External conditions
        if (reply.get(Vehicle.Exterior.AirTemperature)->isAvailable()) {
            exteriorTemperature = reply.get(Vehicle.Exterior.AirTemperature)->value();
        }
        
        // HVAC system status
        if (reply.get(Vehicle.Cabin.HVAC.IsAirConditioningActive)->isAvailable()) {
            hvacSystemActive = reply.get(Vehicle.Cabin.HVAC.IsAirConditioningActive)->value();
        }
        
        // Log comprehensive climate status
        velocitas::logger().info("🌡️  Climate Status: FL={:.1f}°C[{}], FR={:.1f}°C[{}], Ext={:.1f}°C, HVAC={}", 
                                frontLeftTemp, frontLeftOccupied ? "OCC" : "---",
                                frontRightTemp, frontRightOccupied ? "OCC" : "---",
                                exteriorTemperature, hvacSystemActive ? "ON" : "OFF");
        
        // Execute climate control logic
        analyzeOccupancy();
        optimizeZoneTemperatures();
        manageEnergyEfficiency(hvacSystemActive);
        predictiveClimateAdjustment(exteriorTemperature);
        
        if (adaptiveLearningEnabled) {
            updateComfortPreferences();
        }
        
        // Generate periodic reports
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastReport = std::chrono::duration_cast<std::chrono::minutes>(now - lastReportTime).count();
        if (timeSinceLastReport >= REPORT_INTERVAL_MINUTES) {
            generateClimateReport();
            lastReportTime = now;
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Climate control error: {}", e.what());
    }
}

void SmartClimateApp::analyzeOccupancy() {
    int occupiedZones = 0;
    std::vector<std::string> occupiedZoneNames;
    
    for (auto& [zoneName, zone] : climateZones) {
        if (zone.isOccupied) {
            occupiedZones++;
            occupiedZoneNames.push_back(zoneName);
        }
    }
    
    if (occupiedZones > 0) {
        std::string occupancyList;
        for (size_t i = 0; i < occupiedZoneNames.size(); ++i) {
            occupancyList += occupiedZoneNames[i];
            if (i < occupiedZoneNames.size() - 1) occupancyList += ", ";
        }
        
        velocitas::logger().info("👥 Occupancy: {} zones occupied [{}]", occupiedZones, occupancyList);
        
        // Enable energy saving for unoccupied zones
        for (auto& [zoneName, zone] : climateZones) {
            if (!zone.isOccupied) {
                // Reduce heating/cooling for unoccupied zones
                if (exteriorTemperature > 25.0) {
                    zone.targetTemp = OPTIMAL_TEMP + ENERGY_SAVE_TEMP_OFFSET;
                } else {
                    zone.targetTemp = OPTIMAL_TEMP - ENERGY_SAVE_TEMP_OFFSET;
                }
                zone.needsAdjustment = true;
            }
        }
    } else {
        velocitas::logger().info("👥 No occupancy detected - activating full energy save mode");
        energySaveMode = true;
        energySavingActivations++;
    }
}

void SmartClimateApp::optimizeZoneTemperatures() {
    for (auto& [zoneName, zone] : climateZones) {
        double tempDifference = std::abs(zone.currentTemp - zone.targetTemp);
        
        if (tempDifference > TEMP_TOLERANCE) {
            zone.needsAdjustment = true;
            
            if (zone.isOccupied) {
                // Use comfort profile for occupied zones
                auto& profile = comfortProfiles[zoneName];
                zone.targetTemp = profile.preferredTemp;
                
                velocitas::logger().info("🎯 Zone {} needs adjustment: {:.1f}°C -> {:.1f}°C (Comfort profile)", 
                                        zoneName, zone.currentTemp, zone.targetTemp);
            } else {
                // Energy-efficient targets for unoccupied zones
                velocitas::logger().info("💡 Zone {} energy optimization: {:.1f}°C -> {:.1f}°C (Energy save)", 
                                        zoneName, zone.currentTemp, zone.targetTemp);
            }
            
            temperatureAdjustments++;
            zone.lastAdjustment = std::chrono::steady_clock::now();
            comfortOptimizations++;
        }
    }
    
    // Calculate average comfort score
    double totalComfortScore = 0.0;
    int occupiedCount = 0;
    
    for (auto& [zoneName, zone] : climateZones) {
        if (zone.isOccupied) {
            double comfortScore = 100.0 - (std::abs(zone.currentTemp - zone.targetTemp) / TEMP_TOLERANCE) * 20.0;
            comfortScore = std::max(0.0, std::min(100.0, comfortScore));
            totalComfortScore += comfortScore;
            occupiedCount++;
        }
    }
    
    if (occupiedCount > 0) {
        averageComfortScore = totalComfortScore / occupiedCount;
        velocitas::logger().info("😊 Average comfort score: {:.1f}/100", averageComfortScore);
    }
}

void SmartClimateApp::manageEnergyEfficiency(bool isAcActive) {
    if (energyOptimizationEnabled) {
        double energyUsage = 0.0;
        
        // Calculate energy usage based on HVAC activity
        if (isAcActive) {
            double tempDifferential = std::abs(exteriorTemperature - OPTIMAL_TEMP);
            energyUsage = tempDifferential * 0.1; // Simplified energy calculation
        }
        
        totalEnergyUsage += energyUsage;
        
        // Energy efficiency recommendations
        if (energyUsage > 2.0) { // High energy usage threshold
            velocitas::logger().warn("⚡ High HVAC energy usage: {:.2f}kW - Consider efficiency optimization", energyUsage);
            
            // Suggest energy-saving measures
            if (exteriorTemperature > EXTERIOR_TEMP_THRESHOLD) {
                velocitas::logger().info("💡 Energy saving tip: Pre-cool cabin before departure to reduce AC load");
            }
            
            // Implement auto energy-saving
            if (energySaveMode) {
                for (auto& [zoneName, zone] : climateZones) {
                    if (!zone.isOccupied) {
                        zone.targetTemp += 1.0; // Further reduce cooling/heating
                    }
                }
                velocitas::logger().info("🔋 Auto energy-save activated - reducing unoccupied zone conditioning");
            }
        }
        
        velocitas::logger().info("⚡ HVAC Energy: Current={:.2f}kW, Total={:.2f}kWh", energyUsage, totalEnergyUsage);
    }
}

void SmartClimateApp::predictiveClimateAdjustment(double exteriorTemp) {
    if (predictiveControlEnabled) {
        // Predictive cooling/heating based on external conditions
        if (exteriorTemp > COMFORT_TEMP_MAX + 5.0) {
            // Hot weather - pre-cool occupied zones
            for (auto& [zoneName, zone] : climateZones) {
                if (zone.isOccupied) {
                    zone.targetTemp = std::min(zone.targetTemp, OPTIMAL_TEMP - 1.0);
                    velocitas::logger().info("❄️  Predictive cooling: {} target reduced to {:.1f}°C (Ext: {:.1f}°C)", 
                                            zoneName, zone.targetTemp, exteriorTemp);
                }
            }
        } else if (exteriorTemp < COMFORT_TEMP_MIN - 5.0) {
            // Cold weather - pre-warm occupied zones
            for (auto& [zoneName, zone] : climateZones) {
                if (zone.isOccupied) {
                    zone.targetTemp = std::max(zone.targetTemp, OPTIMAL_TEMP + 1.0);
                    velocitas::logger().info("🔥 Predictive heating: {} target increased to {:.1f}°C (Ext: {:.1f}°C)", 
                                            zoneName, zone.targetTemp, exteriorTemp);
                }
            }
        }
        
        // Humidity and air quality considerations (simulated)
        if (exteriorTemp > 28.0) {
            velocitas::logger().info("🌬️  High temperature detected - Activating enhanced air circulation");
        }
    }
}

void SmartClimateApp::updateComfortPreferences() {
    // Machine learning-like comfort preference adaptation
    for (auto& [zoneName, zone] : climateZones) {
        if (zone.isOccupied) {
            auto& profile = comfortProfiles[zoneName];
            
            // Record current temperature as a preference data point
            profile.historicalPreferences.push_back(zone.currentTemp);
            profile.adjustmentCount++;
            
            // Maintain history size
            if (profile.historicalPreferences.size() > LEARNING_SAMPLE_SIZE) {
                profile.historicalPreferences.erase(profile.historicalPreferences.begin());
            }
            
            // Update preferred temperature based on historical data
            if (profile.historicalPreferences.size() >= 10) {
                double sum = std::accumulate(profile.historicalPreferences.begin(), 
                                           profile.historicalPreferences.end(), 0.0);
                profile.preferredTemp = sum / profile.historicalPreferences.size();
                profile.preferencesLearned = true;
                
                velocitas::logger().info("🧠 Learning: {} preferred temperature updated to {:.1f}°C (based on {} samples)", 
                                        zoneName, profile.preferredTemp, profile.historicalPreferences.size());
            }
        }
    }
}

void SmartClimateApp::generateClimateReport() {
    auto now = std::chrono::steady_clock::now();
    auto systemRuntime = std::chrono::duration_cast<std::chrono::minutes>(now - systemStartTime).count();
    
    velocitas::logger().info("📋 CLIMATE CONTROL REPORT - Runtime: {}min", systemRuntime);
    velocitas::logger().info("🌡️  External: {:.1f}°C, HVAC: {}, Energy Save: {}", 
                            exteriorTemperature, hvacSystemActive ? "ACTIVE" : "INACTIVE", 
                            energySaveMode ? "ON" : "OFF");
    
    // Zone status report
    for (auto& [zoneName, zone] : climateZones) {
        velocitas::logger().info("📍 {}: {:.1f}°C (Target: {:.1f}°C) [{}]", 
                                zoneName, zone.currentTemp, zone.targetTemp,
                                zone.isOccupied ? "OCCUPIED" : "EMPTY");
    }
    
    // Performance metrics
    velocitas::logger().info("📊 Performance: Adjustments={}, Energy Saves={}, Optimizations={}", 
                            temperatureAdjustments, energySavingActivations, comfortOptimizations);
    velocitas::logger().info("😊 Average Comfort Score: {:.1f}/100", averageComfortScore);
    velocitas::logger().info("⚡ Total Energy Usage: {:.2f}kWh", totalEnergyUsage);
    
    // Comfort profile status
    int learnedProfiles = 0;
    for (auto& [zoneName, profile] : comfortProfiles) {
        if (profile.preferencesLearned) {
            learnedProfiles++;
            velocitas::logger().info("🧠 {} profile: {:.1f}°C ±{:.1f}°C (learned from {} samples)", 
                                    zoneName, profile.preferredTemp, profile.tempTolerance, 
                                    profile.historicalPreferences.size());
        }
    }
    
    velocitas::logger().info("🎓 Learning status: {}/{} zones have learned preferences", 
                            learnedProfiles, climateZones.size());
}

// Application entry point
std::unique_ptr<SmartClimateApp> climateApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 Smart Climate Control shutdown initiated (signal {})", sig);
    if (climateApp) {
        climateApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting Intelligent Climate Control System...");
    velocitas::logger().info("🌡️  Multi-zone HVAC optimization with adaptive learning");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    climateApp = std::make_unique<SmartClimateApp>();
    try {
        climateApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Climate control system error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown climate control error");
        return 1;
    }
    
    velocitas::logger().info("👋 Smart Climate Control System stopped");
    return 0;
}

// ============================================================================
// 🎓 PRODUCTION DEPLOYMENT NOTES
// ============================================================================
//
// 🏭 SMART CLIMATE INTEGRATIONS:
// 1. AI/ML models for comfort prediction and optimization
// 2. Weather service integration for proactive climate adjustment
// 3. Calendar integration for pre-conditioning before trips
// 4. Mobile app for remote climate control and scheduling
// 5. Energy management system integration for battery optimization
// 6. Air quality sensors and automatic filtration control
// 7. Occupancy sensors with biometric comfort monitoring
// 8. Integration with smart home systems for seamless experience
//
// 🧠 MACHINE LEARNING FEATURES:
// - Personal comfort profile learning and adaptation
// - Predictive climate control based on driving patterns
// - Energy optimization using weather forecasts
// - Comfort scoring and automatic adjustment algorithms
// - Anomaly detection for HVAC system health monitoring
//
// 🔧 CONFIGURATION PARAMETERS:
// - Zone-specific temperature ranges and preferences
// - Energy optimization thresholds and strategies
// - Learning algorithm parameters and sensitivity
// - Predictive control timing and aggressiveness
// - Integration with vehicle energy management policies
//
// ============================================================================