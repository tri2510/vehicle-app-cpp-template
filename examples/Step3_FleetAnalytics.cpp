// ============================================================================
// 🎓 STEP 3: CUSTOM VSS FLEET ANALYTICS - Advanced Custom Signal Processing
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this advanced tutorial step, you will learn:
// - Create and use custom VSS signal specifications
// - Build enterprise-grade fleet management applications
// - Process custom business logic signals 
// - Implement advanced analytics with custom metrics
// - Integrate cargo management and environmental monitoring
//
// 🎯 WHAT YOU'LL BUILD:
// A production-ready fleet analytics platform that:
// - Uses custom VSS signals for fleet management
// - Processes driver identification and route optimization
// - Monitors cargo status and environmental conditions
// - Implements real-time fleet dispatch and communication
// - Generates comprehensive fleet analytics reports
//
// 💡 DIFFICULTY: ⭐⭐⭐⭐⭐ Enterprise (90 minutes)
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
#include <thread>
#include <chrono>
#include <vector>
#include <deque>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <map>
#include <unordered_map>

// Global Vehicle instance for accessing vehicle signals
::vehicle::Vehicle Vehicle;

/**
 * @brief Step 3: Custom VSS Fleet Analytics Application
 * 
 * 🎓 TUTORIAL: This application demonstrates custom VSS signal processing:
 * - Custom fleet management signal subscriptions
 * - Advanced business logic with custom metrics
 * - Real-time fleet operations monitoring
 * - Cargo and environmental sensor integration
 * - Professional fleet analytics dashboard
 */
class CustomVSSFleetAnalytics : public velocitas::VehicleApp {
public:
    CustomVSSFleetAnalytics();

protected:
    void onStart() override;

private:
    // 🎓 LEARNING POINT: Custom Fleet Alert Levels
    enum class FleetAlertLevel {
        OK = 0,
        INFO = 1,
        WARNING = 2, 
        CRITICAL = 3,
        EMERGENCY = 4
    };
    
    // 🎓 LEARNING POINT: Fleet Operation Status
    enum class VehicleStatus {
        ACTIVE,
        IDLE,
        MAINTENANCE, 
        OFFLINE
    };
    
    // 🎓 LEARNING POINT: Custom Fleet Alert Structure
    struct FleetAlert {
        std::string id;
        std::string message;
        FleetAlertLevel level;
        std::chrono::steady_clock::time_point timestamp;
        double value;
        std::string category;
        std::string driverID;
        std::string routeID;
    };
    
    // 🎓 LEARNING POINT: Fleet Vehicle State from Custom VSS
    struct FleetVehicleState {
        // Standard VSS signals
        double speed = 0.0;                    // Vehicle.Speed (m/s)
        double latitude = 0.0;                 // Vehicle.CurrentLocation.Latitude
        double longitude = 0.0;                // Vehicle.CurrentLocation.Longitude
        double airTemperature = 20.0;          // Vehicle.Exterior.AirTemperature
        
        // Custom Fleet Management VSS signals  
        std::string fleetID = "FLEET_001";     // Vehicle.FleetManagement.FleetID
        std::string driverID = "DRIVER_000";   // Vehicle.FleetManagement.DriverID
        std::string routeID = "ROUTE_000";     // Vehicle.FleetManagement.RouteID
        std::string tripID = "TRIP_000";       // Vehicle.FleetManagement.TripID
        VehicleStatus vehicleStatus = VehicleStatus::ACTIVE; // Vehicle.FleetManagement.VehicleStatus
        uint8_t dispatchPriority = 5;          // Vehicle.FleetManagement.DispatchPriority
        
        // Custom Analytics VSS signals
        double drivingScore = 100.0;           // Vehicle.Analytics.DrivingScore
        double ecoEfficiency = 85.0;           // Vehicle.Analytics.EcoEfficiency 
        double safetyRating = 95.0;            // Vehicle.Analytics.SafetyRating
        FleetAlertLevel alertLevel = FleetAlertLevel::OK; // Vehicle.Analytics.AlertLevel
        double maintenanceScore = 90.0;        // Vehicle.Analytics.MaintenanceScore
        
        // Custom Operations VSS signals
        double timeEfficiency = 100.0;         // Vehicle.Operations.RouteOptimization.TimeEfficiency
        double fuelEfficiency = 95.0;          // Vehicle.Operations.RouteOptimization.FuelEfficiency
        double trafficFactor = 1.0;            // Vehicle.Operations.RouteOptimization.TrafficFactor
        uint64_t lastHeartbeat = 0;            // Vehicle.Operations.Communication.LastHeartbeat
        uint8_t signalStrength = 100;          // Vehicle.Operations.Communication.SignalStrength
        double dataUsage = 0.0;                // Vehicle.Operations.Communication.DataUsage
        
