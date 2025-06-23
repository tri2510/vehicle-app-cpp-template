// ============================================================================
// 🚗 V2X COMMUNICATION HUB - Production SDV Example
// ============================================================================
// 
// 📋 WHAT THIS APPLICATION DOES:
// Advanced Vehicle-to-Everything (V2X) communication system that enables
// cooperative driving, traffic optimization, and safety coordination.
//
// 🎯 SDV CONCEPTS DEMONSTRATED:
// - Vehicle-to-vehicle (V2V) hazard warnings and coordination
// - Vehicle-to-infrastructure (V2I) traffic signal optimization
// - Cooperative adaptive cruise control support
// - Emergency vehicle priority handling
// - Real-time traffic condition sharing
// - Connected autonomous driving coordination
//
// 🚀 QUICK START:
// cp examples/V2XCommunicator.cpp templates/app/src/VehicleApp.cpp
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
#include <map>
#include <queue>
#include <string>
#include <random>

::vehicle::Vehicle Vehicle;

/**
 * @brief V2X Communication Hub
 * 
 * Production-ready Vehicle-to-Everything communication system that provides:
 * - Vehicle-to-vehicle hazard warnings and cooperative driving
 * - Vehicle-to-infrastructure traffic signal and route optimization
 * - Cooperative adaptive cruise control with surrounding vehicles
 * - Emergency vehicle priority coordination and routing
 * - Real-time traffic condition sharing and crowd-sourced intelligence
 * - Connected autonomous driving coordination and safety
 */
class V2XCommunicator : public velocitas::VehicleApp {
public:
    V2XCommunicator();

protected:
    void onStart() override;

private:
    void onV2XDataChanged(const velocitas::DataPointReply& reply);
    
    // V2X communication methods
    void processV2VMessages();
    void sendV2VHazardWarning(const std::string& hazardType, double latitude, double longitude);
    void handleTrafficSignalOptimization();
    void coordinateCooperativeCruiseControl(double speed);
    void handleEmergencyVehiclePriority();
    void shareTrafficConditions(double speed, double latitude, double longitude);
    void processConnectedVehicleData();
    void generateV2XReport();
    void sendV2XMessage(const std::string& messageType, const std::string& data);
    
    // V2X message structures
    struct V2VMessage {
        std::string vehicleId;
        std::string messageType;  // HAZARD, EMERGENCY, COOP_CRUISE, TRAFFIC
        double latitude;
        double longitude;
        double speed;
        std::string payload;
        std::chrono::steady_clock::time_point timestamp;
        double distance;          // Distance from our vehicle
        int priority;            // Message priority (1-10)
    };
    
    struct TrafficSignal {
        std::string intersectionId;
        std::string currentPhase;   // RED, YELLOW, GREEN
        int timeRemaining;          // Seconds
        double latitude;
        double longitude;
        double distance;
        std::string optimizationAdvice; // SLOW_DOWN, MAINTAIN, SPEED_UP
    };
    
    struct EmergencyVehicle {
        std::string vehicleId;
        std::string vehicleType;    // AMBULANCE, FIRE, POLICE
        double latitude;
        double longitude;
        double heading;
        double speed;
        std::string route;
        int priority;
        std::chrono::steady_clock::time_point lastUpdate;
    };
    
    struct ConnectedVehicle {
        std::string vehicleId;
        double latitude;
        double longitude;
        double speed;
        double heading;
        std::string intent;         // LANE_CHANGE, MERGE, TURN, PARK
        std::chrono::steady_clock::time_point lastSeen;
    };
    
    // V2X data storage
    std::queue<V2VMessage> incomingMessages;
    std::vector<TrafficSignal> nearbySignals;
    std::vector<EmergencyVehicle> emergencyVehicles;
    std::map<std::string, ConnectedVehicle> connectedVehicles;
    
    // Vehicle state for V2X
    std::string ownVehicleId;
    double currentLatitude = 0.0;
    double currentLongitude = 0.0;
    double currentSpeed = 0.0;
    double currentHeading = 0.0;
    bool tisEnabled = false;
    
