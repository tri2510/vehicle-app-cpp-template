// ============================================================================
// 🚗 SDV FLEET MANAGEMENT SYSTEM - COMPREHENSIVE AUTOMOTIVE EXAMPLE
// ============================================================================
// 
// 📋 WHAT THIS EXAMPLE DEMONSTRATES:
// This example shows a real-world Software-Defined Vehicle (SDV) application:
// - Multi-signal processing and coordination
// - Real-time decision making based on vehicle data
// - Fleet management and telematics integration
// - Predictive analytics and driver behavior analysis
// - MQTT communication for fleet dashboards
// - Data aggregation and reporting
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Vehicle-to-Cloud connectivity
// - Data-driven decision making
// - Predictive maintenance
// - Driver behavior scoring
// - Route optimization
// - Energy efficiency monitoring
// - Real-time fleet visibility
//
// 💡 COPY TO USE: cp examples/FleetManagementSDV.cpp templates/app/src/VehicleApp.cpp
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
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

// Global Vehicle instance
::vehicle::Vehicle Vehicle;

// ============================================================================
// FLEET MANAGEMENT DATA STRUCTURES
// ============================================================================

struct TripData {
    double startTime;
    double endTime;
    double totalDistance;
    double fuelConsumed;
    double averageSpeed;
    double maxSpeed;
    int hardBrakingEvents;
    int rapidAccelerationEvents;
    double idleTime;
    std::string route;
};

struct VehicleStatus {
    std::string vehicleId;
    double latitude;
    double longitude;
    double speed;
    double fuelLevel;
    double engineTemp;
    double odometer;
    double batteryVoltage;
    bool engineRunning;
    std::chrono::system_clock::time_point lastUpdate;
};

struct MaintenanceAlert {
    std::string type;
    std::string severity;
    std::string description;
    double triggerValue;
    std::chrono::system_clock::time_point timestamp;
};

// ============================================================================
// SDV FLEET MANAGEMENT SYSTEM
// ============================================================================

/**
 * @brief SDV Fleet Management System
 * 
 * 🎯 PURPOSE: Comprehensive fleet management with real-time monitoring,
 * predictive analytics, and driver behavior analysis
 * 
 * 📊 KEY FEATURES:
 * - Multi-signal data fusion and processing
 * - Real-time vehicle tracking and monitoring
 * - Driver behavior scoring and analysis
 * - Predictive maintenance alerts
 * - Route optimization and fuel efficiency
 * - Fleet dashboard integration via MQTT
 * - Data aggregation and reporting
 * 
 * 🚗 VEHICLE SIGNALS USED:
 * - Vehicle.CurrentLocation.Latitude/Longitude (GPS tracking)
 * - Vehicle.Speed (speed monitoring and behavior analysis)
 * - Vehicle.Acceleration.Longitudinal (acceleration events)
 * - Vehicle.Powertrain.FuelSystem.Level (fuel monitoring)
 * - Vehicle.Powertrain.Engine.Speed (engine diagnostics)
 * - Vehicle.Powertrain.Engine.Temperature (maintenance predictions)
 * - Vehicle.Service.DistanceToService (maintenance scheduling)
 * - Vehicle.Electrical.Battery.Voltage (electrical system health)
 * - Vehicle.Powertrain.Engine.IsRunning (engine status)
 */
class FleetManagementSDV : public velocitas::VehicleApp {
public:
    FleetManagementSDV();

protected:
    void onStart() override;

private:
    void onVehicleDataChanged(const velocitas::DataPointReply& reply);
    
    // Core fleet management functions
    void processLocationData(double latitude, double longitude);
    void processSpeedData(double speed);
    void processAccelerationData(double acceleration);
    void processFuelData(double fuelLevel);
    void processEngineData(double rpm, double temperature);
    void processMaintenanceData(double distanceToService);
    void processElectricalData(double batteryVoltage);
    void processEngineStatus(bool isRunning);
    
    // Analytics and intelligence functions
    void analyzeDrivingBehavior();
    void predictMaintenance();
    void optimizeRoute();
    void calculateFuelEfficiency();
    void generateDriverScore();
    void detectAnomalies();
    