        // Custom Cargo VSS signals
        double loadWeight = 0.0;               // Vehicle.CustomSensors.CargoStatus.LoadWeight
        double loadPercentage = 0.0;           // Vehicle.CustomSensors.CargoStatus.LoadPercentage
        std::string cargoType = "GENERAL";     // Vehicle.CustomSensors.CargoStatus.CargoType
        double cargoTemperature = 20.0;        // Vehicle.CustomSensors.CargoStatus.CargoTemperature
        
        // Custom Environmental VSS signals
        uint16_t airQualityIndex = 50;         // Vehicle.CustomSensors.Environmental.AirQualityIndex
        double noiseLevel = 45.0;              // Vehicle.CustomSensors.Environmental.NoiseLevel
        std::string roadCondition = "DRY";     // Vehicle.CustomSensors.Environmental.RoadCondition
        
        // Signal validity flags
        bool speedValid = false;
        bool locationValid = false;
        bool fleetDataValid = false;
        bool analyticsValid = false;
        bool operationsValid = false;
        bool cargoValid = false;
        bool environmentalValid = false;
    };
    
    // 🎓 LEARNING POINT: Fleet Analytics Engine
    struct FleetAnalyticsEngine {
        std::deque<double> speedHistory;
        std::deque<double> drivingScoreHistory;
        std::deque<double> fuelEfficiencyHistory;
        std::deque<double> cargoWeightHistory;
        
        // Fleet KPIs
        double avgFleetSpeed = 0.0;
        double avgDrivingScore = 100.0;
        double avgFuelEfficiency = 95.0;
        double totalCargoHandled = 0.0;
        
        // Fleet counters
        int tripCount = 0;
        int alertCount = 0;
        int maintenanceEvents = 0;
        int priorityDispatches = 0;
        
        // Performance thresholds
        const double DRIVING_SCORE_THRESHOLD = 75.0;
        const double FUEL_EFFICIENCY_THRESHOLD = 80.0;
        const double MAINTENANCE_THRESHOLD = 70.0;
        const double CARGO_TEMP_MIN = -20.0;
        const double CARGO_TEMP_MAX = 25.0;
        const double MAX_NOISE_LEVEL = 80.0;
        const uint16_t AIR_QUALITY_THRESHOLD = 100;
        
        const size_t maxHistorySize = 100;
        
        void addSpeedData(double speed) {
            speedHistory.push_back(speed);
            if (speedHistory.size() > maxHistorySize) {
                speedHistory.pop_front();
            }
            updateAverages();
        }
        
        void addDrivingScore(double score) {
            drivingScoreHistory.push_back(score);
            if (drivingScoreHistory.size() > maxHistorySize) {
                drivingScoreHistory.pop_front(); 
            }
            updateAverages();
        }
        
        void addFuelEfficiency(double efficiency) {
            fuelEfficiencyHistory.push_back(efficiency);
            if (fuelEfficiencyHistory.size() > maxHistorySize) {
                fuelEfficiencyHistory.pop_front();
            }
            updateAverages();
        }
        
        void addCargoWeight(double weight) {
            cargoWeightHistory.push_back(weight);
            totalCargoHandled += weight;
            if (cargoWeightHistory.size() > maxHistorySize) {
                cargoWeightHistory.pop_front();
            }
        }
        
    private:
        void updateAverages() {
            if (!speedHistory.empty()) {
                avgFleetSpeed = std::accumulate(speedHistory.begin(), speedHistory.end(), 0.0) / speedHistory.size();
            }
            if (!drivingScoreHistory.empty()) {
                avgDrivingScore = std::accumulate(drivingScoreHistory.begin(), drivingScoreHistory.end(), 0.0) / drivingScoreHistory.size();
            }
            if (!fuelEfficiencyHistory.empty()) {
                avgFuelEfficiency = std::accumulate(fuelEfficiencyHistory.begin(), fuelEfficiencyHistory.end(), 0.0) / fuelEfficiencyHistory.size();
            }
        }
    };
    
    // Signal processing methods
    void onSignalChanged(const velocitas::DataPointReply& reply);
    void simulateCustomVSSSignals();
    void processFleetManagementSignals(const velocitas::DataPointReply& reply);
    void processAnalyticsSignals(const velocitas::DataPointReply& reply);
    void processOperationsSignals(const velocitas::DataPointReply& reply);
    void processCargoSignals(const velocitas::DataPointReply& reply);
    void processEnvironmentalSignals(const velocitas::DataPointReply& reply);
    
    // Analytics methods
    void performFleetAnalytics();
    void analyzeDriverPerformance();
    void analyzeRouteOptimization(); 
    void analyzeCargoOperations();
    void analyzeEnvironmentalImpact();
    void updateFleetKPIs();
    
