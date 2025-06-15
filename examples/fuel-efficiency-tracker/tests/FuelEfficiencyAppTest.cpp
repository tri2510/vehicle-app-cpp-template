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
#include "../src/FuelEfficiencyApp.h"

using namespace fueltracker;
using namespace testing;

class FuelEfficiencyAppTest : public Test {
protected:
    void SetUp() override {
        // Setup test environment
    }

    void TearDown() override {
        // Cleanup test environment
    }

    FuelEfficiencyApp app;
};

TEST_F(FuelEfficiencyAppTest, ConstructorInitializesDefaults) {
    // Test that constructor initializes with default values
    EXPECT_NO_THROW(FuelEfficiencyApp testApp);
}

TEST_F(FuelEfficiencyAppTest, ConversionFunctions) {
    // Test L/100km to km/L conversion
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::consumptionToEfficiency(10.0), 10.0);
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::consumptionToEfficiency(5.0), 20.0);
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::consumptionToEfficiency(0.0), 0.0);
    
    // Test km/L to L/100km conversion
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::efficiencyToConsumption(10.0), 10.0);
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::efficiencyToConsumption(20.0), 5.0);
    EXPECT_DOUBLE_EQ(FuelEfficiencyApp::efficiencyToConsumption(0.0), 999.0);
}

TEST_F(FuelEfficiencyAppTest, ConfigurationParsing) {
    // Test valid configuration JSON
    std::string validConfig = R"({
        "target_efficiency_kmL": 20.0,
        "low_efficiency_threshold": 10.0,
        "enable_efficiency_alerts": true
    })";
    
    // Since parseConfig is private, we'd need to test through public interface
    // or make it protected/public for testing
    EXPECT_NO_THROW(app.onStart());
}

TEST_F(FuelEfficiencyAppTest, EfficiencyStats) {
    EfficiencyStats stats;
    
    // Test initial values
    EXPECT_EQ(stats.currentEfficiencyKmL, 0.0);
    EXPECT_EQ(stats.averageEfficiencyKmL, 0.0);
    EXPECT_EQ(stats.bestEfficiencyKmL, 0.0);
    EXPECT_EQ(stats.worstEfficiencyKmL, 999.0);
    EXPECT_EQ(stats.tripCount, 0);
}

TEST_F(FuelEfficiencyAppTest, TripData) {
    TripData trip;
    
    // Test initial state
    EXPECT_FALSE(trip.isActive);
    EXPECT_EQ(trip.totalDistance, 0.0);
    EXPECT_EQ(trip.totalFuelConsumed, 0.0);
    EXPECT_EQ(trip.efficiency, 0.0);
}

TEST_F(FuelEfficiencyAppTest, FuelConfig) {
    FuelConfig config;
    
    // Test default values
    EXPECT_EQ(config.targetEfficiencyKmL, 15.0);
    EXPECT_EQ(config.lowEfficiencyThreshold, 8.0);
    EXPECT_EQ(config.highConsumptionThreshold, 12.0);
    EXPECT_TRUE(config.enableEfficiencyAlerts);
    EXPECT_TRUE(config.enableTripTracking);
    EXPECT_TRUE(config.enableEcoTips);
}

TEST_F(FuelEfficiencyAppTest, EfficiencyPoint) {
    EfficiencyPoint point(5.0, 60.0, 100.0);
    
    EXPECT_EQ(point.instantConsumption, 5.0);
    EXPECT_EQ(point.speed, 60.0);
    EXPECT_EQ(point.distance, 100.0);
    EXPECT_GT(point.timestamp.time_since_epoch().count(), 0);
}

// Integration tests would require mocking the VehicleApp base class
// and DataPointReply objects, which would need more extensive setup

class MockFuelEfficiencyApp : public FuelEfficiencyApp {
public:
    // Make protected methods public for testing
    using FuelEfficiencyApp::consumptionToEfficiency;
    using FuelEfficiencyApp::efficiencyToConsumption;
    
    // Mock MQTT publishing for testing
    MOCK_METHOD(void, publishToTopic, (const std::string& topic, const std::string& data), (override));
};

TEST(FuelEfficiencyIntegrationTest, AlertGeneration) {
    MockFuelEfficiencyApp mockApp;
    
    // Test that alerts are published when efficiency is low
    EXPECT_CALL(mockApp, publishToTopic(_, _))
        .Times(AtLeast(1));
        
    // This would require setting up proper mock data and calling the app
    // mockApp.checkEfficiencyAlerts();
}

// Additional tests for specific scenarios
TEST_F(FuelEfficiencyAppTest, HighEfficiencyScenario) {
    // Test calculation with high efficiency values
    double consumption = 4.0; // L/100km
    double efficiency = FuelEfficiencyApp::consumptionToEfficiency(consumption);
    EXPECT_DOUBLE_EQ(efficiency, 25.0); // km/L
}

TEST_F(FuelEfficiencyAppTest, LowEfficiencyScenario) {
    // Test calculation with low efficiency values
    double consumption = 15.0; // L/100km
    double efficiency = FuelEfficiencyApp::consumptionToEfficiency(consumption);
    EXPECT_DOUBLE_EQ(efficiency, 100.0/15.0); // km/L
}

TEST_F(FuelEfficiencyAppTest, EdgeCases) {
    // Test edge cases for conversion functions
    EXPECT_EQ(FuelEfficiencyApp::consumptionToEfficiency(-1.0), 0.0);
    EXPECT_EQ(FuelEfficiencyApp::efficiencyToConsumption(-1.0), 999.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}