    // V2X communication parameters
    static constexpr double V2X_RANGE_METERS = 300.0;        // V2X communication range
    static constexpr double HAZARD_ALERT_RANGE = 200.0;      // Hazard alert range
    static constexpr double EMERGENCY_PRIORITY_RANGE = 500.0; // Emergency vehicle range
    static constexpr double COOP_CRUISE_RANGE = 100.0;       // Cooperative cruise control range
    static constexpr int MAX_CONNECTED_VEHICLES = 50;        // Maximum tracked vehicles
    static constexpr int MESSAGE_EXPIRY_SECONDS = 30;        // Message validity period
    
    // V2X performance metrics
    int v2vMessagesReceived = 0;
    int v2vMessagesSent = 0;
    int hazardWarningsIssued = 0;
    int trafficOptimizations = 0;
    int emergencyPriorityEvents = 0;
    int cooperativeCruiseEvents = 0;
    
    // V2X system state
    bool v2xSystemEnabled = true;
    bool emergencyMode = false;
    std::chrono::steady_clock::time_point lastMessageTime;
    std::chrono::steady_clock::time_point systemStartTime;
    std::chrono::steady_clock::time_point lastReportTime;
    
    // Configuration
    bool hazardDetectionEnabled = true;
    bool trafficOptimizationEnabled = true;
    bool cooperativeCruiseEnabled = true;
    bool emergencyPriorityEnabled = true;
    
    static constexpr int REPORT_INTERVAL_MINUTES = 5;
    
    // Utility methods
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    void cleanupExpiredData();
    std::string generateVehicleId();
};

V2XCommunicator::V2XCommunicator()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker")),
      lastMessageTime(std::chrono::steady_clock::now()),
      systemStartTime(std::chrono::steady_clock::now()),
      lastReportTime(std::chrono::steady_clock::now()) {
    
    // Generate unique vehicle ID for V2X communication
    ownVehicleId = generateVehicleId();
    
    velocitas::logger().info("📡 V2X Communication Hub initializing - Vehicle ID: {}", ownVehicleId);
}

void V2XCommunicator::onStart() {
    velocitas::logger().info("🚀 V2X Communication starting - Connected vehicle coordination");
    
    // Subscribe to V2X relevant vehicle signals
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.CurrentLocation.Latitude)
                                               .select(Vehicle.CurrentLocation.Longitude)
                                               .select(Vehicle.Speed)
                                               .select(Vehicle.ADAS.TIS.IsEnabled)
                                               .build())
        ->onItem([this](auto&& item) { onV2XDataChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) {
            velocitas::logger().error("❌ V2X communication subscription error: {}", status.errorMessage());
            velocitas::logger().warn("🔧 V2X system operating in degraded mode");
            v2xSystemEnabled = false;
        });
    
    // Simulate some nearby infrastructure and vehicles for demonstration
    // In production, these would come from actual V2X receivers
    TrafficSignal signal1 = {"INT_001", "GREEN", 25, 40.7589, -73.9851, 150.0, "MAINTAIN"};
    TrafficSignal signal2 = {"INT_002", "RED", 45, 40.7614, -73.9776, 280.0, "SLOW_DOWN"};
    nearbySignals.push_back(signal1);
    nearbySignals.push_back(signal2);
    
    velocitas::logger().info("✅ V2X system active - Communication range: {:.0f}m", V2X_RANGE_METERS);
    velocitas::logger().info("🚦 Traffic Information System (TIS): {}", tisEnabled ? "ENABLED" : "DISABLED");
}

