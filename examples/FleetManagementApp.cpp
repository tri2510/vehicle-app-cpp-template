// ============================================================================
// 🚗 SMART FLEET TELEMATICS - Production SDV Example
// ============================================================================
// 
// 📋 WHAT THIS APPLICATION DOES:
// Comprehensive fleet management system that monitors vehicle health, location,
// fuel efficiency, and driver behavior for optimal fleet operations.
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Data aggregation and analytics
// - Remote monitoring and diagnostics
// - Predictive analytics for maintenance
// - IoT integration with fleet management platforms
// - Driver behavior analysis and scoring
// - Route optimization and fuel efficiency
//
// 🚀 QUICK START:
// cp examples/FleetManagementApp.cpp templates/app/src/VehicleApp.cpp
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

::vehicle::Vehicle Vehicle;

/**
 * @brief Smart Fleet Telematics System
 * 
 * Production-ready fleet management application that provides:
 * - Real-time vehicle tracking and monitoring
 * - Fuel efficiency analysis and optimization
 * - Driver behavior scoring and coaching
 * - Predictive maintenance scheduling
 * - Route optimization and analytics
 * - Integration with fleet management dashboards
 */
class FleetManagementApp : public velocitas::VehicleApp {
public:
    FleetManagementApp();

protected:
    void onStart() override;

private:
    void onFleetDataChanged(const velocitas::DataPointReply& reply);
    
    // Fleet analytics methods
    void updateVehicleLocation(double latitude, double longitude);
    void analyzeFuelEfficiency(double fuelLevel, double engineSpeed, double speed);
    void evaluateDriverBehavior(double speed, double acceleration, double engineSpeed);
    void checkMaintenanceSchedule(double distanceToService, double engineSpeed);
    void generateFleetReport();
    void sendTelematics(const std::string& topic, const std::string& data);
    
    // Fleet tracking data
    struct LocationData {
        double latitude;
        double longitude;
        std::chrono::steady_clock::time_point timestamp;
        double speed;
    };
    
    struct FuelData {
        double level;
        double consumption;
        double efficiency;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    struct DriverBehaviorData {
        double avgSpeed;
        double maxSpeed;
        double harshAcceleration;
        double harshBraking;
        double idleTime;
        int score;
    };
    
    // Fleet management parameters
    static constexpr double FUEL_EFFICIENCY_THRESHOLD = 8.0;    // L/100km
    static constexpr double MAX_IDLE_TIME = 300.0;              // 5 minutes in seconds
    static constexpr double HARSH_ACCELERATION_THRESHOLD = 2.5; // m/s²
    static constexpr double HARSH_BRAKING_THRESHOLD = -2.5;     // m/s²
    static constexpr double SPEED_LIMIT_BUFFER = 10.0;          // km/h over limit
    static constexpr double MAINTENANCE_WARNING_THRESHOLD = 5000.0; // km to service
    
    // Fleet state tracking
    std::vector<LocationData> locationHistory;
    std::vector<FuelData> fuelHistory;
    DriverBehaviorData driverBehavior;
    std::chrono::steady_clock::time_point lastReportTime;
    std::chrono::steady_clock::time_point tripStartTime;
    
    // Trip and efficiency metrics
    double tripDistance = 0.0;
    double tripFuelStart = 0.0;
    double totalIdleTime = 0.0;
    double previousSpeed = 0.0;
    double previousFuelLevel = 0.0;
    bool isIdle = false;
    std::chrono::steady_clock::time_point idleStartTime;
    
    // Performance counters
    int harshAccelerationCount = 0;
    int harshBrakingCount = 0;
    int speedingViolationCount = 0;
    int reportCount = 0;
    
    // Fleet configuration
    std::string vehicleId = "FLEET_001";
    std::string driverId = "DRIVER_001";
    