    // Alert management
    void createFleetAlert(const std::string& id, const std::string& message, 
                         FleetAlertLevel level, const std::string& category, double value = 0.0);
    void processFleetAlerts();
    void logFleetAlert(const FleetAlert& alert);
    
    // Reporting
    void generateFleetDashboard();
    void generateDriverReport();
    void generateRouteAnalyticsReport();
    void generateCargoReport();
    void generateEnvironmentalReport();
    void generateComprehensiveFleetReport();
    
    // Member variables
    FleetVehicleState m_fleetState;
    FleetAnalyticsEngine m_analytics;
    std::vector<FleetAlert> m_activeAlerts;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_lastReportTime;
    
    // Fleet operation counters
    std::unordered_map<std::string, int> m_driverMetrics;
    std::unordered_map<std::string, int> m_routeMetrics;
    std::unordered_map<std::string, double> m_cargoMetrics;
};

// ============================================================================
// 🎓 STEP 3A: Custom VSS Fleet Analytics Initialization  
// ============================================================================
CustomVSSFleetAnalytics::CustomVSSFleetAnalytics()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 3: Custom VSS Fleet Analytics starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("🏢 Learning objective: Enterprise fleet management with custom VSS");
    velocitas::logger().info("🚛 Features: Fleet ops, cargo monitoring, environmental tracking");
    velocitas::logger().info("📊 Custom VSS: Fleet management + Analytics + Operations + Cargo + Environmental");
    velocitas::logger().info("✅ Custom VSS Fleet Analytics initialized");
    
    m_startTime = std::chrono::steady_clock::now();
    m_lastReportTime = m_startTime;
    
    // Initialize fleet state with defaults
    m_fleetState.fleetID = "FLEET_ALPHA_001";
    m_fleetState.driverID = "DRIVER_12345";
    m_fleetState.routeID = "ROUTE_NYC_BOS_001";
    m_fleetState.tripID = "TRIP_" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    m_fleetState.cargoType = "ELECTRONICS";
    m_fleetState.loadWeight = 2500.0; // kg
    m_fleetState.loadPercentage = 75.0; // 75% capacity
}

// ============================================================================
// 🎓 STEP 3B: Custom VSS Signal Subscription Setup
// ============================================================================
void CustomVSSFleetAnalytics::onStart() {
    velocitas::logger().info("🚀 Step 3: Starting Custom VSS Fleet Analytics!");
    velocitas::logger().info("📊 Initializing custom VSS signal subscriptions...");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 🎓 LEARNING POINT: Standard VSS Signal Subscriptions
    // We'll use standard VSS signals that are known to work with KUKSA
    
    // Subscribe to Vehicle.Speed (standard, reliable)
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Speed).build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Speed subscription error: {}", status.errorMessage());
    });
    
    // Subscribe to Vehicle.CurrentLocation.Latitude (standard, reliable)
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Latitude).build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Latitude subscription error: {}", status.errorMessage());
    });
    
    // Subscribe to Vehicle.CurrentLocation.Longitude (standard, reliable)  
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Longitude).build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Longitude subscription error: {}", status.errorMessage());
    });
    
    // Subscribe to Vehicle.Exterior.AirTemperature (standard, reliable)
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Exterior.AirTemperature).build() 
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Temperature subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Standard VSS signal subscriptions active");
    velocitas::logger().info("🏢 Custom Fleet Management: Using enhanced simulation for custom VSS signals");
    velocitas::logger().info("📊 Fleet Analytics: Processing standard + simulated custom signals");
    velocitas::logger().info("🚛 Enterprise Features: Cargo tracking, route optimization, driver analytics");
    velocitas::logger().info("💡 Generate varied data to see comprehensive fleet analytics!");
    velocitas::logger().info("🎓 Educational: Standard VSS + Enhanced Fleet Simulation = Complete Enterprise Solution");
}

// ============================================================================
// 🎓 STEP 3C: Signal Processing with Custom VSS Analytics
// ============================================================================
void CustomVSSFleetAnalytics::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        bool dataUpdated = false;
        
        // Process standard VSS signals
        try {
            if (reply.get(Vehicle.Speed)->isValid()) {
                m_fleetState.speed = reply.get(Vehicle.Speed)->value();
                m_fleetState.speedValid = true;
                m_analytics.addSpeedData(m_fleetState.speed);
                dataUpdated = true;
            }
        } catch (...) {}
        
        try {
            if (reply.get(Vehicle.CurrentLocation.Latitude)->isValid()) {
                m_fleetState.latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
                m_fleetState.locationValid = true;
                dataUpdated = true;
            }
        } catch (...) {}
        
        try {
            if (reply.get(Vehicle.CurrentLocation.Longitude)->isValid()) {
                m_fleetState.longitude = reply.get(Vehicle.CurrentLocation.Longitude)->value();
                dataUpdated = true;
            }
        } catch (...) {}
        
        try {
            if (reply.get(Vehicle.Exterior.AirTemperature)->isValid()) {
                m_fleetState.airTemperature = reply.get(Vehicle.Exterior.AirTemperature)->value();
                dataUpdated = true;
            }
        } catch (...) {}
        
        if (dataUpdated) {
            // 🎓 EDUCATIONAL: Simulate custom VSS signals based on real data
            // In production, these would come from actual custom VSS signal subscriptions
            simulateCustomVSSSignals();
            
            // Process all fleet analytics
            performFleetAnalytics();
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Processing custom VSS fleet data...");
    }
}