void V2XCommunicator::onV2XDataChanged(const velocitas::DataPointReply& reply) {
    try {
        auto now = std::chrono::steady_clock::now();
        
        // Extract V2X relevant signals
        double latitude = currentLatitude;
        double longitude = currentLongitude;
        double speed = currentSpeed;
        
        // Location updates
        if (reply.get(Vehicle.CurrentLocation.Latitude)->isAvailable() && 
            reply.get(Vehicle.CurrentLocation.Longitude)->isAvailable()) {
            latitude = reply.get(Vehicle.CurrentLocation.Latitude)->value();
            longitude = reply.get(Vehicle.CurrentLocation.Longitude)->value();
            currentLatitude = latitude;
            currentLongitude = longitude;
        }
        
        // Speed monitoring
        if (reply.get(Vehicle.Speed)->isAvailable()) {
            speed = reply.get(Vehicle.Speed)->value();
            currentSpeed = speed;
        }
        
        // Traffic Information System status
        if (reply.get(Vehicle.ADAS.TIS.IsEnabled)->isAvailable()) {
            tisEnabled = reply.get(Vehicle.ADAS.TIS.IsEnabled)->value();
        }
        
        // Log V2X status
        velocitas::logger().info("📡 V2X Status: Pos=[{:.6f},{:.6f}], Speed={:.1f}km/h, TIS={}, Range={:.0f}m", 
                                latitude, longitude, speed * 3.6, tisEnabled ? "ON" : "OFF", V2X_RANGE_METERS);
        
        // Execute V2X communication functions
        if (v2xSystemEnabled) {
            processV2VMessages();
            handleTrafficSignalOptimization();
            coordinateCooperativeCruiseControl(speed);
            handleEmergencyVehiclePriority();
            shareTrafficConditions(speed, latitude, longitude);
            processConnectedVehicleData();
            
            // Clean up expired data
            cleanupExpiredData();
        }
        
        // Generate periodic V2X reports
        auto timeSinceLastReport = std::chrono::duration_cast<std::chrono::minutes>(now - lastReportTime).count();
        if (timeSinceLastReport >= REPORT_INTERVAL_MINUTES) {
            generateV2XReport();
            lastReportTime = now;
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 V2X communication error: {}", e.what());
    }
}

void V2XCommunicator::processV2VMessages() {
    // Simulate receiving V2V messages (in production, would come from V2X radio)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> msgChance(1, 100);
    
    // Occasionally simulate receiving messages
    if (msgChance(gen) <= 5) { // 5% chance each cycle
        V2VMessage simulatedMessage;
        simulatedMessage.vehicleId = "V2X_" + std::to_string(msgChance(gen));
        simulatedMessage.messageType = (msgChance(gen) > 50) ? "HAZARD" : "TRAFFIC";
        simulatedMessage.latitude = currentLatitude + (msgChance(gen) - 50) * 0.001;
        simulatedMessage.longitude = currentLongitude + (msgChance(gen) - 50) * 0.001;
        simulatedMessage.speed = 20.0 + msgChance(gen) * 0.3;
        simulatedMessage.payload = "Road construction ahead";
        simulatedMessage.timestamp = std::chrono::steady_clock::now();
        simulatedMessage.distance = calculateDistance(currentLatitude, currentLongitude, 
                                                     simulatedMessage.latitude, simulatedMessage.longitude);
        simulatedMessage.priority = (simulatedMessage.messageType == "HAZARD") ? 8 : 5;
        
        incomingMessages.push(simulatedMessage);
        v2vMessagesReceived++;
    }
    
    // Process incoming V2V messages
    while (!incomingMessages.empty()) {
        V2VMessage message = incomingMessages.front();
        incomingMessages.pop();
        
        // Check if message is within range and not expired
        auto messageAge = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - message.timestamp).count();
        
        if (message.distance <= V2X_RANGE_METERS && messageAge <= MESSAGE_EXPIRY_SECONDS) {
            velocitas::logger().info("📨 V2V Message: {} from {} - {} ({:.0f}m away)", 
                                    message.messageType, message.vehicleId, message.payload, message.distance);
            
            // Process different message types
            if (message.messageType == "HAZARD" && hazardDetectionEnabled) {
                if (message.distance <= HAZARD_ALERT_RANGE) {
                    velocitas::logger().warn("⚠️  HAZARD ALERT: {} - {:.0f}m ahead", 
                                           message.payload, message.distance);
                    
                    // In production, would trigger driver alerts and potentially automatic responses
                    if (currentSpeed > 15.0) { // 54 km/h
                        velocitas::logger().info("🚗 Recommended action: Reduce speed and increase following distance");
                    }
                }
            } else if (message.messageType == "EMERGENCY") {
                velocitas::logger().error("🚨 EMERGENCY: {} - Emergency vehicle approaching!", message.payload);
                emergencyMode = true;
                emergencyPriorityEvents++;
            } else if (message.messageType == "TRAFFIC") {
                velocitas::logger().info("🚦 Traffic info: {} - Optimizing route", message.payload);
                trafficOptimizations++;
            }
        }
    }
}

