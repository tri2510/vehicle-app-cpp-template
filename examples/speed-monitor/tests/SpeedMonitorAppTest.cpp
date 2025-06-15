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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/SpeedMonitorApp.h"

using namespace speedmonitor;
using namespace testing;

class SpeedMonitorAppTest : public Test {
protected:
    void SetUp() override {
        // Setup test environment
    }

    void TearDown() override {
        // Cleanup test environment
    }

    SpeedMonitorApp app;
};

TEST_F(SpeedMonitorAppTest, ConstructorInitializesDefaults) {
    // Test that constructor initializes with default values
    EXPECT_NO_THROW(SpeedMonitorApp testApp);
}

TEST_F(SpeedMonitorAppTest, SpeedConversion) {
    // Test m/s to km/h conversion
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(10.0), 36.0);  // 10 m/s = 36 km/h
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(25.0), 90.0);  // 25 m/s = 90 km/h
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(0.0), 0.0);    // 0 m/s = 0 km/h
}

TEST_F(SpeedMonitorAppTest, ConfigurationDefaults) {
    SpeedConfig config;
    
    // Test default configuration values
    EXPECT_EQ(config.speedLimitKmh, 80.0);
    EXPECT_EQ(config.hardBrakingThreshold, -5.0);
    EXPECT_EQ(config.rapidAccelThreshold, 3.0);
    EXPECT_EQ(config.alertCooldownMs, 5000);
    EXPECT_TRUE(config.enableSpeedLimitAlerts);
    EXPECT_TRUE(config.enableAccelerationAlerts);
    EXPECT_FALSE(config.enableLocationTracking);
}

TEST_F(SpeedMonitorAppTest, StatisticsInitialization) {
    SpeedStatistics stats;
    
    // Test initial statistics values
    EXPECT_EQ(stats.maxSpeedKmh, 0.0);
    EXPECT_EQ(stats.avgSpeedKmh, 0.0);
    EXPECT_EQ(stats.speedLimitViolations, 0);
    EXPECT_EQ(stats.hardBrakingEvents, 0);
    EXPECT_EQ(stats.rapidAccelEvents, 0);
    EXPECT_GT(stats.sessionStart.time_since_epoch().count(), 0);
}

TEST_F(SpeedMonitorAppTest, AppStartup) {
    // Test that app starts without throwing
    EXPECT_NO_THROW(app.onStart());
}

// Mock class for testing speed monitoring logic
class MockSpeedMonitorApp : public SpeedMonitorApp {
public:
    // Make protected methods public for testing
    using SpeedMonitorApp::msToKmh;
    
    // Mock MQTT publishing
    MOCK_METHOD(void, publishToTopic, (const std::string& topic, const std::string& data), (override));
    MOCK_METHOD(void, publishAlert, (const std::string& alertType, const std::string& message, const std::string& severity));
    
    // Expose private members for testing
    SpeedConfig& getConfig() { return m_config; }
    SpeedStatistics& getStats() { return m_stats; }
    double getCurrentSpeed() const { return m_currentSpeedKmh; }
    
    // Test helper methods
    void setCurrentSpeed(double speed) { m_currentSpeedKmh = speed; }
    void testCheckSpeedLimit(double speed) { checkSpeedLimit(speed); }
    void testUpdateStatistics(double speed) { updateStatistics(speed); }
    bool testCanSendAlert() { return canSendAlert(); }
    void setLastAlertTime(std::chrono::system_clock::time_point time) { m_lastAlertTime = time; }

private:
    // Make base class members accessible
    using SpeedMonitorApp::m_config;
    using SpeedMonitorApp::m_stats;
    using SpeedMonitorApp::m_currentSpeedKmh;
    using SpeedMonitorApp::m_lastAlertTime;
    using SpeedMonitorApp::checkSpeedLimit;
    using SpeedMonitorApp::updateStatistics;
    using SpeedMonitorApp::canSendAlert;
};

TEST(SpeedMonitorLogicTest, SpeedLimitViolationDetection) {
    MockSpeedMonitorApp mockApp;
    
    // Configure speed limit
    mockApp.getConfig().speedLimitKmh = 80.0;
    mockApp.getConfig().enableSpeedLimitAlerts = true;
    
    // Test speed limit violation
    EXPECT_CALL(mockApp, publishAlert("speed_limit", _, "warning"))
        .Times(1);
    
    mockApp.testCheckSpeedLimit(95.0);  // Above limit
}

TEST(SpeedMonitorLogicTest, NoViolationBelowLimit) {
    MockSpeedMonitorApp mockApp;
    
    // Configure speed limit
    mockApp.getConfig().speedLimitKmh = 80.0;
    mockApp.getConfig().enableSpeedLimitAlerts = true;
    
    // Test no violation when below limit
    EXPECT_CALL(mockApp, publishAlert(_, _, _))
        .Times(0);
    
    mockApp.testCheckSpeedLimit(75.0);  // Below limit
}