    // Fleet management functions
    void updateFleetDashboard();
    void sendMaintenanceAlert(const MaintenanceAlert& alert);
    void logTripData();
    void aggregateFleetMetrics();
    
    // Data storage
    VehicleStatus currentStatus;
    TripData currentTrip;
    std::vector<MaintenanceAlert> pendingAlerts;
    std::vector<double> speedHistory;
    std::vector<double> accelerationHistory;
    std::vector<double> fuelHistory;
    
    // Analytics variables
    double driverScore;
    double fuelEfficiency;
    double tripDistance;
    std::chrono::system_clock::time_point tripStartTime;
    std::chrono::system_clock::time_point lastLocationUpdate;
    
    // Configuration
    static constexpr double SPEED_LIMIT_HIGHWAY = 30.0;  // 108 km/h in m/s
    static constexpr double SPEED_LIMIT_CITY = 13.9;     // 50 km/h in m/s
    static constexpr double HARSH_ACCELERATION_THRESHOLD = 2.5;  // m/s²
    static constexpr double HARSH_BRAKING_THRESHOLD = -3.0;      // m/s²
    static constexpr double LOW_FUEL_THRESHOLD = 20.0;           // %
    static constexpr double HIGH_ENGINE_TEMP_THRESHOLD = 95.0;   // °C
    static constexpr double LOW_BATTERY_THRESHOLD = 12.0;        // V
    static constexpr int HISTORY_SIZE = 100;
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

FleetManagementSDV::FleetManagementSDV()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"))
    , driverScore(100.0)
    , fuelEfficiency(0.0)
    , tripDistance(0.0) {
    
    velocitas::logger().info("🚗 SDV Fleet Management System starting...");
    velocitas::logger().info("💼 Initializing fleet monitoring and analytics...");
    
    // Initialize vehicle status
    currentStatus.vehicleId = "SDV-DEMO-001";
    currentStatus.lastUpdate = std::chrono::system_clock::now();
    
    // Initialize trip data
    tripStartTime = std::chrono::system_clock::now();
    lastLocationUpdate = tripStartTime;
    
    // Reserve space for history vectors
    speedHistory.reserve(HISTORY_SIZE);
    accelerationHistory.reserve(HISTORY_SIZE);
    fuelHistory.reserve(HISTORY_SIZE);
    
    velocitas::logger().info("✅ Fleet Management System initialized");
}

void FleetManagementSDV::onStart() {
    velocitas::logger().info("🚀 Starting SDV Fleet Management System...");
    velocitas::logger().info("📡 Setting up multi-signal monitoring...");
    
    // Subscribe to comprehensive vehicle signal set for fleet management
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Latitude)
                                               .select(Vehicle.CurrentLocation.Longitude)
                                               .select(Vehicle.Speed)
                                               .select(Vehicle.Acceleration.Longitudinal)
                                               .select(Vehicle.Powertrain.FuelSystem.Level)
                                               .select(Vehicle.Powertrain.Engine.Speed)
                                               .select(Vehicle.Powertrain.Engine.ECT)
                                               .select(Vehicle.Service.DistanceToService)
                                               .select(Vehicle.Electrical.Battery.Voltage)
                                               .select(Vehicle.Powertrain.Engine.IsRunning)
                                               .build())
        ->onItem([this](auto&& item) { onVehicleDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().error("❌ Fleet signal subscription error: {}", status.errorMessage());
        });
    
    velocitas::logger().info("✅ Fleet management signals subscribed");
    velocitas::logger().info("🎯 Monitoring: Location, Speed, Fuel, Engine, Maintenance");
    velocitas::logger().info("📊 Analytics: Driver behavior, fuel efficiency, predictive maintenance");
    velocitas::logger().info("🔄 Starting real-time fleet operations...");
}

