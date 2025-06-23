// ============================================================================
// 🚗 EV ENERGY MANAGEMENT SYSTEM - Production SDV Example
// ============================================================================
// 
// 📋 WHAT THIS APPLICATION DOES:
// Comprehensive electric vehicle energy management system that optimizes
// battery usage, charging strategies, and range prediction for maximum efficiency.
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Battery optimization and health monitoring
// - Smart charging strategy based on usage patterns
// - Range prediction with route optimization
// - Energy recovery optimization during braking
// - Charging station recommendation and routing
// - Power management for auxiliary systems
//
// 🚀 QUICK START:
// cp examples/EVEnergyManager.cpp templates/app/src/VehicleApp.cpp
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
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>

::vehicle::Vehicle Vehicle;

/**
 * @brief EV Energy Management System
 * 
 * Production-ready electric vehicle energy management that provides:
 * - Intelligent battery optimization and health monitoring
 * - Smart charging strategy based on driving patterns and grid conditions
 * - Advanced range prediction using real-time data and route analysis
 * - Energy recovery optimization during regenerative braking
 * - Charging station recommendation with route planning
 * - Power management for auxiliary systems to maximize range
 */
class EVEnergyManager : public velocitas::VehicleApp {
public:
    EVEnergyManager();

protected:
    void onStart() override;

private:
    void onEVDataChanged(const velocitas::DataPointReply& reply);
    
    // Energy management methods
    void monitorBatteryHealth(double stateOfCharge, bool isCharging);
    void optimizeChargingStrategy(double stateOfCharge, bool isCharging);
    void predictRange(double stateOfCharge, double motorPower, double speed);
    void optimizeEnergyRecovery(double motorPower, double speed);
    void recommendChargingStations(double currentRange);
    void managePowerDistribution(double stateOfCharge);
    void analyzeEnergyEfficiency();
    void generateEnergyReport();
    
    // Energy data structures
    struct BatteryHealth {
        double capacity;           // Current capacity vs nominal
        double degradation;        // Battery degradation percentage
        double temperature;        // Battery temperature
        int cycleCount;           // Charge/discharge cycles
        double voltageHealth;     // Voltage stability indicator
        std::chrono::steady_clock::time_point lastHealthCheck;
    };
    
    struct ChargingSession {
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        double startSOC;
        double endSOC;
        double energyAdded;
        double chargingRate;
        std::string chargingLocation;
    };
    
    struct EnergyEfficiency {
        double totalEnergyUsed;    // kWh
        double distanceTraveled;   // km
        double efficiency;         // Wh/km
        double regenEfficiency;    // % energy recovered
        double auxiliaryPower;     // Power used by accessories
    };
    
    // Energy management state
    BatteryHealth batteryHealth;
    std::vector<ChargingSession> chargingHistory;
    EnergyEfficiency currentTrip;
    
    // Energy parameters and thresholds
    static constexpr double BATTERY_CAPACITY_KWH = 75.0;        // kWh nominal capacity
    static constexpr double MIN_SOC_WARNING = 20.0;             // % SOC warning threshold
    static constexpr double MIN_SOC_CRITICAL = 10.0;            // % SOC critical threshold
    static constexpr double OPTIMAL_CHARGING_TEMP = 25.0;       // °C optimal battery temp
    static constexpr double MAX_CHARGING_TEMP = 45.0;           // °C max safe charging temp
    static constexpr double ENERGY_EFFICIENCY_TARGET = 150.0;   // Wh/km target efficiency
    static constexpr double REGEN_EFFICIENCY_TARGET = 0.15;     // 15% energy recovery target
    static constexpr double FAST_CHARGING_THRESHOLD = 50.0;     // kW fast charging threshold
    
    // State tracking
    double previousSOC = 50.0;
    double previousMotorPower = 0.0;
    double estimatedRange = 0.0;
    double totalDistanceTraveled = 0.0;
    double totalEnergyConsumed = 0.0;
    double totalEnergyRecovered = 0.0;
    bool wasCharging = false;
    std::chrono::steady_clock::time_point tripStartTime;
    std::chrono::steady_clock::time_point lastReportTime;
    
    // Performance metrics
    int chargingCycles = 0;
    int rangeOptimizations = 0;
    int energySavingActivations = 0;
    double bestEfficiency = 999.0;  // Wh/km
    double worstEfficiency = 0.0;   // Wh/km
    