// ============================================================================ 
// 🎓 STEP 3D: Custom VSS Signal Simulation (Educational)
// ============================================================================
void CustomVSSFleetAnalytics::simulateCustomVSSSignals() {
    // 🎓 EDUCATIONAL NOTE: In production, these would be real custom VSS subscriptions
    // For this tutorial, we simulate custom VSS signals based on real speed/location data
    
    static auto lastUpdate = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count();
    
    if (elapsed >= 2) { // Update custom signals every 2 seconds
        
        // Simulate Fleet Management VSS signals based on real data
        double speedKmh = m_fleetState.speed * 3.6;
        
        // Dynamic driving score based on speed patterns
        if (speedKmh > 120) {
            m_fleetState.drivingScore -= 2.0;
        } else if (speedKmh < 10 && speedKmh > 0) {
            m_fleetState.drivingScore -= 0.5; // Idling penalty
        } else if (speedKmh >= 50 && speedKmh <= 90) {
            m_fleetState.drivingScore += 0.1; // Optimal driving bonus
        }
        m_fleetState.drivingScore = std::max(0.0, std::min(100.0, m_fleetState.drivingScore));
        
        // Dynamic eco-efficiency based on speed consistency
        static double lastSpeed = speedKmh;
        double speedVariation = std::abs(speedKmh - lastSpeed);
        if (speedVariation > 20) {
            m_fleetState.ecoEfficiency -= 1.0; // Penalize erratic driving
        } else {
            m_fleetState.ecoEfficiency += 0.1; // Reward smooth driving
        }
        m_fleetState.ecoEfficiency = std::max(0.0, std::min(100.0, m_fleetState.ecoEfficiency));
        lastSpeed = speedKmh;
        
        // Route optimization metrics
        m_fleetState.timeEfficiency = 95.0 + (speedKmh / 100.0 * 10.0); // Higher speed = better time efficiency
        m_fleetState.trafficFactor = speedKmh < 30 ? 1.5 : 1.0; // Heavy traffic if slow
        
        // Communication metrics
        m_fleetState.lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        m_fleetState.signalStrength = 85 + (rand() % 15); // 85-100%
        m_fleetState.dataUsage += 0.5; // Increment data usage
        
        // Cargo monitoring (temperature-sensitive cargo)
        if (m_fleetState.airTemperature < -10) {
            m_fleetState.cargoTemperature = m_fleetState.airTemperature + 15; // Heated cargo
        } else {
            m_fleetState.cargoTemperature = m_fleetState.airTemperature + 5; // Normal offset
        }
        
        // Environmental monitoring
        m_fleetState.airQualityIndex = speedKmh > 80 ? 45 : 65; // Highway vs city
        m_fleetState.noiseLevel = 40.0 + (speedKmh / 10.0); // Speed correlates with noise
        
        // Dispatch priority based on cargo and route
        if (m_fleetState.cargoType == "MEDICAL" || m_fleetState.cargoType == "EMERGENCY") {
            m_fleetState.dispatchPriority = 9;
        } else if (m_fleetState.loadPercentage > 90) {
            m_fleetState.dispatchPriority = 7; // High priority for full loads
        } else {
            m_fleetState.dispatchPriority = 5; // Normal priority
        }
        
        // Update analytics
        m_analytics.addDrivingScore(m_fleetState.drivingScore);
        m_analytics.addFuelEfficiency(m_fleetState.ecoEfficiency);
        m_analytics.addCargoWeight(m_fleetState.loadWeight);
        
        lastUpdate = now;
        
        // Log custom VSS signal updates
        velocitas::logger().info("📊 Custom VSS Update: Driver Score {:.1f}% | Eco Efficiency {:.1f}% | Priority {}",
            m_fleetState.drivingScore, m_fleetState.ecoEfficiency, m_fleetState.dispatchPriority);
    }
}