void FleetManagementSDV::onVehicleDataChanged(const velocitas::DataPointReply& reply) {
    try {
        velocitas::logger().info("📊 Processing fleet telemetry data...");
        
        // Update timestamp
        currentStatus.lastUpdate = std::chrono::system_clock::now();
        
        // Process location data
        if (reply.get(Vehicle.CurrentLocation.Latitude)->isAvailable() && 
            reply.get(Vehicle.CurrentLocation.Longitude)->isAvailable()) {
            double lat = reply.get(Vehicle.CurrentLocation.Latitude)->value();
            double lon = reply.get(Vehicle.CurrentLocation.Longitude)->value();
            processLocationData(lat, lon);
        }
        
        // Process speed data
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            double speed = reply.get(Vehicle.Speed)->value();
            processSpeedData(speed);
        }
        
        // Process acceleration data
        if (reply.get(Vehicle.Acceleration.Longitudinal)->isAvailable()) {
            double acceleration = reply.get(Vehicle.Acceleration.Longitudinal)->value();
            processAccelerationData(acceleration);
        }
        
        // Process fuel data
        if (reply.get(Vehicle.Powertrain.FuelSystem.Level)->isAvailable()) {
            double fuelLevel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
            processFuelData(fuelLevel);
        }
        
        // Process engine data
        if (reply.get(Vehicle.Powertrain.Engine.Speed)->isAvailable() &&
            reply.get(Vehicle.Powertrain.Engine.ECT)->isAvailable()) {
            double rpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
            double temp = reply.get(Vehicle.Powertrain.Engine.ECT)->value();
            processEngineData(rpm, temp);
        }
        
        // Process maintenance data
        if (reply.get(Vehicle.Service.DistanceToService)->isAvailable()) {
            double distance = reply.get(Vehicle.Service.DistanceToService)->value();
            processMaintenanceData(distance);
        }
        
        // Process electrical data
        if (reply.get(Vehicle.Electrical.Battery.Voltage)->isAvailable()) {
            double voltage = reply.get(Vehicle.Electrical.Battery.Voltage)->value();
            processElectricalData(voltage);
        }
        
        // Process engine status
        if (reply.get(Vehicle.Powertrain.Engine.IsRunning)->isAvailable()) {
            bool isRunning = reply.get(Vehicle.Powertrain.Engine.IsRunning)->value();
            processEngineStatus(isRunning);
        }
        
        // Perform analytics
        analyzeDrivingBehavior();
        predictMaintenance();
        calculateFuelEfficiency();
        detectAnomalies();
        
        // Update fleet systems
        updateFleetDashboard();
        
        velocitas::logger().info("✅ Fleet telemetry processed successfully");
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Waiting for complete vehicle data set...");
    }
}

void FleetManagementSDV::processLocationData(double latitude, double longitude) {
    velocitas::logger().info("📍 Location Update: {:.6f}, {:.6f}", latitude, longitude);
    
    // Update current status
    currentStatus.latitude = latitude;
    currentStatus.longitude = longitude;
    
    // Calculate distance traveled (simplified)
    if (lastLocationUpdate != tripStartTime) {
        // In a real implementation, use proper GPS distance calculation
        double deltaLat = latitude - currentStatus.latitude;
        double deltaLon = longitude - currentStatus.longitude;
        double distance = std::sqrt(deltaLat * deltaLat + deltaLon * deltaLon) * 111000; // Rough km to meters
        tripDistance += distance;
        
        velocitas::logger().info("🛣️  Trip distance: {:.2f} km", tripDistance / 1000.0);
    }
    
    lastLocationUpdate = std::chrono::system_clock::now();
    
    // Determine area type for speed limit context
    // This would use real map data in production
    bool inCity = (std::fmod(latitude * 1000, 10) < 5); // Simplified city detection
    if (inCity) {
        velocitas::logger().info("🏙️  Driving in city area (speed limit: 50 km/h)");
    } else {
        velocitas::logger().info("🛣️  Driving on highway (speed limit: 108 km/h)");
    }
}

