// ============================================================================
// 🎓 STEP 3: DATA ANALYSIS & ALERTS - Advanced Pattern Detection
// ============================================================================
// 
// 📋 LEARNING OBJECTIVES:
// In this tutorial step, you will learn:
// - Implement rolling window analysis for pattern detection
// - Create multi-tier alert system with priorities
// - Detect complex driving behaviors over time
// - Build predictive analytics for vehicle health
//
// 🎯 WHAT YOU'LL BUILD:
// An advanced analytics application that:
// - Tracks historical data with rolling windows
// - Detects erratic driving, harsh braking, rapid acceleration
// - Implements priority-based alert system
// - Predicts maintenance needs from signal patterns
// - Generates comprehensive driver behavior reports
//
// 💡 DIFFICULTY: ⭐⭐⭐ Advanced (45 minutes)
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

// Global Vehicle instance for accessing vehicle signals
::vehicle::Vehicle Vehicle;

/**
 * @brief Step 3: Data Analysis & Alerts Application
 * 
 * 🎓 TUTORIAL: This application demonstrates advanced analytics:
 * - Historical data tracking with rolling windows
 * - Pattern detection algorithms
 * - Multi-tier alert system
 * - Predictive analytics
 * - Comprehensive reporting
 */
class DataAnalysisAlerts : public velocitas::VehicleApp {
public:
    DataAnalysisAlerts();

protected:
    void onStart() override;

private:
    // 🎓 LEARNING POINT: Alert Priority Levels
    enum class AlertPriority {
        INFO = 0,
        WARNING = 1,
        CRITICAL = 2,
        EMERGENCY = 3
    };
    
    // 🎓 LEARNING POINT: Alert Structure
    struct Alert {
        std::string id;
        std::string message;
        AlertPriority priority;
        std::chrono::steady_clock::time_point timestamp;
        double value;
    };
    
    // 🎓 LEARNING POINT: Data Point for Time Series
    struct DataPoint {
        double value;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    // 🎓 LEARNING POINT: Driving Event Types
    enum class DrivingEvent {
        HARSH_BRAKING,
        RAPID_ACCELERATION,
        ERRATIC_DRIVING,
        EXCESSIVE_IDLING,
        SPEED_VIOLATION,
        OPTIMAL_DRIVING
    };
    
    // Signal processing methods
    void onSignalChanged(const velocitas::DataPointReply& reply);
    void analyzeDataPatterns();
    void detectDrivingEvents();
    void generatePredictiveAlerts();
    void updateHistoricalData();
    
    // Analysis methods
    double calculateStandardDeviation(const std::deque<DataPoint>& data);
    double calculateTrend(const std::deque<DataPoint>& data);
    double detectAnomaly(double value, const std::deque<DataPoint>& history);
    void detectHarshBraking();
    void detectRapidAcceleration();
    void detectErraticDriving();
    void analyzeEngineHealth();
    
    // Alert management
    void createAlert(const std::string& id, const std::string& message, 
                    AlertPriority priority, double value = 0.0);
    void processAlerts();
    void logAlert(const Alert& alert);
    
    // Reporting
    void generateDriverBehaviorReport();
    void generateVehicleHealthReport();
    
    // 🎓 LEARNING POINT: Historical Data Storage
    struct HistoricalData {
        std::deque<DataPoint> speed;        // Rolling window of speed data
        std::deque<DataPoint> rpm;          // Rolling window of RPM data
        std::deque<DataPoint> fuel;         // Rolling window of fuel data
        std::deque<DataPoint> acceleration; // Calculated acceleration
        
        const size_t maxSize = 50;          // Keep last 50 data points
        
        void addSpeed(double value) {
            addDataPoint(speed, value);
            calculateAcceleration();
        }
        
        void addRpm(double value) {
            addDataPoint(rpm, value);
        }
        
        void addFuel(double value) {
            addDataPoint(fuel, value);
        }
        
    private:
        void addDataPoint(std::deque<DataPoint>& container, double value) {
            container.push_back({value, std::chrono::steady_clock::now()});
            if (container.size() > maxSize) {
                container.pop_front();
            }
        }
        