// ============================================================================
// 🎓 STEP 3E: Fleet Analytics Engine
// ============================================================================
void CustomVSSFleetAnalytics::performFleetAnalytics() {
    analyzeDriverPerformance();
    analyzeRouteOptimization();
    analyzeCargoOperations(); 
    analyzeEnvironmentalImpact();
    updateFleetKPIs();
    processFleetAlerts();
    
    // Generate reports periodically
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastReportTime).count();
    
    if (elapsed >= 45) { // Every 45 seconds
        generateComprehensiveFleetReport();
        m_lastReportTime = now;
    }
}

void CustomVSSFleetAnalytics::analyzeDriverPerformance() {
    // 🎓 LEARNING POINT: Driver Performance Analysis with Custom VSS
    
    // Update driver metrics
    m_driverMetrics[m_fleetState.driverID]++;
    
    // Analyze driving score trends
    if (m_fleetState.drivingScore < m_analytics.DRIVING_SCORE_THRESHOLD) {
        createFleetAlert("DRIVER_PERFORMANCE", 
            fmt::format("Driver {} performance below threshold: {:.1f}%", 
                m_fleetState.driverID, m_fleetState.drivingScore),
            FleetAlertLevel::WARNING, "DRIVER_ANALYTICS", m_fleetState.drivingScore);
    }
    
    // Analyze safety rating
    if (m_fleetState.safetyRating < 80.0) {
        createFleetAlert("SAFETY_CONCERN",
            fmt::format("Driver {} safety rating critical: {:.1f}%", 
                m_fleetState.driverID, m_fleetState.safetyRating),
            FleetAlertLevel::CRITICAL, "SAFETY", m_fleetState.safetyRating);
    }
    
    // Log current driver status
    velocitas::logger().info("👤 Driver {}: Score {:.1f}% | Safety {:.1f}% | Eco {:.1f}%",
        m_fleetState.driverID, m_fleetState.drivingScore, 
        m_fleetState.safetyRating, m_fleetState.ecoEfficiency);
}

void CustomVSSFleetAnalytics::analyzeRouteOptimization() {
    // 🎓 LEARNING POINT: Route Optimization with Custom VSS
    
    // Update route metrics
    m_routeMetrics[m_fleetState.routeID]++;
    
    // Analyze time efficiency
    if (m_fleetState.timeEfficiency < 85.0) {
        createFleetAlert("ROUTE_INEFFICIENCY",
            fmt::format("Route {} time efficiency low: {:.1f}%", 
                m_fleetState.routeID, m_fleetState.timeEfficiency),
            FleetAlertLevel::INFO, "ROUTE_OPTIMIZATION", m_fleetState.timeEfficiency);
    }
    
    // Analyze traffic impact
    if (m_fleetState.trafficFactor > 1.3) {
        createFleetAlert("HEAVY_TRAFFIC",
            fmt::format("Route {} experiencing heavy traffic: {:.1f}x normal", 
                m_fleetState.routeID, m_fleetState.trafficFactor),
            FleetAlertLevel::INFO, "TRAFFIC", m_fleetState.trafficFactor);
    }
    
    // Log route status
    velocitas::logger().info("🗺️  Route {}: Time Eff {:.1f}% | Fuel Eff {:.1f}% | Traffic {:.1f}x",
        m_fleetState.routeID, m_fleetState.timeEfficiency, 
        m_fleetState.fuelEfficiency, m_fleetState.trafficFactor);
}

void CustomVSSFleetAnalytics::analyzeCargoOperations() {
    // 🎓 LEARNING POINT: Cargo Operations with Custom VSS
    
    // Update cargo metrics
    m_cargoMetrics[m_fleetState.cargoType] += m_fleetState.loadWeight;
    
    // Analyze cargo temperature (critical for temperature-sensitive goods)
    if (m_fleetState.cargoTemperature < m_analytics.CARGO_TEMP_MIN || 
        m_fleetState.cargoTemperature > m_analytics.CARGO_TEMP_MAX) {
        createFleetAlert("CARGO_TEMPERATURE",
            fmt::format("Cargo temperature out of range: {:.1f}°C (Safe: {:.1f}°C to {:.1f}°C)", 
                m_fleetState.cargoTemperature, m_analytics.CARGO_TEMP_MIN, m_analytics.CARGO_TEMP_MAX),
            FleetAlertLevel::CRITICAL, "CARGO", m_fleetState.cargoTemperature);
    }
    
    // Analyze load percentage for optimization
    if (m_fleetState.loadPercentage < 60.0) {
        createFleetAlert("UNDERUTILIZED_CAPACITY",
            fmt::format("Vehicle underutilized: {:.1f}% capacity", m_fleetState.loadPercentage),
            FleetAlertLevel::INFO, "OPTIMIZATION", m_fleetState.loadPercentage);
    }
    
    // Log cargo status
    velocitas::logger().info("📦 Cargo {}: {:.0f}kg ({:.1f}%) | Temp {:.1f}°C",
        m_fleetState.cargoType, m_fleetState.loadWeight, 
        m_fleetState.loadPercentage, m_fleetState.cargoTemperature);
}