void V2XCommunicator::sendV2VHazardWarning(const std::string& hazardType, double latitude, double longitude) {
    hazardWarningsIssued++;
    
    std::string messageData = fmt::format(
        "{{\"vehicleId\":\"{}\",\"type\":\"HAZARD\",\"hazard\":\"{}\",\"lat\":{:.6f},\"lon\":{:.6f},\"speed\":{:.1f}}}",
        ownVehicleId, hazardType, latitude, longitude, currentSpeed
    );
    
    sendV2XMessage("V2V_HAZARD", messageData);
    velocitas::logger().info("📡 V2V Hazard broadcast: {} at [{:.6f},{:.6f}]", hazardType, latitude, longitude);
}

void V2XCommunicator::handleTrafficSignalOptimization() {
    if (!trafficOptimizationEnabled || !tisEnabled) return;
    
    for (auto& signal : nearbySignals) {
        signal.distance = calculateDistance(currentLatitude, currentLongitude, 
                                          signal.latitude, signal.longitude);
        
        if (signal.distance <= 300.0) { // Within signal communication range
            // Calculate time to intersection
            double timeToIntersection = (signal.distance / (currentSpeed + 0.1)) / 3.6; // seconds
            
            velocitas::logger().info("🚦 Traffic Signal {}: {} for {}s, {:.0f}m away, ETA: {:.1f}s", 
                                    signal.intersectionId, signal.currentPhase, signal.timeRemaining, 
                                    signal.distance, timeToIntersection);
            
            // Signal phase and timing optimization (SPAT)
            if (signal.currentPhase == "GREEN") {
                if (timeToIntersection > signal.timeRemaining) {
                    signal.optimizationAdvice = "SPEED_UP";
                    velocitas::logger().info("💡 SPAT Optimization: Increase speed to catch green light");
                } else {
                    signal.optimizationAdvice = "MAINTAIN";
                    velocitas::logger().info("✅ SPAT: Maintain current speed to catch green light");
                }
            } else if (signal.currentPhase == "RED") {
                if (timeToIntersection < signal.timeRemaining - 5) {
                    signal.optimizationAdvice = "SLOW_DOWN";
                    velocitas::logger().info("💡 SPAT Optimization: Reduce speed to avoid red light");
                }
            }
            
            trafficOptimizations++;
            
            // Simulate signal timing updates (in production, comes from infrastructure)
            signal.timeRemaining = std::max(0, signal.timeRemaining - 1);
            if (signal.timeRemaining == 0) {
                if (signal.currentPhase == "GREEN") {
                    signal.currentPhase = "YELLOW";
                    signal.timeRemaining = 5;
                } else if (signal.currentPhase == "YELLOW") {
                    signal.currentPhase = "RED";
                    signal.timeRemaining = 30;
                } else {
                    signal.currentPhase = "GREEN";
                    signal.timeRemaining = 25;
                }
            }
        }
    }
}