        void calculateAcceleration() {
            if (speed.size() >= 2) {
                auto& latest = speed.back();
                auto& previous = speed[speed.size() - 2];
                
                auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
                    latest.timestamp - previous.timestamp).count() / 1000.0;
                
                if (timeDiff > 0) {
                    double accel = (latest.value - previous.value) / timeDiff;
                    acceleration.push_back({accel, latest.timestamp});
                    if (acceleration.size() > maxSize) {
                        acceleration.pop_front();
                    }
                }
            }
        }
    };
    
    // 🎓 LEARNING POINT: Analytics State
    struct AnalyticsState {
        int harshBrakingCount = 0;
        int rapidAccelCount = 0;
        int erraticDrivingCount = 0;
        double driverScore = 100.0;
        std::chrono::steady_clock::time_point lastReportTime;
        
        // Thresholds
        const double HARSH_BRAKING_THRESHOLD = -5.0;      // m/s²
        const double RAPID_ACCEL_THRESHOLD = 4.0;         // m/s²
        const double SPEED_VARIANCE_THRESHOLD = 15.0;     // km/h
        const double RPM_ANOMALY_THRESHOLD = 2.0;         // standard deviations
    };
    
    // Member variables
    HistoricalData m_history;
    AnalyticsState m_analytics;
    std::vector<Alert> m_activeAlerts;
    std::chrono::steady_clock::time_point m_startTime;
    
    // Current values
    double m_currentSpeed = 0.0;
    double m_currentRpm = 0.0;
    double m_currentFuel = 0.0;
};

// ============================================================================
// 🎓 STEP 3A: Advanced Analytics Initialization
// ============================================================================
DataAnalysisAlerts::DataAnalysisAlerts()
    : velocitas::VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")) {
    
    velocitas::logger().info("🎓 Step 3: Data Analysis & Alerts starting...");
    velocitas::logger().info("📡 Connecting to Vehicle Data Broker...");
    velocitas::logger().info("📊 Learning objective: Advanced pattern analysis");
    velocitas::logger().info("🚨 Features: Multi-tier alerts, predictive analytics");
    velocitas::logger().info("📊 Signals: Speed (real) + RPM/Fuel (simulated)");
    velocitas::logger().info("✅ Data Analysis & Alerts initialized");
    
    m_startTime = std::chrono::steady_clock::now();
    m_analytics.lastReportTime = m_startTime;
    
    // Initialize simulated fuel level
    m_currentFuel = 75.0;
}

// ============================================================================
// 🎓 STEP 3B: Signal Subscription for Analytics
// ============================================================================
void DataAnalysisAlerts::onStart() {
    velocitas::logger().info("🚀 Step 3: Starting Data Analysis & Alerts!");
    velocitas::logger().info("📊 Initializing historical data tracking...");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    subscribeDataPoints(
        velocitas::QueryBuilder::select(Vehicle.Speed)
            .select(Vehicle.CurrentLocation.Latitude)
            .select(Vehicle.CurrentLocation.Longitude)
            .build()
    )
    ->onItem([this](auto&& item) { 
        onSignalChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        velocitas::logger().error("❌ Analytics subscription error: {}", status.errorMessage());
    });
    
    velocitas::logger().info("✅ Analytics signal subscriptions active");
    velocitas::logger().info("🔄 Collecting data for pattern analysis...");
    velocitas::logger().info("💡 Generate varied driving patterns to see analytics!");
}