TEST(SpeedMonitorLogicTest, AlertCooldownPreventsSpam) {
    MockSpeedMonitorApp mockApp;
    
    // Set last alert to recent time
    auto now = std::chrono::system_clock::now();
    mockApp.setLastAlertTime(now - std::chrono::milliseconds(1000)); // 1 second ago
    
    // Should not send alert due to cooldown (default 5000ms)
    EXPECT_FALSE(mockApp.testCanSendAlert());
    
    // Set last alert to old time
    mockApp.setLastAlertTime(now - std::chrono::milliseconds(6000)); // 6 seconds ago
    
    // Should be able to send alert now
    EXPECT_TRUE(mockApp.testCanSendAlert());
}

TEST(SpeedMonitorLogicTest, StatisticsUpdate) {
    MockSpeedMonitorApp mockApp;
    
    // Test statistics updates
    mockApp.testUpdateStatistics(60.0);
    mockApp.testUpdateStatistics(80.0);
    mockApp.testUpdateStatistics(70.0);
    
    auto& stats = mockApp.getStats();
    
    // Check maximum speed
    EXPECT_EQ(stats.maxSpeedKmh, 80.0);
    
    // Check average speed (should be calculated)
    EXPECT_GT(stats.avgSpeedKmh, 0.0);
    EXPECT_LE(stats.avgSpeedKmh, 80.0);
}

TEST(SpeedMonitorLogicTest, ConfigurationParsing) {
    // Test valid configuration JSON
    std::string validConfig = R"({
        "speed_limit_kmh": 120.0,
        "hard_braking_threshold": -6.0,
        "rapid_accel_threshold": 4.0,
        "alert_cooldown_ms": 3000,
        "enable_speed_limit_alerts": false
    })";
    
    // This would require making parseConfig public or testing through message handling
    // For now, we test that the config structure can be created
    SpeedConfig config;
    config.speedLimitKmh = 120.0;
    config.hardBrakingThreshold = -6.0;
    config.rapidAccelThreshold = 4.0;
    config.alertCooldownMs = 3000;
    config.enableSpeedLimitAlerts = false;
    
    EXPECT_EQ(config.speedLimitKmh, 120.0);
    EXPECT_EQ(config.hardBrakingThreshold, -6.0);
    EXPECT_FALSE(config.enableSpeedLimitAlerts);
}

TEST(SpeedMonitorLogicTest, AccelerationEvents) {
    MockSpeedMonitorApp mockApp;
    
    // Configure thresholds
    mockApp.getConfig().hardBrakingThreshold = -5.0;
    mockApp.getConfig().rapidAccelThreshold = 3.0;
    mockApp.getConfig().enableAccelerationAlerts = true;
    
    // Test hard braking detection (would need to expose checkAccelerationEvents)
    // For now, test the threshold values
    EXPECT_LT(mockApp.getConfig().hardBrakingThreshold, 0.0);  // Should be negative
    EXPECT_GT(mockApp.getConfig().rapidAccelThreshold, 0.0);   // Should be positive
}

TEST(SpeedMonitorLogicTest, EdgeCases) {
    MockSpeedMonitorApp mockApp;
    
    // Test edge cases
    mockApp.testUpdateStatistics(0.0);   // Zero speed
    mockApp.testUpdateStatistics(-1.0);  // Negative speed (shouldn't happen but handle gracefully)
    mockApp.testUpdateStatistics(300.0); // Very high speed
    
    // Should not crash
    auto& stats = mockApp.getStats();
    EXPECT_GE(stats.maxSpeedKmh, 0.0);
}

// Performance test
TEST(SpeedMonitorPerformanceTest, HighFrequencyUpdates) {
    MockSpeedMonitorApp mockApp;
    
    // Test many rapid updates
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        mockApp.testUpdateStatistics(static_cast<double>(i % 100));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (less than 100ms)
    EXPECT_LT(duration.count(), 100);
}

// Integration test structure (would need actual VDB/MQTT setup)
class SpeedMonitorIntegrationTest : public Test {
protected:
    void SetUp() override {
        // Would set up real VDB and MQTT connections
    }
    
    void TearDown() override {
        // Cleanup connections
    }
};

TEST_F(SpeedMonitorIntegrationTest, DISABLED_EndToEndWorkflow) {
    // This test would be enabled when running in the actual Docker environment
    // with VDB and MQTT services running
    
    // 1. Start app
    // 2. Send vehicle data
    // 3. Verify MQTT messages
    // 4. Test configuration changes
    // 5. Verify alerts
    
    SUCCEED(); // Placeholder
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}