void V2XCommunicator::coordinateCooperativeCruiseControl(double speed) {
    if (!cooperativeCruiseEnabled) return;
    
    // Simulate nearby vehicles for cooperative cruise control
    for (auto& [vehicleId, vehicle] : connectedVehicles) {
        vehicle.distance = calculateDistance(currentLatitude, currentLongitude, 
                                           vehicle.latitude, vehicle.longitude);
        
        if (vehicle.distance <= COOP_CRUISE_RANGE) {
            // Calculate relative speed and position
            double relativeSpeed = currentSpeed - vehicle.speed;
            
            velocitas::logger().info("🤝 Cooperative Cruise: Vehicle {} at {:.0f}m, Speed diff: {:.1f}km/h", 
                                    vehicleId, vehicle.distance, relativeSpeed * 3.6);
            
            // Cooperative cruise control logic
            if (vehicle.distance < 50.0 && relativeSpeed > 2.0) {
                velocitas::logger().info("🚗 Coop Cruise: Adjusting speed to maintain safe following distance");
                cooperativeCruiseEvents++;
            }
            
            // Lane change coordination
            if (vehicle.intent == "LANE_CHANGE" && vehicle.distance < 30.0) {
                velocitas::logger().info("🔄 Lane Change Assist: Creating space for {} to merge", vehicleId);
                cooperativeCruiseEvents++;
            }
            
            // Platoon formation opportunity
            if (std::abs(relativeSpeed) < 1.0 && vehicle.distance > 20.0 && vehicle.distance < 80.0) {
                velocitas::logger().info("🚛 Platoon Opportunity: Vehicle {} - Similar speed, good spacing", vehicleId);
            }
        }
    }
    
    // Send our cooperative cruise data
    std::string coopData = fmt::format(
        "{{\"vehicleId\":\"{}\",\"speed\":{:.1f},\"intent\":\"CRUISE\",\"lat\":{:.6f},\"lon\":{:.6f}}}",
        ownVehicleId, currentSpeed, currentLatitude, currentLongitude
    );
    sendV2XMessage("COOP_CRUISE", coopData);
}

void V2XCommunicator::handleEmergencyVehiclePriority() {
    if (!emergencyPriorityEnabled) return;
    
    for (auto& emergency : emergencyVehicles) {
        emergency.distance = calculateDistance(currentLatitude, currentLongitude, 
                                             emergency.latitude, emergency.longitude);
        
        if (emergency.distance <= EMERGENCY_PRIORITY_RANGE) {
            velocitas::logger().error("🚨 EMERGENCY VEHICLE: {} {} approaching - {:.0f}m away", 
                                     emergency.vehicleType, emergency.vehicleId, emergency.distance);
            
            // Emergency vehicle priority actions
            if (emergency.distance < 200.0) {
                velocitas::logger().error("🚨 IMMEDIATE ACTION: Emergency vehicle priority - Clear lane!");
                emergencyMode = true;
                
                // In production, would trigger:
                // - Automatic lane change assistance
                // - Speed adjustment recommendations
                // - Audio/visual alerts to driver
                // - Coordination with other vehicles
                
                if (emergency.distance < 100.0) {
                    velocitas::logger().error("🚨 CRITICAL: Emergency vehicle very close - Pull over safely!");
                }
            }
            
            emergencyPriorityEvents++;
        }
    }
    
    // Reset emergency mode if no emergency vehicles nearby
    bool emergencyNearby = false;
    for (const auto& emergency : emergencyVehicles) {
        if (emergency.distance <= 300.0) {
            emergencyNearby = true;
            break;
        }
    }
    
    if (!emergencyNearby && emergencyMode) {
        emergencyMode = false;
        velocitas::logger().info("✅ Emergency cleared - Resuming normal operation");
    }
}

void V2XCommunicator::shareTrafficConditions(double speed, double latitude, double longitude) {
    // Share traffic conditions with other vehicles and infrastructure
    std::string trafficCondition;
    
    if (speed < 2.0) {
        trafficCondition = "STOPPED";
    } else if (speed < 8.0) {
        trafficCondition = "CONGESTED";
    } else if (speed < 15.0) {
        trafficCondition = "SLOW";
    } else {
        trafficCondition = "FREE_FLOW";
    }
    
    std::string trafficData = fmt::format(
        "{{\"vehicleId\":\"{}\",\"condition\":\"{}\",\"speed\":{:.1f},\"lat\":{:.6f},\"lon\":{:.6f},\"timestamp\":{}}}",
        ownVehicleId, trafficCondition, speed, latitude, longitude, 
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()
    );
    
    sendV2XMessage("TRAFFIC_CONDITIONS", trafficData);
    v2vMessagesSent++;
    
    // Simulate hazard detection
    if (speed < 2.0 && currentSpeed > 10.0) { // Sudden stop
        sendV2VHazardWarning("SUDDEN_STOP", latitude, longitude);
    }
}