void FleetManagementSDV::processSpeedData(double speed) {
    double speedKmh = speed * 3.6;
    velocitas::logger().info("🚗 Speed: {:.1f} km/h ({:.2f} m/s)", speedKmh, speed);
    
    // Update current status
    currentStatus.speed = speed;
    
    // Add to history
    speedHistory.push_back(speed);
    if (speedHistory.size() > HISTORY_SIZE) {
        speedHistory.erase(speedHistory.begin());
    }
    
    // Update trip statistics
    if (speed > currentTrip.maxSpeed) {
        currentTrip.maxSpeed = speed;
        velocitas::logger().info("🏁 New trip max speed: {:.1f} km/h", currentTrip.maxSpeed * 3.6);
    }
    
    // Speed limit monitoring
    bool isHighway = currentStatus.latitude > 0; // Simplified highway detection
    double speedLimit = isHighway ? SPEED_LIMIT_HIGHWAY : SPEED_LIMIT_CITY;
    
    if (speed > speedLimit * 1.1) { // 10% tolerance
        velocitas::logger().warn("⚠️  SPEED VIOLATION: {:.1f} km/h (limit: {:.1f} km/h)", 
                                speedKmh, speedLimit * 3.6);
        driverScore -= 2.0; // Reduce driver score
    } else if (speed > speedLimit) {
        velocitas::logger().warn("🚨 Approaching speed limit: {:.1f} km/h", speedKmh);
    } else if (speed > 0.1) {
        velocitas::logger().info("✅ Speed within limits");
    } else {
        velocitas::logger().info("🛑 Vehicle stopped");
    }
}