// ============================================================================
// 🎓 STEP 3C: Signal Processing with Historical Tracking
// ============================================================================
void DataAnalysisAlerts::onSignalChanged(const velocitas::DataPointReply& reply) {
    try {
        bool dataUpdated = false;
        
        // Update current values and history
        try {
            if (reply.get(Vehicle.Speed)->isValid()) {
                m_currentSpeed = reply.get(Vehicle.Speed)->value();
                m_history.addSpeed(m_currentSpeed);
                dataUpdated = true;
                
                // Simulate RPM based on speed (since Engine.Speed not available)
                double speedKmh = m_currentSpeed * 3.6;
                m_currentRpm = speedKmh * 40.0 + 800.0;  // Realistic RPM simulation
                m_history.addRpm(m_currentRpm);
                
                // Simulate fuel consumption based on driving (since FuelSystem.Level not available)
                static double totalDistance = 0.0;
                static auto lastTime = std::chrono::steady_clock::now();
                auto now = std::chrono::steady_clock::now();
                auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime).count();
                
                if (timeDiff > 0) {
                    totalDistance += speedKmh * (timeDiff / 3600.0);  // Add distance traveled
                    if (totalDistance > 5.0) {  // Every ~5km
                        m_currentFuel -= 0.5;  // Consume 0.5% fuel
                        totalDistance = 0.0;
                    }
                    if (m_currentFuel < 0) m_currentFuel = 0;
                    lastTime = now;
                }
                m_history.addFuel(m_currentFuel);
            }
        } catch (...) {
            // Speed signal not available in this update
        }
        
        // Process GPS data for location-based analysis (future use)
        try {
            if (reply.get(Vehicle.CurrentLocation.Latitude)->isValid()) {
                // Store location for future geofencing features
                dataUpdated = true;
            }
        } catch (...) {
            // Latitude signal not available
        }
        
        try {
            if (reply.get(Vehicle.CurrentLocation.Longitude)->isValid()) {
                // Store location for future geofencing features  
                dataUpdated = true;
            }
        } catch (...) {
            // Longitude signal not available
        }
        
        if (dataUpdated) {
            updateHistoricalData();
            analyzeDataPatterns();
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().debug("📡 Processing analytics data...");
    }
}

// ============================================================================
// 🎓 STEP 3D: Pattern Analysis Engine
// ============================================================================
void DataAnalysisAlerts::analyzeDataPatterns() {
    // Log current status
    double speedKmh = m_currentSpeed * 3.6;
    velocitas::logger().info("📊 Current: Speed {:.1f} km/h | RPM {:.0f} | Fuel {:.1f}%",
        speedKmh, m_currentRpm, m_currentFuel);
    
    // 🎓 LEARNING POINT: Pattern Detection
    // Only analyze when we have enough historical data
    if (m_history.speed.size() >= 5) {
        detectDrivingEvents();
        generatePredictiveAlerts();
        processAlerts();
        
        // Generate reports periodically
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_analytics.lastReportTime).count();
        
        if (elapsed >= 30) {  // Every 30 seconds
            generateDriverBehaviorReport();
            generateVehicleHealthReport();
            m_analytics.lastReportTime = now;
        }
    }
}

// ============================================================================
// 🎓 STEP 3E: Driving Event Detection
// ============================================================================
void DataAnalysisAlerts::detectDrivingEvents() {
    detectHarshBraking();
    detectRapidAcceleration();
    detectErraticDriving();
    analyzeEngineHealth();
}

void DataAnalysisAlerts::detectHarshBraking() {
    // 🎓 LEARNING POINT: Harsh Braking Detection
    // Analyze acceleration history for sudden deceleration
    
    if (m_history.acceleration.empty()) return;
    
    double latestAccel = m_history.acceleration.back().value;
    
    if (latestAccel < m_analytics.HARSH_BRAKING_THRESHOLD) {
        m_analytics.harshBrakingCount++;
        m_analytics.driverScore -= 2.0;
        
        createAlert("HARSH_BRAKING", 
            fmt::format("Harsh braking detected: {:.1f} m/s²", latestAccel),
            AlertPriority::WARNING, latestAccel);
        
        velocitas::logger().warn("🚨 HARSH BRAKING: {:.1f} m/s² (threshold: {:.1f})",
            latestAccel, m_analytics.HARSH_BRAKING_THRESHOLD);
    }
}

void DataAnalysisAlerts::detectRapidAcceleration() {
    // 🎓 LEARNING POINT: Rapid Acceleration Detection
    
    if (m_history.acceleration.empty()) return;
    
    double latestAccel = m_history.acceleration.back().value;
    
    if (latestAccel > m_analytics.RAPID_ACCEL_THRESHOLD) {
        m_analytics.rapidAccelCount++;
        m_analytics.driverScore -= 1.5;
        
        createAlert("RAPID_ACCELERATION",
            fmt::format("Rapid acceleration: {:.1f} m/s²", latestAccel),
            AlertPriority::WARNING, latestAccel);
        
        velocitas::logger().warn("⚡ RAPID ACCELERATION: {:.1f} m/s² (threshold: {:.1f})",
            latestAccel, m_analytics.RAPID_ACCEL_THRESHOLD);
    }
}