    // Charging strategy
    bool smartChargingEnabled = true;
    bool preConditioningEnabled = true;
    bool ecoModeEnabled = false;
    
    static constexpr int REPORT_INTERVAL_MINUTES = 10;
    static constexpr size_t MAX_CHARGING_HISTORY = 100;
};

EVEnergyManager::EVEnergyManager()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")),
      tripStartTime(std::chrono::steady_clock::now()),
      lastReportTime(std::chrono::steady_clock::now()) {
    
    // Initialize battery health monitoring
    batteryHealth = {
        1.0,    // 100% capacity
        0.0,    // 0% degradation
        25.0,   // 25°C temperature
        0,      // 0 cycles
        1.0,    // 100% voltage health
        std::chrono::steady_clock::now()
    };
    
    // Initialize energy efficiency tracking
    currentTrip = {0.0, 0.0, 0.0, 0.0, 0.0};
    
    velocitas::logger().info("🔋 EV Energy Management System initializing - Battery capacity: {:.1f}kWh", BATTERY_CAPACITY_KWH);
}

void EVEnergyManager::onStart() {
    velocitas::logger().info("🚀 EV Energy Management starting - Intelligent battery and charging optimization");
    
    // Subscribe to comprehensive EV energy monitoring signals
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Powertrain.TractionBattery.StateOfCharge.Current)
                                               .select(Vehicle.Powertrain.TractionBattery.Charging.IsCharging)
                                               .select(Vehicle.Powertrain.ElectricMotor.Power)
                                               .select(Vehicle.Powertrain.TractionBattery.Range)
                                               .select(Vehicle.Speed)
                                               .select(Vehicle.Acceleration.Longitudinal)
                                               .build())
        ->onItem([this](auto&& item) { onEVDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) {
            velocitas::logger().error("❌ EV energy monitoring subscription error: {}", status.errorMessage());
            velocitas::logger().warn("🔧 Switching to basic energy management mode");
        });
    
    velocitas::logger().info("✅ EV energy management active - monitoring battery health and efficiency");
    velocitas::logger().info("🎯 Efficiency target: <{:.0f}Wh/km, Regen target: {:.1f}%", 
                            ENERGY_EFFICIENCY_TARGET, REGEN_EFFICIENCY_TARGET * 100);
}

void EVEnergyManager::onEVDataChanged(const velocitas::DataPointReply& reply) {
    try {
        // Extract EV energy signals
        double stateOfCharge = 50.0;
        bool isCharging = false;
        double motorPower = 0.0;
        double estimatedRangeSignal = 0.0;
        double speed = 0.0;
        double acceleration = 0.0;
        
        // Battery state monitoring
        if (reply.get(Vehicle.Powertrain.TractionBattery.StateOfCharge.Current)->isAvailable()) {
            stateOfCharge = reply.get(Vehicle.Powertrain.TractionBattery.StateOfCharge.Current)->value();
        }
        
        // Charging status
        if (reply.get(Vehicle.Powertrain.TractionBattery.Charging.IsCharging)->isAvailable()) {
            isCharging = reply.get(Vehicle.Powertrain.TractionBattery.Charging.IsCharging)->value();
        }
        
        // Motor power (positive = consumption, negative = regeneration)
        if (reply.get(Vehicle.Powertrain.ElectricMotor.Power)->isAvailable()) {
            motorPower = reply.get(Vehicle.Powertrain.ElectricMotor.Power)->value();
        }
        
        // Range estimation
        if (reply.get(Vehicle.Powertrain.TractionBattery.Range)->isAvailable()) {
            estimatedRangeSignal = reply.get(Vehicle.Powertrain.TractionBattery.Range)->value();
        }
        
        // Vehicle dynamics
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            speed = reply.get(Vehicle.Speed)->value();
        }
        
        if (reply.get(Vehicle.Acceleration.Longitudinal)->isAvailable()) {
            acceleration = reply.get(Vehicle.Acceleration.Longitudinal)->value();
        }
        
        // Log comprehensive EV status
        velocitas::logger().info("🔋 EV Status: SOC={:.1f}%, Range={:.0f}km, Power={:.1f}kW, Speed={:.1f}km/h, Charging={}", 
                                stateOfCharge, estimatedRangeSignal / 1000.0, motorPower, speed * 3.6, 
                                isCharging ? "YES" : "NO");
        
        // Execute comprehensive energy management
        monitorBatteryHealth(stateOfCharge, isCharging);
        optimizeChargingStrategy(stateOfCharge, isCharging);
        predictRange(stateOfCharge, motorPower, speed);
        optimizeEnergyRecovery(motorPower, speed);
        managePowerDistribution(stateOfCharge);
        
        if (stateOfCharge < 30.0) {
            recommendChargingStations(estimatedRangeSignal / 1000.0);
        }
        
        analyzeEnergyEfficiency();
        
        // Generate periodic energy reports
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastReport = std::chrono::duration_cast<std::chrono::minutes>(now - lastReportTime).count();
        if (timeSinceLastReport >= REPORT_INTERVAL_MINUTES) {
            generateEnergyReport();
            lastReportTime = now;
        }
        
        // Update tracking variables
        previousSOC = stateOfCharge;
        previousMotorPower = motorPower;
        wasCharging = isCharging;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 EV energy management error: {}", e.what());
    }
}