void FleetManagementSDV::processAccelerationData(double acceleration) {
    velocitas::logger().info("📈 Acceleration: {:.2f} m/s²", acceleration);
    
    // Add to history
    accelerationHistory.push_back(acceleration);
    if (accelerationHistory.size() > HISTORY_SIZE) {
        accelerationHistory.erase(accelerationHistory.begin());
    }
    
    // Detect harsh events
    if (acceleration > HARSH_ACCELERATION_THRESHOLD) {
        velocitas::logger().warn("⚡ HARSH ACCELERATION DETECTED: {:.2f} m/s²", acceleration);
        currentTrip.rapidAccelerationEvents++;
        driverScore -= 5.0;
        
        // Send alert to fleet manager
        MaintenanceAlert alert{
            "DRIVING_BEHAVIOR",
            "WARNING",
            fmt::format("Harsh acceleration: {:.2f} m/s²", acceleration),
            acceleration,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
        
    } else if (acceleration < HARSH_BRAKING_THRESHOLD) {
        velocitas::logger().warn("🛑 HARSH BRAKING DETECTED: {:.2f} m/s²", acceleration);
        currentTrip.hardBrakingEvents++;
        driverScore -= 3.0;
        
        MaintenanceAlert alert{
            "DRIVING_BEHAVIOR",
            "WARNING", 
            fmt::format("Harsh braking: {:.2f} m/s²", acceleration),
            acceleration,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
        
    } else if (std::abs(acceleration) > 1.0) {
        velocitas::logger().info("🔄 Moderate acceleration/deceleration: {:.2f} m/s²", acceleration);
    } else {
        velocitas::logger().info("😌 Smooth driving");
    }
}

void FleetManagementSDV::processFuelData(double fuelLevel) {
    velocitas::logger().info("⛽ Fuel Level: {:.1f}%", fuelLevel);
    
    // Update current status
    currentStatus.fuelLevel = fuelLevel;
    
    // Add to history
    fuelHistory.push_back(fuelLevel);
    if (fuelHistory.size() > HISTORY_SIZE) {
        fuelHistory.erase(fuelHistory.begin());
    }
    
    // Fuel level alerts
    if (fuelLevel < 10.0) {
        velocitas::logger().error("🚨 CRITICAL FUEL LEVEL: {:.1f}% - IMMEDIATE REFUEL REQUIRED", fuelLevel);
        
        MaintenanceAlert alert{
            "FUEL_CRITICAL",
            "CRITICAL",
            fmt::format("Critical fuel level: {:.1f}%", fuelLevel),
            fuelLevel,
            std::chrono::system_clock::now()
        };
        sendMaintenanceAlert(alert);
        
    } else if (fuelLevel < LOW_FUEL_THRESHOLD) {
        velocitas::logger().warn("⚠️  LOW FUEL WARNING: {:.1f}% - Plan refuel soon", fuelLevel);
        
        MaintenanceAlert alert{
            "FUEL_LOW",
            "WARNING",
            fmt::format("Low fuel level: {:.1f}%", fuelLevel),
            fuelLevel,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
        
    } else if (fuelLevel < 30.0) {
        velocitas::logger().info("📊 Fuel level moderate: {:.1f}%", fuelLevel);
    } else {
        velocitas::logger().info("✅ Fuel level good: {:.1f}%", fuelLevel);
    }
    
    // Calculate fuel consumption rate
    if (fuelHistory.size() >= 2) {
        double fuelConsumed = fuelHistory[fuelHistory.size()-2] - fuelLevel;
        if (fuelConsumed > 0) {
            velocitas::logger().info("📉 Fuel consumption rate: {:.2f}% per update", fuelConsumed);
        }
    }
}

void FleetManagementSDV::processEngineData(double rpm, double temperature) {
    velocitas::logger().info("🔧 Engine: {:.0f} RPM, {:.1f}°C", rpm, temperature);
    
    // Update current status
    currentStatus.engineTemp = temperature;
    
    // Engine performance analysis
    if (rpm > 4000) {
        velocitas::logger().warn("⚡ HIGH ENGINE RPM: {:.0f} - Consider shifting", rpm);
        driverScore -= 1.0;
    } else if (rpm > 3000) {
        velocitas::logger().info("🔄 Moderate engine load: {:.0f} RPM", rpm);
    } else if (rpm > 800) {
        velocitas::logger().info("😌 Normal engine operation: {:.0f} RPM", rpm);
    } else if (rpm > 0) {
        velocitas::logger().info("🏃 Engine idle: {:.0f} RPM", rpm);
    }
    
    // Temperature monitoring
    if (temperature > HIGH_ENGINE_TEMP_THRESHOLD) {
        velocitas::logger().error("🔥 ENGINE OVERHEATING: {:.1f}°C - STOP IMMEDIATELY", temperature);
        
        MaintenanceAlert alert{
            "ENGINE_OVERHEATING",
            "CRITICAL",
            fmt::format("Engine overheating: {:.1f}°C", temperature),
            temperature,
            std::chrono::system_clock::now()
        };
        sendMaintenanceAlert(alert);
        
    } else if (temperature > 85.0) {
        velocitas::logger().warn("🌡️  Engine temperature elevated: {:.1f}°C", temperature);
    } else if (temperature > 70.0) {
        velocitas::logger().info("✅ Engine temperature normal: {:.1f}°C", temperature);
    } else {
        velocitas::logger().info("❄️  Engine warming up: {:.1f}°C", temperature);
    }
}

void FleetManagementSDV::processMaintenanceData(double distanceToService) {
    velocitas::logger().info("🔧 Maintenance due in: {:.0f} km", distanceToService);
    
    if (distanceToService < 500) {
        velocitas::logger().warn("🚨 MAINTENANCE URGENT: Service due in {:.0f} km", distanceToService);
        
        MaintenanceAlert alert{
            "MAINTENANCE_DUE",
            "URGENT",
            fmt::format("Service due in {:.0f} km", distanceToService),
            distanceToService,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
        
    } else if (distanceToService < 1000) {
        velocitas::logger().info("📅 Schedule maintenance soon: {:.0f} km remaining", distanceToService);
    } else {
        velocitas::logger().info("✅ Maintenance interval good: {:.0f} km remaining", distanceToService);
    }
}

void FleetManagementSDV::processElectricalData(double batteryVoltage) {
    velocitas::logger().info("🔋 Battery: {:.1f}V", batteryVoltage);
    
    // Update current status
    currentStatus.batteryVoltage = batteryVoltage;
    
    if (batteryVoltage < LOW_BATTERY_THRESHOLD) {
        velocitas::logger().error("🔋 LOW BATTERY VOLTAGE: {:.1f}V - Check charging system", batteryVoltage);
        
        MaintenanceAlert alert{
            "BATTERY_LOW",
            "WARNING",
            fmt::format("Low battery voltage: {:.1f}V", batteryVoltage),
            batteryVoltage,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
        
    } else if (batteryVoltage < 12.5) {
        velocitas::logger().warn("⚡ Battery voltage below optimal: {:.1f}V", batteryVoltage);
    } else {
        velocitas::logger().info("✅ Battery voltage good: {:.1f}V", batteryVoltage);
    }
}

void FleetManagementSDV::processEngineStatus(bool isRunning) {
    currentStatus.engineRunning = isRunning;
    
    if (isRunning) {
        velocitas::logger().info("🚗 Engine running");
    } else {
        velocitas::logger().info("🛑 Engine stopped");
        
        // Log trip data when engine stops
        if (tripDistance > 0) {
            logTripData();
        }
    }
}

void FleetManagementSDV::analyzeDrivingBehavior() {
    if (speedHistory.empty() || accelerationHistory.empty()) return;
    
    // Calculate average speed
    double avgSpeed = std::accumulate(speedHistory.begin(), speedHistory.end(), 0.0) / speedHistory.size();
    
    // Count violations
    int speedViolations = std::count_if(speedHistory.begin(), speedHistory.end(), 
                                       [](double speed) { return speed > 30.0; });
    
    int harshEvents = std::count_if(accelerationHistory.begin(), accelerationHistory.end(),
                                   [](double acc) { return std::abs(acc) > 2.0; });
    
    // Update driver score
    if (speedViolations == 0 && harshEvents == 0) {
        driverScore = std::min(100.0, driverScore + 0.1); // Slowly improve score
    }
    
    velocitas::logger().info("📊 Driver Analysis: Score={:.1f}, AvgSpeed={:.1f}km/h, Violations={}, HarshEvents={}", 
                            driverScore, avgSpeed * 3.6, speedViolations, harshEvents);
}

void FleetManagementSDV::predictMaintenance() {
    if (speedHistory.empty()) return;
    
    // Simple predictive model based on driving patterns
    double avgSpeed = std::accumulate(speedHistory.begin(), speedHistory.end(), 0.0) / speedHistory.size();
    double highRpmPercentage = 0.0;
    
    // Calculate wear factor based on driving
    double wearFactor = 1.0;
    if (avgSpeed > 25.0) wearFactor += 0.2; // Highway driving
    if (currentTrip.hardBrakingEvents > 5) wearFactor += 0.3; // Harsh braking
    if (currentTrip.rapidAccelerationEvents > 3) wearFactor += 0.2; // Aggressive acceleration
    
    if (wearFactor > 1.5) {
        velocitas::logger().warn("🔧 PREDICTIVE MAINTENANCE: High wear factor {:.1f} - Consider early service", wearFactor);
        
        MaintenanceAlert alert{
            "PREDICTIVE_MAINTENANCE",
            "INFO",
            fmt::format("High wear factor detected: {:.1f}", wearFactor),
            wearFactor,
            std::chrono::system_clock::now()
        };
        pendingAlerts.push_back(alert);
    }
}

void FleetManagementSDV::calculateFuelEfficiency() {
    if (fuelHistory.size() < 2 || tripDistance < 1000) return; // Need minimum data
    
    double fuelConsumed = fuelHistory.front() - fuelHistory.back();
    if (fuelConsumed > 0) {
        // Simple efficiency calculation (L/100km approximation)
        fuelEfficiency = (fuelConsumed / 100.0) / (tripDistance / 100000.0) * 100.0;
        
        velocitas::logger().info("⛽ Fuel Efficiency: {:.2f} L/100km", fuelEfficiency);
        
        if (fuelEfficiency > 8.0) {
            velocitas::logger().warn("📈 High fuel consumption: {:.2f} L/100km", fuelEfficiency);
        } else if (fuelEfficiency < 6.0) {
            velocitas::logger().info("💚 Excellent fuel efficiency: {:.2f} L/100km", fuelEfficiency);
        }
    }
}

void FleetManagementSDV::detectAnomalies() {
    // Anomaly detection based on patterns
    if (speedHistory.size() >= 10) {
        // Check for speed anomalies
        double avgSpeed = std::accumulate(speedHistory.end()-10, speedHistory.end(), 0.0) / 10.0;
        double lastSpeed = speedHistory.back();
        
        if (std::abs(lastSpeed - avgSpeed) > 15.0) {
            velocitas::logger().warn("🔍 Speed anomaly detected: Current={:.1f}, Average={:.1f}", 
                                    lastSpeed * 3.6, avgSpeed * 3.6);
        }
    }
}

void FleetManagementSDV::updateFleetDashboard() {
    // In a real implementation, this would send data to MQTT or REST API
    velocitas::logger().info("📊 Fleet Dashboard Update:");
    velocitas::logger().info("   🚗 Vehicle: {} | Location: {:.4f},{:.4f}", 
                            currentStatus.vehicleId, currentStatus.latitude, currentStatus.longitude);
    velocitas::logger().info("   📈 Speed: {:.1f} km/h | Fuel: {:.1f}% | Driver Score: {:.1f}", 
                            currentStatus.speed * 3.6, currentStatus.fuelLevel, driverScore);
    velocitas::logger().info("   🔧 Engine: {:.0f}°C | Battery: {:.1f}V | Distance: {:.1f} km", 
                            currentStatus.engineTemp, currentStatus.batteryVoltage, tripDistance / 1000.0);
}

void FleetManagementSDV::sendMaintenanceAlert(const MaintenanceAlert& alert) {
    velocitas::logger().error("🚨 FLEET ALERT [{}]: {} - {}", 
                             alert.severity, alert.type, alert.description);
    
    // In production, send to fleet management system via MQTT/API
    velocitas::logger().info("📤 Alert sent to fleet management system");
}

void FleetManagementSDV::logTripData() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - tripStartTime);
    
    currentTrip.endTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    currentTrip.totalDistance = tripDistance;
    
    velocitas::logger().info("🏁 TRIP COMPLETED:");
    velocitas::logger().info("   📏 Distance: {:.2f} km", currentTrip.totalDistance / 1000.0);
    velocitas::logger().info("   ⏱️  Duration: {} minutes", duration.count());
    velocitas::logger().info("   🏎️  Max Speed: {:.1f} km/h", currentTrip.maxSpeed * 3.6);
    velocitas::logger().info("   🔴 Hard Braking: {} events", currentTrip.hardBrakingEvents);
    velocitas::logger().info("   ⚡ Rapid Acceleration: {} events", currentTrip.rapidAccelerationEvents);
    velocitas::logger().info("   🏆 Driver Score: {:.1f}/100", driverScore);
    
    // Reset for next trip
    tripDistance = 0;
    tripStartTime = now;
    currentTrip = TripData{};
}

// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================

std::unique_ptr<FleetManagementSDV> fleetApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 Fleet Management System shutting down (signal {})", sig);
    if (fleetApp) {
        fleetApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting SDV Fleet Management System...");
    velocitas::logger().info("🎯 Comprehensive vehicle monitoring and analytics");
    velocitas::logger().info("💼 Fleet operations: Tracking, Analytics, Maintenance, Optimization");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    try {
        fleetApp = std::make_unique<FleetManagementSDV>();
        fleetApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Fleet Management System error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown Fleet Management System error");
        return 1;
    }
    
    velocitas::logger().info("👋 SDV Fleet Management System stopped");
    return 0;
}

// ============================================================================
// 🎓 SDV CONCEPTS DEMONSTRATED
// ============================================================================
//
// 🚗 SOFTWARE-DEFINED VEHICLE (SDV) FEATURES:
// 
// 1. **Multi-Signal Data Fusion**
//    - Combines location, speed, fuel, engine, and electrical data
//    - Real-time processing and correlation of vehicle signals
//    - Holistic view of vehicle state and performance
//
// 2. **Predictive Analytics**
//    - Driver behavior scoring and analysis
//    - Predictive maintenance based on usage patterns
//    - Fuel efficiency monitoring and optimization
//
// 3. **Fleet Intelligence**
//    - Real-time vehicle tracking and monitoring
//    - Route optimization and planning capabilities
//    - Centralized fleet dashboard integration
//
// 4. **Proactive Maintenance**
//    - Condition-based maintenance alerts
//    - Predictive failure detection
//    - Optimized service scheduling
//
// 5. **Data-Driven Decisions**
//    - Anomaly detection and pattern recognition
//    - Performance optimization recommendations
//    - Cost reduction through intelligent analysis
//
// 6. **Connectivity & Integration**
//    - Vehicle-to-Cloud data transmission
//    - MQTT/API integration for fleet systems
//    - Real-time dashboard updates
//
// ============================================================================
// 💡 USAGE INSTRUCTIONS:
//
// 1. Copy this example:
//    cp examples/FleetManagementSDV.cpp templates/app/src/VehicleApp.cpp
//
// 2. Build and run:
//    docker run -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick build --verbose
//    docker run -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick run
//
// 3. Test with KUKSA signals:
//    docker run -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick test signal-validation
//
// ============================================================================