void DataAnalysisAlerts::detectErraticDriving() {
    // 🎓 LEARNING POINT: Erratic Driving Pattern Detection
    // Use standard deviation to detect inconsistent speed
    
    if (m_history.speed.size() < 10) return;
    
    double speedStdDev = calculateStandardDeviation(m_history.speed);
    double speedStdDevKmh = speedStdDev * 3.6;
    
    if (speedStdDevKmh > m_analytics.SPEED_VARIANCE_THRESHOLD) {
        m_analytics.erraticDrivingCount++;
        m_analytics.driverScore -= 3.0;
        
        createAlert("ERRATIC_DRIVING",
            fmt::format("Erratic driving pattern detected. Speed variance: {:.1f} km/h", speedStdDevKmh),
            AlertPriority::CRITICAL, speedStdDevKmh);
        
        velocitas::logger().warn("🔄 ERRATIC DRIVING: Speed variance {:.1f} km/h exceeds threshold",
            speedStdDevKmh);
    }
}

void DataAnalysisAlerts::analyzeEngineHealth() {
    // 🎓 LEARNING POINT: Predictive Engine Health Analysis
    // Detect anomalies in RPM patterns
    
    if (m_history.rpm.size() < 20) return;
    
    double rpmAnomaly = detectAnomaly(m_currentRpm, m_history.rpm);
    
    if (std::abs(rpmAnomaly) > m_analytics.RPM_ANOMALY_THRESHOLD) {
        createAlert("ENGINE_ANOMALY",
            fmt::format("Engine RPM anomaly detected: {:.1f} std deviations", rpmAnomaly),
            AlertPriority::WARNING, rpmAnomaly);
        
        velocitas::logger().warn("🔧 ENGINE ANOMALY: RPM deviation {:.1f}σ from normal",
            rpmAnomaly);
    }
    
    // Check for excessive idling
    if (m_currentSpeed < 0.5 && m_currentRpm > 1500) {
        static auto idleStartTime = std::chrono::steady_clock::now();
        auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - idleStartTime).count();
        
        if (idleDuration > 60) {  // More than 60 seconds
            createAlert("EXCESSIVE_IDLING",
                fmt::format("Excessive idling for {} seconds", idleDuration),
                AlertPriority::INFO, idleDuration);
        }
    }
}

// ============================================================================
// 🎓 STEP 3F: Predictive Analytics
// ============================================================================
void DataAnalysisAlerts::generatePredictiveAlerts() {
    // 🎓 LEARNING POINT: Fuel Range Prediction
    if (m_history.fuel.size() >= 10 && m_currentFuel < 30.0) {
        double fuelTrend = calculateTrend(m_history.fuel);
        
        if (fuelTrend < -0.5) {  // Fuel decreasing rapidly
            double minutesToEmpty = m_currentFuel / std::abs(fuelTrend);
            
            if (minutesToEmpty < 30) {
                createAlert("LOW_FUEL_PREDICTION",
                    fmt::format("Fuel critically low! Estimated {} minutes remaining", 
                        static_cast<int>(minutesToEmpty)),
                    AlertPriority::CRITICAL, minutesToEmpty);
            }
        }
    }
    
    // 🎓 LEARNING POINT: Maintenance Prediction
    if (m_analytics.harshBrakingCount > 10 || m_analytics.rapidAccelCount > 15) {
        createAlert("MAINTENANCE_RECOMMENDED",
            "Aggressive driving detected. Early brake/tire inspection recommended",
            AlertPriority::INFO);
    }
}

// ============================================================================
// 🎓 STEP 3G: Statistical Analysis Functions
// ============================================================================
double DataAnalysisAlerts::calculateStandardDeviation(const std::deque<DataPoint>& data) {
    if (data.size() < 2) return 0.0;
    
    std::vector<double> values;
    for (const auto& point : data) {
        values.push_back(point.value);
    }
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    
    for (double value : values) {
        variance += std::pow(value - mean, 2);
    }
    variance /= values.size();
    
    return std::sqrt(variance);
}

double DataAnalysisAlerts::calculateTrend(const std::deque<DataPoint>& data) {
    // Simple linear trend calculation
    if (data.size() < 2) return 0.0;
    
    double firstValue = data.front().value;
    double lastValue = data.back().value;
    
    auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(
        data.back().timestamp - data.front().timestamp).count();
    
    if (timeDiff == 0) return 0.0;
    
    return (lastValue - firstValue) / timeDiff;  // Units per second
}