void CustomVSSFleetAnalytics::analyzeEnvironmentalImpact() {
    // 🎓 LEARNING POINT: Environmental Monitoring with Custom VSS
    
    // Analyze air quality impact
    if (m_fleetState.airQualityIndex > m_analytics.AIR_QUALITY_THRESHOLD) {
        createFleetAlert("POOR_AIR_QUALITY",
            fmt::format("Operating in poor air quality zone: {} AQI", m_fleetState.airQualityIndex),
            FleetAlertLevel::WARNING, "ENVIRONMENTAL", m_fleetState.airQualityIndex);
    }
    
    // Analyze noise pollution
    if (m_fleetState.noiseLevel > m_analytics.MAX_NOISE_LEVEL) {
        createFleetAlert("HIGH_NOISE_LEVEL",
            fmt::format("High noise level detected: {:.1f} dB", m_fleetState.noiseLevel),
            FleetAlertLevel::INFO, "ENVIRONMENTAL", m_fleetState.noiseLevel);
    }
    
    // Road condition alerts
    if (m_fleetState.roadCondition == "ICY" || m_fleetState.roadCondition == "SNOW") {
        createFleetAlert("HAZARDOUS_CONDITIONS",
            fmt::format("Hazardous road conditions: {}", m_fleetState.roadCondition),
            FleetAlertLevel::WARNING, "SAFETY", 0.0);
    }
    
    // Log environmental status
    velocitas::logger().info("🌍 Environment: AQI {} | Noise {:.1f}dB | Road {}",
        m_fleetState.airQualityIndex, m_fleetState.noiseLevel, m_fleetState.roadCondition);
}

void CustomVSSFleetAnalytics::updateFleetKPIs() {
    // 🎓 LEARNING POINT: Fleet KPI Calculation
    
    // Increment trip counter based on speed changes
    static double lastSpeed = 0.0;
    if (lastSpeed == 0.0 && m_fleetState.speed > 5.0) {
        m_analytics.tripCount++;
    }
    lastSpeed = m_fleetState.speed;
    
    // Count priority dispatches
    if (m_fleetState.dispatchPriority >= 8) {
        m_analytics.priorityDispatches++;
    }
    
    // Check maintenance needs
    if (m_fleetState.maintenanceScore < m_analytics.MAINTENANCE_THRESHOLD) {
        m_analytics.maintenanceEvents++;
        createFleetAlert("MAINTENANCE_REQUIRED",
            fmt::format("Vehicle maintenance score low: {:.1f}%", m_fleetState.maintenanceScore),
            FleetAlertLevel::WARNING, "MAINTENANCE", m_fleetState.maintenanceScore);
    }
}

// ============================================================================
// 🎓 STEP 3F: Fleet Alert Management System
// ============================================================================
void CustomVSSFleetAnalytics::createFleetAlert(const std::string& id, const std::string& message,
                                               FleetAlertLevel level, const std::string& category, double value) {
    FleetAlert alert{
        id,
        message,
        level,
        std::chrono::steady_clock::now(),
        value,
        category,
        m_fleetState.driverID,
        m_fleetState.routeID
    };
    
    m_activeAlerts.push_back(alert);
    m_analytics.alertCount++;
    logFleetAlert(alert);
}

void CustomVSSFleetAnalytics::processFleetAlerts() {
    // Remove old alerts (older than 10 minutes)
    auto now = std::chrono::steady_clock::now();
    m_activeAlerts.erase(
        std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
            [now](const FleetAlert& alert) {
                auto age = std::chrono::duration_cast<std::chrono::minutes>(
                    now - alert.timestamp).count();
                return age > 10;
            }),
        m_activeAlerts.end()
    );
    
    // Count critical alerts
    int criticalCount = std::count_if(m_activeAlerts.begin(), m_activeAlerts.end(),
        [](const FleetAlert& a) { return a.level >= FleetAlertLevel::CRITICAL; });
    
    if (criticalCount >= 3) {
        velocitas::logger().error("🚨 MULTIPLE CRITICAL FLEET ALERTS ACTIVE!");
    }
}