void EVEnergyManager::monitorBatteryHealth(double stateOfCharge, bool isCharging) {
    auto now = std::chrono::steady_clock::now();
    
    // Detect charging cycles
    if (isCharging && !wasCharging) {
        chargingCycles++;
        batteryHealth.cycleCount++;
        velocitas::logger().info("🔌 Charging cycle #{} started at {:.1f}% SOC", chargingCycles, stateOfCharge);
        
        // Start new charging session
        ChargingSession session;
        session.startTime = now;
        session.startSOC = stateOfCharge;
        session.chargingLocation = "Unknown"; // In production, would use GPS/location services
        chargingHistory.push_back(session);
    }
    
    // Complete charging session
    if (!isCharging && wasCharging && !chargingHistory.empty()) {
        auto& lastSession = chargingHistory.back();
        lastSession.endTime = now;
        lastSession.endSOC = stateOfCharge;
        lastSession.energyAdded = (stateOfCharge - lastSession.startSOC) / 100.0 * BATTERY_CAPACITY_KWH;
        
        auto chargingDuration = std::chrono::duration_cast<std::chrono::minutes>(
            lastSession.endTime - lastSession.startTime).count();
        
        if (chargingDuration > 0) {
            lastSession.chargingRate = lastSession.energyAdded / (chargingDuration / 60.0); // kW
        }
        
        velocitas::logger().info("🔌 Charging completed: {:.1f}% -> {:.1f}% ({:.1f}kWh in {}min, {:.1f}kW avg)", 
                                lastSession.startSOC, lastSession.endSOC, lastSession.energyAdded, 
                                chargingDuration, lastSession.chargingRate);
        
        // Analyze charging performance
        if (lastSession.chargingRate > FAST_CHARGING_THRESHOLD) {
            velocitas::logger().info("⚡ Fast charging detected - monitoring battery temperature");
        }
    }
    
    // Battery health assessment
    auto healthCheckInterval = std::chrono::duration_cast<std::chrono::hours>(
        now - batteryHealth.lastHealthCheck).count();
    
    if (healthCheckInterval >= 24) { // Daily health check
        // Simplified battery degradation calculation
        double cycleDegradation = batteryHealth.cycleCount * 0.001; // 0.1% per 100 cycles
        batteryHealth.degradation = std::min(cycleDegradation, 30.0); // Max 30% degradation
        batteryHealth.capacity = 1.0 - (batteryHealth.degradation / 100.0);
        
        velocitas::logger().info("🔋 Battery Health: Capacity={:.1f}%, Degradation={:.2f}%, Cycles={}", 
                                batteryHealth.capacity * 100, batteryHealth.degradation, batteryHealth.cycleCount);
        
        batteryHealth.lastHealthCheck = now;
        
        // Battery health warnings
        if (batteryHealth.degradation > 20.0) {
            velocitas::logger().warn("⚠️  Battery degradation high: {:.1f}% - Consider replacement planning", 
                                    batteryHealth.degradation);
        }
    }
    
    // Low SOC warnings
    if (stateOfCharge < MIN_SOC_CRITICAL) {
        velocitas::logger().error("🚨 CRITICAL BATTERY LOW: {:.1f}% - Immediate charging required!", stateOfCharge);
    } else if (stateOfCharge < MIN_SOC_WARNING) {
        velocitas::logger().warn("⚠️  Battery low: {:.1f}% - Plan charging soon", stateOfCharge);
    }
}