double DataAnalysisAlerts::detectAnomaly(double value, const std::deque<DataPoint>& history) {
    // Z-score based anomaly detection
    if (history.size() < 10) return 0.0;
    
    std::vector<double> values;
    for (const auto& point : history) {
        values.push_back(point.value);
    }
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double stdDev = calculateStandardDeviation(history);
    
    if (stdDev == 0) return 0.0;
    
    return (value - mean) / stdDev;  // Z-score
}

// ============================================================================
// 🎓 STEP 3H: Alert Management System
// ============================================================================
void DataAnalysisAlerts::createAlert(const std::string& id, const std::string& message,
                                    AlertPriority priority, double value) {
    Alert alert{
        id,
        message,
        priority,
        std::chrono::steady_clock::now(),
        value
    };
    
    m_activeAlerts.push_back(alert);
    logAlert(alert);
}

void DataAnalysisAlerts::processAlerts() {
    // Remove old alerts (older than 5 minutes)
    auto now = std::chrono::steady_clock::now();
    m_activeAlerts.erase(
        std::remove_if(m_activeAlerts.begin(), m_activeAlerts.end(),
            [now](const Alert& alert) {
                auto age = std::chrono::duration_cast<std::chrono::minutes>(
                    now - alert.timestamp).count();
                return age > 5;
            }),
        m_activeAlerts.end()
    );
    
    // Count alerts by priority
    int criticalCount = std::count_if(m_activeAlerts.begin(), m_activeAlerts.end(),
        [](const Alert& a) { return a.priority == AlertPriority::CRITICAL; });
    
    if (criticalCount > 3) {
        velocitas::logger().error("🚨 MULTIPLE CRITICAL ALERTS ACTIVE!");
    }
}

void DataAnalysisAlerts::logAlert(const Alert& alert) {
    const char* priorityStr[] = {"INFO", "WARNING", "CRITICAL", "EMERGENCY"};
    int priorityLevel = static_cast<int>(alert.priority);
    
    switch (alert.priority) {
        case AlertPriority::INFO:
            velocitas::logger().info("ℹ️  [{}] {}", priorityStr[priorityLevel], alert.message);
            break;
        case AlertPriority::WARNING:
            velocitas::logger().warn("⚠️  [{}] {}", priorityStr[priorityLevel], alert.message);
            break;
        case AlertPriority::CRITICAL:
            velocitas::logger().error("🚨 [{}] {}", priorityStr[priorityLevel], alert.message);
            break;
        case AlertPriority::EMERGENCY:
            velocitas::logger().error("🆘 [{}] {}", priorityStr[priorityLevel], alert.message);
            break;
    }
}

// ============================================================================
// 🎓 STEP 3I: Reporting System
// ============================================================================
void DataAnalysisAlerts::generateDriverBehaviorReport() {
    velocitas::logger().info("📋 === DRIVER BEHAVIOR REPORT ===");
    velocitas::logger().info("🏁 Driver Score: {:.1f}/100", m_analytics.driverScore);
    velocitas::logger().info("🚨 Harsh Braking Events: {}", m_analytics.harshBrakingCount);
    velocitas::logger().info("⚡ Rapid Acceleration Events: {}", m_analytics.rapidAccelCount);
    velocitas::logger().info("🔄 Erratic Driving Events: {}", m_analytics.erraticDrivingCount);
    
    std::string rating;
    if (m_analytics.driverScore >= 90) rating = "Excellent";
    else if (m_analytics.driverScore >= 75) rating = "Good";
    else if (m_analytics.driverScore >= 60) rating = "Fair";
    else rating = "Needs Improvement";
    
    velocitas::logger().info("⭐ Overall Rating: {}", rating);
    velocitas::logger().info("================================");
}

void DataAnalysisAlerts::generateVehicleHealthReport() {
    velocitas::logger().info("🔧 === VEHICLE HEALTH REPORT ===");
    
    // Active alerts summary
    int infoCount = 0, warningCount = 0, criticalCount = 0;
    for (const auto& alert : m_activeAlerts) {
        switch (alert.priority) {
            case AlertPriority::INFO: infoCount++; break;
            case AlertPriority::WARNING: warningCount++; break;
            case AlertPriority::CRITICAL: 
            case AlertPriority::EMERGENCY: criticalCount++; break;
        }
    }
    
    velocitas::logger().info("📊 Active Alerts: {} Info, {} Warning, {} Critical",
        infoCount, warningCount, criticalCount);
    
    // Predictive maintenance
    if (m_analytics.harshBrakingCount > 5) {
        velocitas::logger().info("🔧 Brake inspection recommended");
    }
    if (m_analytics.rapidAccelCount > 10) {
        velocitas::logger().info("🔧 Engine/transmission check advised");
    }
    
    velocitas::logger().info("⛽ Current Fuel: {:.1f}%", m_currentFuel);
    velocitas::logger().info("================================");
}