void V2XCommunicator::processConnectedVehicleData() {
    // Simulate connected vehicle updates (in production, from V2X messages)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> vehicleUpdate(1, 100);
    
    if (vehicleUpdate(gen) <= 10) { // 10% chance of vehicle data update
        ConnectedVehicle vehicle;
        vehicle.vehicleId = "CV_" + std::to_string(vehicleUpdate(gen));
        vehicle.latitude = currentLatitude + (vehicleUpdate(gen) - 50) * 0.0005;
        vehicle.longitude = currentLongitude + (vehicleUpdate(gen) - 50) * 0.0005;
        vehicle.speed = 10.0 + vehicleUpdate(gen) * 0.2;
        vehicle.heading = vehicleUpdate(gen) * 3.6;
        vehicle.intent = (vehicleUpdate(gen) > 80) ? "LANE_CHANGE" : "CRUISE";
        vehicle.lastSeen = std::chrono::steady_clock::now();
        
        connectedVehicles[vehicle.vehicleId] = vehicle;
        
        // Limit number of tracked vehicles
        if (connectedVehicles.size() > MAX_CONNECTED_VEHICLES) {
            // Remove oldest vehicle
            auto oldest = std::min_element(connectedVehicles.begin(), connectedVehicles.end(),
                [](const auto& a, const auto& b) {
                    return a.second.lastSeen < b.second.lastSeen;
                });
            connectedVehicles.erase(oldest);
        }
    }
}

void V2XCommunicator::generateV2XReport() {
    auto now = std::chrono::steady_clock::now();
    auto systemRuntime = std::chrono::duration_cast<std::chrono::minutes>(now - systemStartTime).count();
    
    velocitas::logger().info("📋 V2X COMMUNICATION REPORT - Runtime: {}min", systemRuntime);
    velocitas::logger().info("📡 Vehicle ID: {}, Position: [{:.6f},{:.6f}], Speed: {:.1f}km/h", 
                            ownVehicleId, currentLatitude, currentLongitude, currentSpeed * 3.6);
    
    // Communication statistics
    velocitas::logger().info("📊 V2V Traffic: Received {} messages, Sent {} messages", 
                            v2vMessagesReceived, v2vMessagesSent);
    velocitas::logger().info("⚠️  Safety Events: {} hazard warnings, {} emergency priorities", 
                            hazardWarningsIssued, emergencyPriorityEvents);
    velocitas::logger().info("🚦 Optimization: {} traffic optimizations, {} cooperative cruise events", 
                            trafficOptimizations, cooperativeCruiseEvents);
    
    // Connected vehicles summary
    int nearbyVehicles = 0;
    for (const auto& [vehicleId, vehicle] : connectedVehicles) {
        if (calculateDistance(currentLatitude, currentLongitude, 
                            vehicle.latitude, vehicle.longitude) <= V2X_RANGE_METERS) {
            nearbyVehicles++;
        }
    }
    velocitas::logger().info("🚗 Connected Vehicles: {} nearby, {} total tracked", 
                            nearbyVehicles, connectedVehicles.size());
    
    // Traffic signals status
    int activeSignals = 0;
    for (const auto& signal : nearbySignals) {
        if (signal.distance <= 300.0) {
            activeSignals++;
            velocitas::logger().info("🚦 Signal {}: {} ({:.0f}m) - Advice: {}", 
                                    signal.intersectionId, signal.currentPhase, 
                                    signal.distance, signal.optimizationAdvice);
        }
    }
    
    // System status
    velocitas::logger().info("🔧 System Status: V2X={}, TIS={}, Emergency Mode={}", 
                            v2xSystemEnabled ? "ACTIVE" : "DEGRADED",
                            tisEnabled ? "ON" : "OFF",
                            emergencyMode ? "ACTIVE" : "NORMAL");
    
    // Performance metrics
    double messageRate = static_cast<double>(v2vMessagesReceived + v2vMessagesSent) / systemRuntime;
    velocitas::logger().info("📈 Performance: {:.1f} messages/min, {} active signals", 
                            messageRate, activeSignals);
}