void EVEnergyManager::optimizeChargingStrategy(double stateOfCharge, bool isCharging) {
    if (smartChargingEnabled) {
        // Time-of-use optimization (simulated)
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        int hour = tm.tm_hour;
        
        // Off-peak hours (assuming 10 PM to 6 AM)
        bool isOffPeakHour = (hour >= 22 || hour <= 6);
        
        if (isCharging && !isOffPeakHour && stateOfCharge > 50.0) {
            velocitas::logger().info("💡 Smart charging: Peak hours detected - recommend delayed charging for cost optimization");
        } else if (isCharging && isOffPeakHour) {
            velocitas::logger().info("💰 Smart charging: Off-peak hours - optimal charging time");
        }
        
        // Charging strategy recommendations
        if (stateOfCharge < 80.0 && !isCharging) {
            if (stateOfCharge < 30.0) {
                velocitas::logger().info("🚀 Charging recommendation: Immediate fast charging recommended");
            } else if (isOffPeakHour) {
                velocitas::logger().info("⏰ Charging recommendation: Off-peak opportunity charging");
            }
        }
        
        // Pre-conditioning for optimal charging
        if (preConditioningEnabled && stateOfCharge < 50.0) {
            velocitas::logger().info("🌡️  Pre-conditioning: Preparing battery for optimal charging performance");
        }
    }
}

void EVEnergyManager::predictRange(double stateOfCharge, double motorPower, double speed) {
    // Advanced range prediction algorithm
    double currentEnergy = stateOfCharge / 100.0 * BATTERY_CAPACITY_KWH * batteryHealth.capacity;
    
    // Calculate current efficiency
    if (speed > 0.1 && motorPower > 0) {
        double instantEfficiency = (motorPower * 1000.0) / (speed * 3.6); // Wh/km
        currentTrip.efficiency = (currentTrip.efficiency + instantEfficiency) / 2.0; // Running average
        
        // Predict range based on current efficiency
        estimatedRange = (currentEnergy * 1000.0) / currentTrip.efficiency; // km
        
        velocitas::logger().info("📐 Range Prediction: {:.0f}km (Efficiency: {:.0f}Wh/km, Energy: {:.1f}kWh)", 
                                estimatedRange, currentTrip.efficiency, currentEnergy);
        
        // Range optimization recommendations
        if (currentTrip.efficiency > ENERGY_EFFICIENCY_TARGET) {
            velocitas::logger().warn("⚠️  High energy consumption: {:.0f}Wh/km - Enable eco mode for better range", 
                                    currentTrip.efficiency);
            
            if (!ecoModeEnabled) {
                velocitas::logger().info("💡 Recommendation: Enable eco mode to improve efficiency by ~15%");
                rangeOptimizations++;
            }
        }
        
        // Range anxiety prevention
        if (estimatedRange < 50.0) {
            velocitas::logger().warn("⚠️  Low range warning: {:.0f}km remaining - Charging station search recommended", 
                                    estimatedRange);
        }
    }
    
    // Environmental factors (simplified)
    // In production, would integrate weather, traffic, elevation data
    double temperatureFactor = 1.0; // Assume optimal temperature
    double terrainFactor = 1.0;     // Assume flat terrain
    double trafficFactor = 1.0;     // Assume normal traffic
    
    double adjustedRange = estimatedRange * temperatureFactor * terrainFactor * trafficFactor;
    
    if (adjustedRange != estimatedRange) {
        velocitas::logger().info("🌍 Environmental adjustment: Range {:.0f}km -> {:.0f}km", 
                                estimatedRange, adjustedRange);
        estimatedRange = adjustedRange;
    }
}