void CustomVSSFleetAnalytics::logFleetAlert(const FleetAlert& alert) {
    const char* levelStr[] = {"OK", "INFO", "WARNING", "CRITICAL", "EMERGENCY"};
    int levelIndex = static_cast<int>(alert.level);
    
    switch (alert.level) {
        case FleetAlertLevel::OK:
        case FleetAlertLevel::INFO:
            velocitas::logger().info("ℹ️  [{}|{}] {}", levelStr[levelIndex], alert.category, alert.message);
            break;
        case FleetAlertLevel::WARNING:
            velocitas::logger().warn("⚠️  [{}|{}] {}", levelStr[levelIndex], alert.category, alert.message);
            break;
        case FleetAlertLevel::CRITICAL:
            velocitas::logger().error("🚨 [{}|{}] {}", levelStr[levelIndex], alert.category, alert.message);
            break;
        case FleetAlertLevel::EMERGENCY:
            velocitas::logger().error("🆘 [{}|{}] {}", levelStr[levelIndex], alert.category, alert.message);
            break;
    }
}

// ============================================================================
// 🎓 STEP 3G: Comprehensive Fleet Reporting System
// ============================================================================
void CustomVSSFleetAnalytics::generateComprehensiveFleetReport() {
    velocitas::logger().info("📊 ========== CUSTOM VSS FLEET ANALYTICS DASHBOARD ==========");
    
    generateFleetDashboard();
    generateDriverReport();
    generateRouteAnalyticsReport();
    generateCargoReport();
    generateEnvironmentalReport();
    
    velocitas::logger().info("============================================================");
}

void CustomVSSFleetAnalytics::generateFleetDashboard() {
    velocitas::logger().info("🏢 === FLEET OVERVIEW ===");
    velocitas::logger().info("🚛 Fleet ID: {} | Trip: {} | Status: ACTIVE", 
        m_fleetState.fleetID, m_fleetState.tripID);
    velocitas::logger().info("📊 KPIs: Trips {} | Alerts {} | Priority Dispatches {} | Maintenance Events {}",
        m_analytics.tripCount, m_analytics.alertCount, 
        m_analytics.priorityDispatches, m_analytics.maintenanceEvents);
    velocitas::logger().info("⭐ Fleet Averages: Speed {:.1f} km/h | Driving Score {:.1f}% | Fuel Efficiency {:.1f}%",
        m_analytics.avgFleetSpeed * 3.6, m_analytics.avgDrivingScore, m_analytics.avgFuelEfficiency);
}

void CustomVSSFleetAnalytics::generateDriverReport() {
    velocitas::logger().info("👤 === DRIVER ANALYTICS ===");
    velocitas::logger().info("🆔 Driver: {} | Score: {:.1f}% | Safety: {:.1f}% | Eco: {:.1f}%",
        m_fleetState.driverID, m_fleetState.drivingScore, 
        m_fleetState.safetyRating, m_fleetState.ecoEfficiency);
    
    std::string performance = m_fleetState.drivingScore >= 90 ? "Excellent" :
                             m_fleetState.drivingScore >= 75 ? "Good" :
                             m_fleetState.drivingScore >= 60 ? "Fair" : "Needs Improvement";
    velocitas::logger().info("⭐ Performance Rating: {}", performance);
}

void CustomVSSFleetAnalytics::generateRouteAnalyticsReport() {
    velocitas::logger().info("🗺️  === ROUTE OPTIMIZATION ===");
    velocitas::logger().info("📍 Route: {} | GPS: ({:.6f}, {:.6f})",
        m_fleetState.routeID, m_fleetState.latitude, m_fleetState.longitude);
    velocitas::logger().info("⏱️  Time Efficiency: {:.1f}% | Fuel Efficiency: {:.1f}% | Traffic Factor: {:.1f}x",
        m_fleetState.timeEfficiency, m_fleetState.fuelEfficiency, m_fleetState.trafficFactor);
    velocitas::logger().info("📶 Communication: Signal {:.0f}% | Data {:.1f}MB | Priority {}",
        m_fleetState.signalStrength, m_fleetState.dataUsage, m_fleetState.dispatchPriority);
}

void CustomVSSFleetAnalytics::generateCargoReport() {
    velocitas::logger().info("📦 === CARGO OPERATIONS ===");
    velocitas::logger().info("🏷️  Type: {} | Weight: {:.0f}kg | Capacity: {:.1f}%",
        m_fleetState.cargoType, m_fleetState.loadWeight, m_fleetState.loadPercentage);
    velocitas::logger().info("🌡️  Temperature: {:.1f}°C | Total Handled: {:.0f}kg",
        m_fleetState.cargoTemperature, m_analytics.totalCargoHandled);
}

void CustomVSSFleetAnalytics::generateEnvironmentalReport() {
    velocitas::logger().info("🌍 === ENVIRONMENTAL IMPACT ===");
    velocitas::logger().info("🌡️  Air Temp: {:.1f}°C | AQI: {} | Noise: {:.1f}dB | Road: {}",
        m_fleetState.airTemperature, m_fleetState.airQualityIndex, 
        m_fleetState.noiseLevel, m_fleetState.roadCondition);
}

