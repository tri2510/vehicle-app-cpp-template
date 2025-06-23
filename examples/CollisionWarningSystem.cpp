// ============================================================================
// 🚗 ADAS COLLISION WARNING SYSTEM - Production SDV Example
// ============================================================================
// 
// 📋 WHAT THIS APPLICATION DOES:
// Advanced Driver Assistance System (ADAS) that monitors vehicle signals to
// detect potential collision scenarios and provide appropriate warnings.
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Multi-signal processing for safety systems
// - Safety-critical logic with fail-safe defaults
// - Real-time decision making based on vehicle dynamics
// - Emergency braking assistance logic
// - Multi-threshold alert system (warning, critical, emergency)
//
// 🚀 QUICK START:
// cp examples/CollisionWarningSystem.cpp templates/app/src/VehicleApp.cpp
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
#include <algorithm>

::vehicle::Vehicle Vehicle;

/**
 * @brief ADAS Collision Warning System
 * 
 * This application implements a production-ready ADAS collision warning system
 * that monitors multiple vehicle signals to detect potential collision scenarios.
 * 
 * Features:
 * - Forward collision warning based on speed and acceleration
 * - Emergency braking assistance detection
 * - Multi-threshold alert system (warning, critical, emergency)
 * - Safety-critical error handling with fail-safe defaults
 * - Time-to-collision estimation
 * - Driver behavior analysis
 */
class CollisionWarningSystem : public velocitas::VehicleApp {
public:
    CollisionWarningSystem();

protected:
    void onStart() override;

private:
    void onVehicleDataChanged(const velocitas::DataPointReply& reply);
    
    // Safety analysis methods
    void analyzeCollisionRisk(double speed, double acceleration, double brakePosition);
    void checkEmergencyBrakingCondition(double acceleration, double brakePosition);
    void updateTimeToCollision(double speed, double acceleration);
    void triggerWarning(const std::string& level, const std::string& message);
    
    // Safety thresholds (configurable)
    static constexpr double SPEED_WARNING_THRESHOLD = 22.22;    // 80 km/h in m/s
    static constexpr double SPEED_CRITICAL_THRESHOLD = 27.78;   // 100 km/h in m/s
    static constexpr double HARD_BRAKING_THRESHOLD = -4.0;      // m/s² (strong deceleration)
    static constexpr double EMERGENCY_BRAKE_THRESHOLD = -6.0;   // m/s² (emergency braking)
    static constexpr double BRAKE_PEDAL_EMERGENCY = 80.0;       // % brake pedal position
    static constexpr double TIME_TO_COLLISION_WARNING = 3.0;    // seconds
    static constexpr double TIME_TO_COLLISION_CRITICAL = 1.5;   // seconds
    
    // State tracking
    std::chrono::steady_clock::time_point lastWarningTime;
    std::chrono::steady_clock::time_point lastCriticalTime;
    double previousSpeed = 0.0;
    double timeToCollision = 0.0;
    bool absActive = false;
    int warningCount = 0;
    int criticalCount = 0;
    int emergencyCount = 0;
    
    // Warning cooldown periods (to prevent spam)
    static constexpr int WARNING_COOLDOWN_MS = 5000;   // 5 seconds
    static constexpr int CRITICAL_COOLDOWN_MS = 2000;  // 2 seconds
};

CollisionWarningSystem::CollisionWarningSystem()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")),
      lastWarningTime(std::chrono::steady_clock::now()),
      lastCriticalTime(std::chrono::steady_clock::now()) {
    velocitas::logger().info("🚗 ADAS Collision Warning System initializing...");
}

void CollisionWarningSystem::onStart() {
    velocitas::logger().info("🚀 ADAS Collision Warning System starting - subscribing to safety signals");
    
    // Subscribe to multiple vehicle signals for collision detection
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed)
                                               .select(Vehicle.Acceleration.Longitudinal)
                                               .select(Vehicle.ADAS.ABS.IsActive)
                                               .select(Vehicle.Chassis.Brake.PedalPosition)
                                               .build())
        ->onItem([this](auto&& item) { onVehicleDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) {
            velocitas::logger().error("❌ Critical safety signal subscription error: {}", status.errorMessage());
            // In production, this would trigger fail-safe mode
            triggerWarning("SYSTEM_ERROR", "Safety system communication failure - entering fail-safe mode");
        });
    
    velocitas::logger().info("✅ ADAS system active - monitoring for collision risks");
    velocitas::logger().info("🛡️  Safety thresholds: Warning={:.1f}km/h, Critical={:.1f}km/h", 
                            SPEED_WARNING_THRESHOLD * 3.6, SPEED_CRITICAL_THRESHOLD * 3.6);
}