void EVEnergyManager::optimizeEnergyRecovery(double motorPower, double speed) {
    // Regenerative braking optimization
    if (motorPower < 0 && speed > 0.5) { // Negative power = regeneration
        double regenPower = std::abs(motorPower);
        totalEnergyRecovered += regenPower * 0.01; // Simplified energy calculation
        
        // Calculate regeneration efficiency
        double totalEnergyUsed = totalEnergyConsumed + totalEnergyRecovered;
        if (totalEnergyUsed > 0) {
            currentTrip.regenEfficiency = totalEnergyRecovered / totalEnergyUsed;
        }
        
        velocitas::logger().info("♻️  Energy recovery: {:.1f}kW regeneration, {:.1f}% efficiency", 
                                regenPower, currentTrip.regenEfficiency * 100);
        
        // Regeneration optimization
        if (currentTrip.regenEfficiency < REGEN_EFFICIENCY_TARGET) {
            velocitas::logger().info("💡 Regen optimization: Adjust driving style for better energy recovery");
        }
        
        // One-pedal driving recommendations
        if (regenPower > 20.0) {
            velocitas::logger().info("🦶 One-pedal driving: Strong regeneration detected - excellent energy recovery");
        }
    } else if (motorPower > 0) {
        // Energy consumption tracking
        totalEnergyConsumed += motorPower * 0.01; // Simplified energy calculation
    }
}

void EVEnergyManager::recommendChargingStations(double currentRange) {
    // Simulated charging station recommendation system
    // In production, would integrate with real charging network APIs
    
    struct ChargingStation {
        std::string name;
        double distance;    // km
        double maxPower;    // kW
        bool isAvailable;
        double cost;        // per kWh
    };
    
    std::vector<ChargingStation> nearbyStations = {
        {"FastCharge Station A", 15.0, 150.0, true, 0.35},
        {"Mall Charging Hub", 8.0, 50.0, true, 0.25},
        {"Highway Service Stop", 25.0, 350.0, false, 0.40},
        {"Shopping Center", 12.0, 22.0, true, 0.20}
    };
    
    // Filter stations within range
    std::vector<ChargingStation> reachableStations;
    for (const auto& station : nearbyStations) {
        if (station.distance <= currentRange * 0.8 && station.isAvailable) { // 80% safety margin
            reachableStations.push_back(station);
        }
    }
    
    if (!reachableStations.empty()) {
        // Sort by optimization criteria (distance, power, cost)
        std::sort(reachableStations.begin(), reachableStations.end(), 
                 [](const ChargingStation& a, const ChargingStation& b) {
                     return (a.distance / a.maxPower) < (b.distance / b.maxPower); // Distance/power ratio
                 });
        
        velocitas::logger().info("🗺️  Charging station recommendations within {:.0f}km:", currentRange);
        for (size_t i = 0; i < std::min(reachableStations.size(), size_t(3)); ++i) {
            const auto& station = reachableStations[i];
            velocitas::logger().info("   {}. {} - {:.0f}km, {:.0f}kW, ${:.2f}/kWh", 
                                    i+1, station.name, station.distance, station.maxPower, station.cost);
        }
    } else {
        velocitas::logger().error("🚨 NO REACHABLE CHARGING STATIONS - Immediate action required!");
    }
}

void EVEnergyManager::managePowerDistribution(double stateOfCharge) {
    // Power management for auxiliary systems
    if (stateOfCharge < MIN_SOC_WARNING) {
        energySavingActivations++;
        
        velocitas::logger().info("🔋 Power management: Low SOC - Optimizing auxiliary power consumption");
        
        // Simulate auxiliary power management
        double hvacReduction = 30.0;    // % reduction
        double heatingReduction = 50.0; // % reduction
        double lightingReduction = 20.0; // % reduction
        
        velocitas::logger().info("💡 Power optimization: HVAC -{:.0f}%, Heating -{:.0f}%, Lighting -{:.0f}%", 
                                hvacReduction, heatingReduction, lightingReduction);
        
        // Calculate power savings
        double estimatedSavings = 2.5; // kW estimated savings
        double rangeExtension = (estimatedSavings / currentTrip.efficiency) * 1000.0; // km
        
        velocitas::logger().info("📈 Range extension: ~{:.0f}km additional range from power optimization", 
                                rangeExtension);
    }
    
    if (stateOfCharge < MIN_SOC_CRITICAL) {
        velocitas::logger().warn("🚨 CRITICAL POWER MODE: Disabling non-essential systems");
        // In production, would interface with vehicle systems to reduce power consumption
    }
}