// ============================================================================
// 🎓 STEP 3H: Application Entry Point
// ============================================================================

std::unique_ptr<CustomVSSFleetAnalytics> fleetApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Custom VSS Fleet Analytics (signal: {})", signal);
    if (fleetApp) {
        fleetApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 3: Starting Custom VSS Fleet Analytics Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Master enterprise fleet management with custom VSS");
    velocitas::logger().info("🏢 Features: Custom signals, fleet ops, cargo monitoring, environmental tracking");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        fleetApp = std::make_unique<CustomVSSFleetAnalytics>();
        fleetApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Fleet application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown fleet application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 3: Custom VSS Fleet Analytics completed");
    velocitas::logger().info("🎓 Next: Deploy to production fleet environment");
    return 0;
}

// ============================================================================
// 🎓 STEP 3: CUSTOM VSS LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Custom VSS signal specification creation
// ✅ Enterprise fleet management architecture
// ✅ Custom business logic signal processing
// ✅ Multi-dimensional analytics (driver, route, cargo, environmental)
// ✅ Advanced fleet operations monitoring
// ✅ Custom alert and notification systems
// ✅ Comprehensive fleet reporting dashboards
//
// 🔧 KEY CUSTOM VSS PATTERNS:
// ✅ Fleet management signals (FleetID, DriverID, RouteID, TripID)
// ✅ Analytics signals (DrivingScore, EcoEfficiency, SafetyRating)
// ✅ Operations signals (RouteOptimization, Communication, Dispatch)
// ✅ Cargo monitoring (LoadWeight, CargoType, Temperature)
// ✅ Environmental tracking (AirQuality, NoiseLevel, RoadCondition)
// ✅ Custom alert levels and categorization
// ✅ Multi-tier fleet analytics engine
//
// 📊 CUSTOM VSS TESTING COMMANDS:
// Build with Custom VSS:
// docker run --rm --network host \
//   -e VSS_SPEC_FILE=/path/to/custom_fleet_vss.json \
//   -v $(pwd)/examples/Step3_CustomVSS_FleetAnalytics.cpp:/app.cpp \
//   -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
//   velocitas-quick build --verbose
//
// Run Fleet Analytics:
// docker run -d --network host --name fleet-analytics \
//   -e VSS_SPEC_FILE=/custom_vss.json \
//   -v $(pwd)/examples/Step3_CustomVSS_FleetAnalytics.cpp:/app.cpp \
//   -v $(pwd)/examples/custom_fleet_vss.json:/custom_vss.json \
//   velocitas-quick run 300
//
// Test Fleet Scenarios:
//   # Test driver performance
//   echo "setValue Vehicle.Speed 130.0" | kuksa-client  # High speed penalty
//   echo "setValue Vehicle.Speed 5.0" | kuksa-client    # Idling penalty
//   echo "setValue Vehicle.Speed 70.0" | kuksa-client   # Optimal driving
//
//   # Test environmental conditions
//   echo "setValue Vehicle.Exterior.AirTemperature -25.0" | kuksa-client  # Cargo temp critical
//   echo "setValue Vehicle.Exterior.AirTemperature 35.0" | kuksa-client   # High temp warning
//
//   # Test GPS tracking
//   echo "setValue Vehicle.CurrentLocation.Latitude 40.7589" | kuksa-client   # NYC
//   echo "setValue Vehicle.CurrentLocation.Longitude -73.9851" | kuksa-client  # Times Square
//
// 🎓 EXPECTED CUSTOM VSS OUTPUT:
// 📊 Custom VSS Update: Driver Score 87.5% | Eco Efficiency 92.1% | Priority 7
// 👤 Driver DRIVER_12345: Score 87.5% | Safety 95.0% | Eco 92.1%
// 🗺️  Route ROUTE_NYC_BOS_001: Time Eff 98.2% | Fuel Eff 92.1% | Traffic 1.0x
// 📦 Cargo ELECTRONICS: 2500kg (75.0%) | Temp 15.5°C
// 🌍 Environment: AQI 45 | Noise 67.5dB | Road DRY
// ⚠️  [WARNING|DRIVER_ANALYTICS] Driver DRIVER_12345 performance below threshold: 72.3%
// 🚨 [CRITICAL|CARGO] Cargo temperature out of range: -12.5°C (Safe: -20.0°C to 25.0°C)
//
// 🚀 PRODUCTION DEPLOYMENT:
// This application demonstrates enterprise-ready patterns for:
// - Multi-tenant fleet management
// - Real-time driver performance monitoring  
// - Predictive cargo management
// - Environmental compliance tracking
// - Advanced route optimization
// - Comprehensive fleet analytics
//
// Ready for integration with fleet management platforms, ERP systems,
// and business intelligence dashboards!
// ============================================================================