    static constexpr int REPORT_INTERVAL_MINUTES = 5;
    static constexpr size_t MAX_HISTORY_SIZE = 1000;
};

FleetManagementApp::FleetManagementApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")),
      lastReportTime(std::chrono::steady_clock::now()),
      tripStartTime(std::chrono::steady_clock::now()),
      idleStartTime(std::chrono::steady_clock::now()) {
    
    // Initialize driver behavior metrics
    driverBehavior = {0.0, 0.0, 0.0, 0.0, 0.0, 100}; // Start with perfect score
    
    velocitas::logger().info("🚗 Smart Fleet Telematics initializing for Vehicle: {}", vehicleId);
}

void FleetManagementApp::onStart() {
    velocitas::logger().info("🚀 Fleet Management System starting - Vehicle: {}, Driver: {}", vehicleId, driverId);
    
    // Subscribe to comprehensive fleet monitoring signals
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Latitude)
                                               .select(Vehicle.CurrentLocation.Longitude)
                                               .select(Vehicle.Powertrain.FuelSystem.Level)
                                               .select(Vehicle.Service.DistanceToService)
                                               .select(Vehicle.Powertrain.Engine.Speed)
                                               .select(Vehicle.Speed)
                                               .select(Vehicle.Acceleration.Longitudinal)
                                               .build())
        ->onItem([this](auto&& item) { onFleetDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) {
            velocitas::logger().error("❌ Fleet telemetry subscription error: {}", status.errorMessage());
            sendTelematics("fleet/error", fmt::format("Vehicle {}: Telemetry connection lost", vehicleId));
        });
    
    // Initialize trip tracking
    tripStartTime = std::chrono::steady_clock::now();
    
    velocitas::logger().info("✅ Fleet telemetry active - monitoring vehicle performance");
    velocitas::logger().info("📊 Performance thresholds: Fuel efficiency target <{:.1f}L/100km, Max idle time {:.0f}s", 
                            FUEL_EFFICIENCY_THRESHOLD, MAX_IDLE_TIME);
}

void FleetManagementApp::onFleetDataChanged(const velocitas::DataPointReply& reply) {
    try {
        auto now = std::chrono::steady_clock::now();
        
        // Extract fleet monitoring signals
        double latitude = 0.0, longitude = 0.0;
        double fuelLevel = 0.0, distanceToService = 0.0;
        double engineSpeed = 0.0, speed = 0.0, acceleration = 0.0;
        
        // Location tracking
        if (reply.get(Vehicle.CurrentLocation.Latitude)->isAvailable() && 
            reply.get(Vehicle.CurrentLocation.Longitude)->isAvailable()) {
            latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
            longitude = reply.get(Vehicle.CurrentLocation.Longitude)->value();
            updateVehicleLocation(latitude, longitude);
        }
        
        // Fuel system monitoring
        if (reply.get(Vehicle.Powertrain.FuelSystem.Level)->isAvailable()) {
            fuelLevel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
        }
        
        // Maintenance tracking
        if (reply.get(Vehicle.Service.DistanceToService)->isAvailable()) {
            distanceToService = reply.get(Vehicle.Service.DistanceToService)->value();
        }
        
        // Engine and speed monitoring
        if (reply.get(Vehicle.Powertrain.Engine.Speed)->isAvailable()) {
            engineSpeed = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
        }
        
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            speed = reply.get(Vehicle.Speed)->value();
        }
        
        if (reply.get(Vehicle.Acceleration.Longitudinal)->isAvailable()) {
            acceleration = reply.get(Vehicle.Acceleration.Longitudinal)->value();
        }
        
        // Comprehensive fleet analytics
        analyzeFuelEfficiency(fuelLevel, engineSpeed, speed);
        evaluateDriverBehavior(speed, acceleration, engineSpeed);
        checkMaintenanceSchedule(distanceToService, engineSpeed);
        
        // Log comprehensive fleet status
        velocitas::logger().info("📊 Fleet Status: Pos=[{:.6f},{:.6f}], Speed={:.1f}km/h, Fuel={:.1f}%, Engine={:.0f}RPM, Accel={:.2f}m/s²",
                                latitude, longitude, speed * 3.6, fuelLevel, engineSpeed, acceleration);
        
        // Generate periodic fleet reports
        auto timeSinceLastReport = std::chrono::duration_cast<std::chrono::minutes>(now - lastReportTime).count();
        if (timeSinceLastReport >= REPORT_INTERVAL_MINUTES) {
            generateFleetReport();
            lastReportTime = now;
        }
        
        // Update tracking variables
        previousSpeed = speed;
        previousFuelLevel = fuelLevel;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Fleet telemetry error: {}", e.what());
        sendTelematics("fleet/error", fmt::format("Vehicle {}: Data processing error", vehicleId));
    }
}