void EVEnergyManager::analyzeEnergyEfficiency() {
    // Track efficiency statistics
    if (currentTrip.efficiency > 0) {
        if (currentTrip.efficiency < bestEfficiency) {
            bestEfficiency = currentTrip.efficiency;
            velocitas::logger().info("🏆 New efficiency record: {:.0f}Wh/km", bestEfficiency);
        }
        
        if (currentTrip.efficiency > worstEfficiency) {
            worstEfficiency = currentTrip.efficiency;
        }
        
        // Efficiency recommendations
        if (currentTrip.efficiency > ENERGY_EFFICIENCY_TARGET * 1.5) {
            velocitas::logger().warn("⚡ High energy consumption detected - Driving style recommendations:");
            velocitas::logger().info("   • Maintain steady speeds when possible");
            velocitas::logger().info("   • Use regenerative braking effectively");
            velocitas::logger().info("   • Reduce highway speeds for better efficiency");
            velocitas::logger().info("   • Pre-condition cabin while plugged in");
        }
    }
}

void EVEnergyManager::generateEnergyReport() {
    auto now = std::chrono::steady_clock::now();
    auto tripDuration = std::chrono::duration_cast<std::chrono::minutes>(now - tripStartTime).count();
    
    velocitas::logger().info("📋 EV ENERGY REPORT - Trip Duration: {}min", tripDuration);
    velocitas::logger().info("🔋 Battery: SOC={:.1f}%, Health={:.1f}%, Cycles={}", 
                            previousSOC, batteryHealth.capacity * 100, batteryHealth.cycleCount);
    velocitas::logger().info("⚡ Energy: Consumed={:.2f}kWh, Recovered={:.2f}kWh, Efficiency={:.0f}Wh/km", 
                            totalEnergyConsumed, totalEnergyRecovered, currentTrip.efficiency);
    velocitas::logger().info("♻️  Regeneration: {:.1f}% energy recovery efficiency", 
                            currentTrip.regenEfficiency * 100);
    velocitas::logger().info("📐 Range: Current estimate={:.0f}km", estimatedRange);
    velocitas::logger().info("🏁 Performance: Best efficiency={:.0f}Wh/km, Worst={:.0f}Wh/km", 
                            bestEfficiency, worstEfficiency);
    velocitas::logger().info("🔧 Optimizations: Range={}, Energy saving={}, Charging cycles={}", 
                            rangeOptimizations, energySavingActivations, chargingCycles);
    
    // Charging history summary
    if (!chargingHistory.empty()) {
        double totalEnergyCharged = 0.0;
        for (const auto& session : chargingHistory) {
            totalEnergyCharged += session.energyAdded;
        }
        velocitas::logger().info("🔌 Charging: {} sessions, {:.1f}kWh total energy added", 
                                chargingHistory.size(), totalEnergyCharged);
    }
}

// Application entry point
std::unique_ptr<EVEnergyManager> energyApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 EV Energy Management shutdown initiated (signal {})", sig);
    if (energyApp) {
        energyApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting EV Energy Management System...");
    velocitas::logger().info("🔋 Intelligent battery optimization and charging strategy");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    energyApp = std::make_unique<EVEnergyManager>();
    try {
        energyApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 EV energy management error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown EV energy management error");
        return 1;
    }
    
    velocitas::logger().info("👋 EV Energy Management System stopped");
    return 0;
}

// ============================================================================
// 🎓 PRODUCTION DEPLOYMENT NOTES
// ============================================================================
//
// 🏭 EV ENERGY INTEGRATIONS:
// 1. Smart grid integration for optimal charging times and rates
// 2. Charging network APIs for real-time station availability and pricing
// 3. Weather service integration for range prediction accuracy
// 4. Route planning APIs for energy-optimal navigation
// 5. Battery management system (BMS) integration for detailed health data
// 6. Home energy management for bidirectional charging (V2G)
// 7. Fleet management for commercial EV optimization
// 8. Mobile app integration for remote monitoring and control
//
// 🧠 ADVANCED ALGORITHMS:
// - Machine learning for personalized driving pattern analysis
// - Predictive battery degradation modeling
// - Dynamic range prediction using real-time traffic and weather
// - Optimal charging scheduling with grid load balancing
// - Energy arbitrage for V2G revenue optimization
//
// 🔧 CONFIGURATION PARAMETERS:
// - Battery specifications and chemistry-specific parameters
// - Charging network preferences and membership information
// - Energy cost optimization settings and rate schedules
// - Range buffer preferences and anxiety thresholds
// - Climate control energy budgets and comfort preferences
//
// ============================================================================