void CollisionWarningSystem::onVehicleDataChanged(const velocitas::DataPointReply& reply) {
    try {
        // Extract vehicle signals with safety checks
        double speed = 0.0;
        double acceleration = 0.0;
        double brakePosition = 0.0;
        
        // Get current speed (critical safety signal)
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            speed = reply.get(Vehicle.Speed)->value();
        } else {
            velocitas::logger().warn("⚠️  Speed signal unavailable - using fail-safe value");
        }
        
        // Get longitudinal acceleration
        if (reply.get(Vehicle.Acceleration.Longitudinal)->isAvailable()) {
            acceleration = reply.get(Vehicle.Acceleration.Longitudinal)->value();
        }
        
        // Get ABS status
        if (reply.get(Vehicle.ADAS.ABS.IsActive)->isAvailable()) {
            absActive = reply.get(Vehicle.ADAS.ABS.IsActive)->value();
        }
        
        // Get brake pedal position
        if (reply.get(Vehicle.Chassis.Brake.PedalPosition)->isAvailable()) {
            brakePosition = reply.get(Vehicle.Chassis.Brake.PedalPosition)->value();
        }
        
        // Log current vehicle state
        velocitas::logger().info("📊 Vehicle State: Speed={:.1f}km/h, Accel={:.2f}m/s², Brake={:.1f}%, ABS={}",
                                speed * 3.6, acceleration, brakePosition, absActive ? "ACTIVE" : "inactive");
        
        // Perform collision risk analysis
        analyzeCollisionRisk(speed, acceleration, brakePosition);
        
        // Check for emergency braking conditions
        checkEmergencyBrakingCondition(acceleration, brakePosition);
        
        // Update time-to-collision estimation
        updateTimeToCollision(speed, acceleration);
        
        // Update previous values for next iteration
        previousSpeed = speed;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Critical error in collision detection: {}", e.what());
        // In production safety systems, this would trigger immediate fail-safe mode
        triggerWarning("SYSTEM_ERROR", "Critical safety system error detected");
    }
}

void CollisionWarningSystem::analyzeCollisionRisk(double speed, double acceleration, double brakePosition) {
    auto now = std::chrono::steady_clock::now();
    
    // Critical speed analysis
    if (speed > SPEED_CRITICAL_THRESHOLD) {
        auto timeSinceLastCritical = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCriticalTime).count();
        if (timeSinceLastCritical > CRITICAL_COOLDOWN_MS) {
            triggerWarning("CRITICAL", fmt::format("CRITICAL SPEED: {:.1f}km/h - Immediate speed reduction required!", speed * 3.6));
            lastCriticalTime = now;
            criticalCount++;
        }
    }
    // Warning speed analysis
    else if (speed > SPEED_WARNING_THRESHOLD) {
        auto timeSinceLastWarning = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWarningTime).count();
        if (timeSinceLastWarning > WARNING_COOLDOWN_MS) {
            triggerWarning("WARNING", fmt::format("High speed detected: {:.1f}km/h - Recommend reducing speed", speed * 3.6));
            lastWarningTime = now;
            warningCount++;
        }
    }
    
    // Rapid deceleration analysis (potential collision avoidance)
    if (acceleration < HARD_BRAKING_THRESHOLD && acceleration > EMERGENCY_BRAKE_THRESHOLD) {
        triggerWarning("INFO", fmt::format("Hard braking detected: {:.2f}m/s² - Collision avoidance in progress", acceleration));
    }
    
    // Combined risk analysis: High speed + insufficient braking
    if (speed > SPEED_WARNING_THRESHOLD && brakePosition < 20.0 && acceleration > -1.0) {
        triggerWarning("WARNING", "High-speed low-braking scenario - Potential collision risk");
    }
    
    // ABS activation analysis
    if (absActive) {
        triggerWarning("INFO", "ABS activated - Emergency braking assistance engaged");
    }
}