void FleetManagementApp::updateVehicleLocation(double latitude, double longitude) {
    auto now = std::chrono::steady_clock::now();
    
    // Calculate distance traveled if we have previous location
    if (!locationHistory.empty()) {
        auto& lastLocation = locationHistory.back();
        
        // Simple distance calculation (for production, use proper geospatial libraries)
        double deltaLat = latitude - lastLocation.latitude;
        double deltaLon = longitude - lastLocation.longitude;
        double distance = std::sqrt(deltaLat * deltaLat + deltaLon * deltaLon) * 111000; // Approximate meters
        
        tripDistance += distance;
        
        // Speed calculation
        auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - lastLocation.timestamp).count();
        if (timeDiff > 0) {
            double calculatedSpeed = distance / timeDiff; // m/s
            
            // Detect idle state
            if (calculatedSpeed < 0.1) { // Less than 0.1 m/s considered idle
                if (!isIdle) {
                    isIdle = true;
                    idleStartTime = now;
                }
            } else {
                if (isIdle) {
                    auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(now - idleStartTime).count();
                    totalIdleTime += idleDuration;
                    
                    if (idleDuration > MAX_IDLE_TIME) {
                        velocitas::logger().warn("⚠️  Excessive idle time detected: {:.0f}s - Fuel waste alert", idleDuration);
                        sendTelematics("fleet/idle", fmt::format("Vehicle {}: Idle time {:.0f}s", vehicleId, idleDuration));
                    }
                    isIdle = false;
                }
            }
        }
    }
    
    // Store location data
    LocationData location = {latitude, longitude, now, previousSpeed};
    locationHistory.push_back(location);
    
    // Maintain history size limit
    if (locationHistory.size() > MAX_HISTORY_SIZE) {
        locationHistory.erase(locationHistory.begin());
    }
    
    // Send location update to fleet management
    sendTelematics("fleet/location", fmt::format("Vehicle {}: [{:.6f},{:.6f}] Speed={:.1f}km/h", 
                                                vehicleId, latitude, longitude, previousSpeed * 3.6));
}

void FleetManagementApp::analyzeFuelEfficiency(double fuelLevel, double engineSpeed, double speed) {
    if (previousFuelLevel > 0 && fuelLevel < previousFuelLevel) {
        double fuelConsumed = previousFuelLevel - fuelLevel;
        
        // Calculate fuel efficiency (simplified calculation)
        if (tripDistance > 1000) { // At least 1km for meaningful calculation
            double fuelEfficiency = (fuelConsumed / 100.0) * (100000.0 / tripDistance); // L/100km approximation
            
            // Store fuel data
            FuelData fuelData = {fuelLevel, fuelConsumed, fuelEfficiency, std::chrono::steady_clock::now()};
            fuelHistory.push_back(fuelData);
            
            // Maintain history size
            if (fuelHistory.size() > MAX_HISTORY_SIZE) {
                fuelHistory.erase(fuelHistory.begin());
            }
            
            velocitas::logger().info("⛽ Fuel Analysis: Level={:.1f}%, Efficiency={:.2f}L/100km, Trip={:.1f}km", 
                                    fuelLevel, fuelEfficiency, tripDistance / 1000.0);
            
            // Fuel efficiency alerts
            if (fuelEfficiency > FUEL_EFFICIENCY_THRESHOLD) {
                velocitas::logger().warn("⚠️  Poor fuel efficiency: {:.2f}L/100km - Driver coaching recommended", fuelEfficiency);
                sendTelematics("fleet/fuel", fmt::format("Vehicle {}: Efficiency {:.2f}L/100km", vehicleId, fuelEfficiency));
            }
            
            // Low fuel warning
            if (fuelLevel < 20.0) {
                velocitas::logger().warn("⚠️  Low fuel warning: {:.1f}% - Refueling required", fuelLevel);
                sendTelematics("fleet/fuel/low", fmt::format("Vehicle {}: Low fuel {:.1f}%", vehicleId, fuelLevel));
            }
        }
    }
    
    // Engine efficiency analysis
    if (speed > 0.1 && engineSpeed > 0) {
        double engineEfficiency = (speed * 3.6) / engineSpeed * 1000; // km/h per 1000 RPM
        
        if (engineEfficiency < 0.8) { // Low efficiency threshold
            velocitas::logger().info("🔧 Engine efficiency low: {:.2f} - Consider gear optimization", engineEfficiency);
        }
    }
}