void V2XCommunicator::sendV2XMessage(const std::string& messageType, const std::string& data) {
    // In production, would send via:
    // - DSRC (Dedicated Short Range Communications)
    // - C-V2X (Cellular Vehicle-to-Everything)
    // - 5G networks for extended range
    // - Edge computing platforms
    
    velocitas::logger().info("📡 V2X Broadcast [{}]: {}", messageType, data);
    lastMessageTime = std::chrono::steady_clock::now();
}

double V2XCommunicator::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Simplified distance calculation (Haversine formula approximation)
    const double R = 6371000; // Earth radius in meters
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

void V2XCommunicator::cleanupExpiredData() {
    auto now = std::chrono::steady_clock::now();
    
    // Remove expired connected vehicles
    for (auto it = connectedVehicles.begin(); it != connectedVehicles.end();) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastSeen).count();
        if (age > MESSAGE_EXPIRY_SECONDS * 2) {
            it = connectedVehicles.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remove expired emergency vehicles
    emergencyVehicles.erase(
        std::remove_if(emergencyVehicles.begin(), emergencyVehicles.end(),
            [now](const EmergencyVehicle& ev) {
                auto age = std::chrono::duration_cast<std::chrono::seconds>(now - ev.lastUpdate).count();
                return age > MESSAGE_EXPIRY_SECONDS;
            }),
        emergencyVehicles.end()
    );
}

std::string V2XCommunicator::generateVehicleId() {
    // Generate unique vehicle ID (in production, would use VIN or registered ID)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return "VEH_" + std::to_string(dis(gen));
}

// Application entry point
std::unique_ptr<V2XCommunicator> v2xApp;

void signal_handler(int sig) {
    velocitas::logger().info("🛑 V2X Communication Hub shutdown initiated (signal {})", sig);
    if (v2xApp) {
        v2xApp->stop();
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handler);
    
    velocitas::logger().info("🚀 Starting V2X Communication Hub...");
    velocitas::logger().info("📡 Vehicle-to-Everything cooperative driving and safety");
    velocitas::logger().info("💡 Press Ctrl+C to stop the system");
    
    v2xApp = std::make_unique<V2XCommunicator>();
    try {
        v2xApp->run();
    } catch (const std::exception& e) {
        velocitas::logger().error("💥 V2X communication error: {}", e.what());
        return 1;
    } catch (...) {
        velocitas::logger().error("💥 Unknown V2X communication error");
        return 1;
    }
    
    velocitas::logger().info("👋 V2X Communication Hub stopped");
    return 0;
}

// ============================================================================
// 🎓 PRODUCTION DEPLOYMENT NOTES
// ============================================================================
//
// 🏭 V2X TECHNOLOGY INTEGRATIONS:
// 1. DSRC (IEEE 802.11p) radio hardware for direct vehicle communication
// 2. C-V2X (3GPP Release 14+) cellular technology for extended range
// 3. 5G networks for high-bandwidth, low-latency communication
// 4. Edge computing platforms for real-time processing
// 5. Traffic management centers for infrastructure coordination
// 6. Emergency services integration for priority handling
// 7. Cloud platforms for data aggregation and analytics
// 8. Cybersecurity frameworks for secure V2X communication
//
// 🛡️ SAFETY AND SECURITY:
// - PKI (Public Key Infrastructure) for message authentication
// - Certificate management for vehicle identity verification
// - Message integrity and anti-replay protection
// - Privacy protection through pseudonym certificates
// - Intrusion detection for malicious message filtering
//
// 🔧 CONFIGURATION PARAMETERS:
// - V2X radio parameters (frequency, power, range)
// - Message priorities and handling policies
// - Safety application thresholds and responses
// - Traffic optimization algorithms and preferences
// - Emergency vehicle detection and response protocols
//
// ============================================================================