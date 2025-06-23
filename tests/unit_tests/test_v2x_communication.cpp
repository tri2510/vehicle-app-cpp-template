// ============================================================================
// 🧪 UNIT TESTS - V2X Communication Hub
// ============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>

class V2XTest : public ::testing::Test {
protected:
    static constexpr double V2X_RANGE_METERS = 300.0;
    static constexpr double HAZARD_ALERT_RANGE = 200.0;
    static constexpr double EMERGENCY_PRIORITY_RANGE = 500.0;
    
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        const double R = 6371000; // Earth radius in meters
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
        double a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon/2) * sin(dLon/2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        return R * c;
    }
};

TEST_F(V2XTest, CommunicationRangeValidation) {
    struct RangeTest {
        double lat1, lon1, lat2, lon2;
        bool shouldCommunicate;
        std::string description;
    };
    
    std::vector<RangeTest> tests = {
        {40.7589, -73.9851, 40.7614, -73.9776, true, "Close vehicles in NYC"},
        {40.7589, -73.9851, 40.8589, -73.9851, false, "Vehicles too far apart"},
        {40.7589, -73.9851, 40.7589, -73.9851, true, "Same position"},
    };
    
    for (const auto& test : tests) {
        double distance = calculateDistance(test.lat1, test.lon1, test.lat2, test.lon2);
        bool inRange = distance <= V2X_RANGE_METERS;
        
        EXPECT_EQ(inRange, test.shouldCommunicate) 
            << "Communication range test failed for: " << test.description 
            << " (distance: " << distance << "m)";
    }
}

TEST_F(V2XTest, MessagePriorityHandling) {
    enum MessagePriority { INFO = 1, WARNING = 5, CRITICAL = 8, EMERGENCY = 10 };
    
    struct MessageTest {
        std::string messageType;
        MessagePriority expectedPriority;
        bool shouldAlert;
    };
    
    std::vector<MessageTest> messages = {
        {"TRAFFIC", INFO, false},
        {"HAZARD", WARNING, true},
        {"EMERGENCY", EMERGENCY, true},
        {"COOP_CRUISE", INFO, false},
    };
    
    for (const auto& msg : messages) {
        bool isHighPriority = msg.expectedPriority >= CRITICAL;
        bool requiresAlert = msg.messageType == "HAZARD" || msg.messageType == "EMERGENCY";
        
        EXPECT_EQ(requiresAlert, msg.shouldAlert) 
            << "Alert requirement incorrect for message type: " << msg.messageType;
    }
}

TEST_F(V2XTest, TrafficSignalOptimization) {
    struct SignalTest {
        std::string phase;        // RED, YELLOW, GREEN
        int timeRemaining;        // seconds
        double distanceToSignal;  // meters
        double currentSpeed;      // m/s
        std::string expectedAdvice;
    };
    
    std::vector<SignalTest> tests = {
        {"GREEN", 20, 200.0, 10.0, "MAINTAIN"}, // Can make it through
        {"GREEN", 5, 200.0, 10.0, "SPEED_UP"},  // Need to hurry
        {"RED", 30, 150.0, 8.0, "SLOW_DOWN"},   // Wait for green
        {"YELLOW", 3, 50.0, 15.0, "MAINTAIN"},  // Continue through
    };
    
    for (const auto& test : tests) {
        double timeToIntersection = test.distanceToSignal / (test.currentSpeed + 0.1);
        std::string advice = "MAINTAIN";
        
        if (test.phase == "GREEN" && timeToIntersection > test.timeRemaining) {
            advice = "SPEED_UP";
        } else if (test.phase == "RED" && timeToIntersection < test.timeRemaining - 5) {
            advice = "SLOW_DOWN";
        }
        
        EXPECT_EQ(advice, test.expectedAdvice) 
            << "Signal optimization advice incorrect for phase: " << test.phase 
            << ", time remaining: " << test.timeRemaining;
    }
}

TEST_F(V2XTest, EmergencyVehiclePriority) {
    struct EmergencyTest {
        std::string vehicleType;
        double distance;
        bool shouldActivatePriority;
        std::string expectedAction;
    };
    
    std::vector<EmergencyTest> tests = {
        {"AMBULANCE", 600.0, false, "NONE"},
        {"AMBULANCE", 400.0, true, "ALERT"},
        {"AMBULANCE", 150.0, true, "CLEAR_LANE"},
        {"FIRE", 80.0, true, "PULL_OVER"},
        {"POLICE", 250.0, true, "ALERT"},
    };
    
    for (const auto& test : tests) {
        bool inRange = test.distance <= EMERGENCY_PRIORITY_RANGE;
        std::string action = "NONE";
        
        if (inRange) {
            if (test.distance < 100.0) {
                action = "PULL_OVER";
            } else if (test.distance < 200.0) {
                action = "CLEAR_LANE";
            } else {
                action = "ALERT";
            }
        }
        
        EXPECT_EQ(inRange, test.shouldActivatePriority) 
            << "Emergency priority activation incorrect for distance: " << test.distance;
        
        if (test.shouldActivatePriority) {
            EXPECT_EQ(action, test.expectedAction) 
                << "Emergency action incorrect for " << test.vehicleType 
                << " at distance: " << test.distance;
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}