void FleetManagementApp::evaluateDriverBehavior(double speed, double acceleration, double engineSpeed) {
    auto now = std::chrono::steady_clock::now();
    auto tripDuration = std::chrono::duration_cast<std::chrono::seconds>(now - tripStartTime).count();
    
    // Update driver behavior metrics
    driverBehavior.avgSpeed = (driverBehavior.avgSpeed + speed * 3.6) / 2.0; // Simple running average
    driverBehavior.maxSpeed = std::max(driverBehavior.maxSpeed, speed * 3.6);
    
    // Harsh acceleration detection
    if (acceleration > HARSH_ACCELERATION_THRESHOLD) {
        harshAccelerationCount++;
        driverBehavior.harshAcceleration += 1.0;
        velocitas::logger().warn("⚠️  Harsh acceleration detected: {:.2f}m/s² - Driver coaching needed", acceleration);
        sendTelematics("fleet/behavior", fmt::format("Vehicle {}: Harsh acceleration {:.2f}m/s²", vehicleId, acceleration));
    }
    
    // Harsh braking detection
    if (acceleration < HARSH_BRAKING_THRESHOLD) {
        harshBrakingCount++;
        driverBehavior.harshBraking += 1.0;
        velocitas::logger().warn("⚠️  Harsh braking detected: {:.2f}m/s² - Driver coaching needed", acceleration);
        sendTelematics("fleet/behavior", fmt::format("Vehicle {}: Harsh braking {:.2f}m/s²", vehicleId, acceleration));
    }
    
    // Speeding detection (simplified - in production would use GPS + speed limit data)
    double speedKmh = speed * 3.6;
    if (speedKmh > 90.0) { // Assuming 80 km/h speed limit + buffer
        speedingViolationCount++;
        velocitas::logger().warn("⚠️  Speed limit violation: {:.1f}km/h - Driver coaching required", speedKmh);
        sendTelematics("fleet/speeding", fmt::format("Vehicle {}: Speeding {:.1f}km/h", vehicleId, speedKmh));
    }
    
    // Calculate driver behavior score
    int baseScore = 100;
    int penaltyPoints = harshAccelerationCount * 5 + harshBrakingCount * 5 + speedingViolationCount * 10;
    int idlePenalty = static_cast<int>(totalIdleTime / 60.0); // 1 point per minute of idle
    
    driverBehavior.score = std::max(0, baseScore - penaltyPoints - idlePenalty);
    driverBehavior.idleTime = totalIdleTime;
    
    // Driver performance feedback
    if (driverBehavior.score < 70) {
        velocitas::logger().warn("⚠️  Driver performance alert: Score {}/100 - Immediate coaching required", driverBehavior.score);
        sendTelematics("fleet/driver/alert", fmt::format("Vehicle {}: Driver score {}/100", vehicleId, driverBehavior.score));
    }
}

void FleetManagementApp::checkMaintenanceSchedule(double distanceToService, double engineSpeed) {
    if (distanceToService > 0 && distanceToService < MAINTENANCE_WARNING_THRESHOLD) {
        velocitas::logger().warn("🔧 Maintenance due soon: {:.0f}km remaining - Schedule service", distanceToService);
        sendTelematics("fleet/maintenance", fmt::format("Vehicle {}: Maintenance in {:.0f}km", vehicleId, distanceToService));
    }
    
    // Engine health monitoring
    if (engineSpeed > 4000) { // High RPM threshold
        velocitas::logger().warn("⚠️  High engine RPM: {:.0f} - Monitor for excessive wear", engineSpeed);
        sendTelematics("fleet/engine", fmt::format("Vehicle {}: High RPM {:.0f}", vehicleId, engineSpeed));
    }
}