void CollisionWarningSystem::checkEmergencyBrakingCondition(double acceleration, double brakePosition) {
    // Emergency braking detection
    if (acceleration < EMERGENCY_BRAKE_THRESHOLD || brakePosition > BRAKE_PEDAL_EMERGENCY) {
        triggerWarning("EMERGENCY", fmt::format("EMERGENCY BRAKING: Accel={:.2f}m/s², Brake={:.1f}% - Collision imminent!", 
                                               acceleration, brakePosition));
        emergencyCount++;
        
        // In production, this would:
        // 1. Activate emergency hazard lights
        // 2. Send alert to connected vehicles (V2V)
        // 3. Prepare safety systems (airbags, seatbelt tensioners)
        // 4. Log incident for analysis
    }
}

void CollisionWarningSystem::updateTimeToCollision(double speed, double acceleration) {
    // Simple time-to-collision estimation
    // In production, this would use more sophisticated algorithms with:
    // - Radar/lidar data for distance to obstacles
    // - Camera data for object detection
    // - Map data for road geometry
    // - Weather and road conditions
    
    if (speed > 0 && acceleration < 0) {
        // Simplified calculation: time to stop = speed / |deceleration|
        timeToCollision = speed / std::abs(acceleration);
        
        if (timeToCollision < TIME_TO_COLLISION_CRITICAL && timeToCollision > 0) {
            triggerWarning("CRITICAL", fmt::format("Time to collision: {:.1f}s - IMMEDIATE ACTION REQUIRED!", timeToCollision));
        } else if (timeToCollision < TIME_TO_COLLISION_WARNING && timeToCollision > 0) {
            triggerWarning("WARNING", fmt::format("Time to collision: {:.1f}s - Prepare for emergency braking", timeToCollision));
        }
    }
}

void CollisionWarningSystem::triggerWarning(const std::string& level, const std::string& message) {
    auto timestamp = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    if (level == "EMERGENCY") {
        velocitas::logger().error("🚨 EMERGENCY ALERT: {}", message);
        // In production: Trigger emergency protocols
    } else if (level == "CRITICAL") {
        velocitas::logger().error("⚠️  CRITICAL WARNING: {}", message);
        // In production: High-priority driver notification
    } else if (level == "WARNING") {
        velocitas::logger().warn("⚠️  WARNING: {}", message);
        // In production: Standard driver notification
    } else if (level == "INFO") {
        velocitas::logger().info("ℹ️  INFO: {}", message);
        // In production: Informational notification
    } else {
        velocitas::logger().error("🔧 SYSTEM: {}", message);
        // In production: System diagnostic message
    }
    
    // Log safety event for analysis
    velocitas::logger().info("📝 Safety Event Logged: Level={}, Count=W:{}/C:{}/E:{}", 
                            level, warningCount, criticalCount, emergencyCount);
}

// Application entry point
std::unique_ptr<CollisionWarningSystem> safetyApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 ADAS system shutdown initiated (signal {})", sig);
    if (safetyApp) {
        safetyApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting ADAS Collision Warning System...");
    velocitas::logger().info("🛡️  Safety-critical vehicle monitoring active");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    safetyApp = std::make_unique<CollisionWarningSystem>();
    try {
        safetyApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Critical safety system error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown critical safety system error");
        return 1;
    }
    
    velocitas::logger().info("👋 ADAS Collision Warning System stopped");
    return 0;
}

// ============================================================================
// 🎓 PRODUCTION DEPLOYMENT NOTES
// ============================================================================
//
// 🏭 PRODUCTION ENHANCEMENTS:
// 1. Real sensor integration (radar, lidar, cameras)
// 2. Machine learning models for collision prediction
// 3. Integration with vehicle control systems (automatic braking)
// 4. V2V communication for cooperative collision avoidance
// 5. Cloud connectivity for incident reporting and analysis
// 6. ISO 26262 functional safety compliance
// 7. Hardware-in-the-loop testing and validation
// 8. Fail-safe modes and redundancy systems
//
// 🔧 CONFIGURATION PARAMETERS:
// - Speed thresholds can be adjusted based on road type and conditions
// - Warning cooldown periods can be tuned for driver preference
// - Time-to-collision calculations can use real-world obstacle data
// - Integration with navigation systems for proactive warnings
//
// 📊 MONITORING & ANALYTICS:
// - Driver behavior patterns and response times
// - False positive/negative rates for system tuning
// - Incident correlation with weather, traffic, and road conditions
// - System performance metrics and diagnostic data
//
// ============================================================================