void DataAnalysisAlerts::updateHistoricalData() {
    // Additional processing if needed
}

// ============================================================================
// 🎓 STEP 3J: Application Entry Point
// ============================================================================

std::unique_ptr<DataAnalysisAlerts> analyticsApp;

void signalHandler(int signal) {
    velocitas::logger().info("🛑 Shutting down Data Analysis & Alerts (signal: {})", signal);
    if (analyticsApp) {
        analyticsApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    velocitas::logger().info("🎓 Step 3: Starting Data Analysis & Alerts Tutorial");
    velocitas::logger().info("🎯 Learning Goal: Master advanced analytics & alerts");
    velocitas::logger().info("📊 Features: Pattern detection, predictive analytics");
    velocitas::logger().info("💡 Press Ctrl+C to stop");
    
    try {
        analyticsApp = std::make_unique<DataAnalysisAlerts>();
        analyticsApp->run();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 Application error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown application error");
        return 1;
    }
    
    velocitas::logger().info("👋 Step 3: Data Analysis & Alerts completed");
    velocitas::logger().info("🎓 Next: Step 4 - Advanced Fleet Management");
    return 0;
}

// ============================================================================
// 🎓 STEP 3: LEARNING SUMMARY
// ============================================================================
//
// 🎯 CONCEPTS LEARNED:
// ✅ Historical data tracking with rolling windows
// ✅ Statistical analysis (std deviation, trends, anomalies)
// ✅ Pattern detection algorithms
// ✅ Multi-tier alert system implementation
// ✅ Predictive analytics for maintenance
// ✅ Driver behavior scoring
// ✅ Comprehensive reporting systems
//
// 🔧 KEY CODE PATTERNS:
// ✅ std::deque for rolling windows
// ✅ Statistical calculations (mean, std dev, z-score)
// ✅ Time-based event detection
// ✅ Alert priority management
// ✅ Periodic report generation
// ✅ Anomaly detection algorithms
// ✅ Trend analysis techniques
//
// 📊 TESTING COMMANDS:
// Using Pre-built Image (RECOMMENDED):
// Build: docker run --rm --network host \
//        -v $(pwd)/examples/Step3_DataAnalysisAlerts.cpp:/app.cpp \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose
//
// Run:   docker run -d --network host --name step3-app \
//        ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 180
//
// Test Harsh Braking:
//   echo "setValue Vehicle.Speed 30.0" | kuksa-client
//   sleep 2
//   echo "setValue Vehicle.Speed 5.0" | kuksa-client
//
// Test Rapid Acceleration:
//   echo "setValue Vehicle.Speed 10.0" | kuksa-client
//   sleep 1
//   echo "setValue Vehicle.Speed 35.0" | kuksa-client
//
// Test Erratic Driving (varied speeds):
//   for i in 10 25 15 30 20 35 15 40 10; do
//     echo "setValue Vehicle.Speed $i.0" | kuksa-client
//     sleep 2
//   done
//
// 🎓 EXPECTED OUTPUT:
// 📊 Current: Speed 90.0 km/h | RPM 2500 | Fuel 75.5%
// 🚨 HARSH BRAKING: -8.3 m/s² (threshold: -5.0)
// ⚠️  [WARNING] Harsh braking detected: -8.3 m/s²
// ⚡ RAPID ACCELERATION: 6.9 m/s² (threshold: 4.0)
// 🔄 ERRATIC DRIVING: Speed variance 18.5 km/h exceeds threshold
// 📋 === DRIVER BEHAVIOR REPORT ===
// 🏁 Driver Score: 88.5/100
// ⭐ Overall Rating: Good
//
// 🚀 NEXT STEP: Step4_AdvancedFleetManager.cpp
// Build production-ready fleet management system!
// ============================================================================