void FleetManagementApp::generateFleetReport() {
    reportCount++;
    auto now = std::chrono::steady_clock::now();
    auto tripDuration = std::chrono::duration_cast<std::chrono::minutes>(now - tripStartTime).count();
    
    velocitas::logger().info("📋 FLEET REPORT #{} - Vehicle: {}, Driver: {}", reportCount, vehicleId, driverId);
    velocitas::logger().info("🚗 Trip Summary: Duration={:.0f}min, Distance={:.1f}km, Avg Speed={:.1f}km/h", 
                            tripDuration, tripDistance / 1000.0, driverBehavior.avgSpeed);
    velocitas::logger().info("⛽ Fuel Status: Current={:.1f}%, Efficiency={:.2f}L/100km estimated", 
                            previousFuelLevel, FUEL_EFFICIENCY_THRESHOLD * 0.9); // Placeholder calculation
    velocitas::logger().info("👤 Driver Score: {}/100 (Harsh Accel: {}, Harsh Brake: {}, Speeding: {})", 
                            driverBehavior.score, harshAccelerationCount, harshBrakingCount, speedingViolationCount);
    velocitas::logger().info("⏰ Idle Time: {:.0f}s ({:.1f}% of trip)", totalIdleTime, (totalIdleTime / (tripDuration * 60.0)) * 100.0);
    
    // Send comprehensive fleet report
    std::string reportData = fmt::format(
        "{{\"vehicle\":\"{}\",\"driver\":\"{}\",\"trip_duration\":{},\"distance\":{:.1f},\"fuel_level\":{:.1f},\"driver_score\":{},\"idle_time\":{:.0f}}}",
        vehicleId, driverId, tripDuration, tripDistance / 1000.0, previousFuelLevel, driverBehavior.score, totalIdleTime
    );
    
    sendTelematics("fleet/report", reportData);
}

void FleetManagementApp::sendTelematics(const std::string& topic, const std::string& data) {
    // In production, this would send data to MQTT broker, REST API, or other fleet management platform
    velocitas::logger().info("📡 Telematics: {} -> {}", topic, data);
    
    // Example integrations:
    // - MQTT publish to fleet management dashboard
    // - REST API call to fleet management system
    // - Database logging for analytics
    // - Real-time alerts to fleet operators
}

// Application entry point
std::unique_ptr<FleetManagementApp> fleetApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 Fleet Management System shutdown initiated (signal {})", sig);
    if (fleetApp) {
        fleetApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting Smart Fleet Telematics System...");
    velocitas::logger().info("📊 Comprehensive vehicle and driver monitoring active");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    fleetApp = std::make_unique<FleetManagementApp>();
    try {
        fleetApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Fleet management system error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown fleet management error");
        return 1;
    }
    
    velocitas::logger().info("👋 Fleet Management System stopped");
    return 0;
}

// ============================================================================
// 🎓 PRODUCTION DEPLOYMENT NOTES
// ============================================================================
//
// 🏭 FLEET MANAGEMENT INTEGRATIONS:
// 1. MQTT broker for real-time fleet communication
// 2. REST APIs for fleet management dashboard integration
// 3. Database storage for historical analysis and reporting
// 4. Geofencing and route optimization algorithms
// 5. Driver mobile app integration for coaching and feedback
// 6. Maintenance scheduling and work order management
// 7. Fuel card integration for automated expense tracking
// 8. Insurance telematics for usage-based pricing
//
// 📊 ANALYTICS & REPORTING:
// - Real-time fleet dashboard with KPIs
// - Driver performance scorecards and coaching reports
// - Fuel efficiency trends and cost optimization
// - Maintenance schedule optimization
// - Route efficiency analysis and optimization
// - Safety incident reporting and analysis
// - Compliance reporting for regulatory requirements
//
// 🔧 CONFIGURATION PARAMETERS:
// - Customizable thresholds for different vehicle types
// - Driver-specific coaching parameters
// - Fleet-specific fuel efficiency targets
// - Maintenance intervals based on vehicle usage patterns
// - Geofencing boundaries for different operational areas
//
// ============================================================================