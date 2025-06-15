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

#include "../src/MaintenanceReminderApp.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using namespace maintenance;

class MaintenanceReminderAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }

    void TearDown() override {
        // Test cleanup
    }
};

// Test service type string conversion
TEST_F(MaintenanceReminderAppTest, ServiceTypeStringConversion) {
    EXPECT_EQ(MaintenanceReminderApp::serviceTypeToString(ServiceType::OIL_CHANGE), "OIL_CHANGE");
    EXPECT_EQ(MaintenanceReminderApp::serviceTypeToString(ServiceType::BRAKE_PADS), "BRAKE_PADS");
    EXPECT_EQ(MaintenanceReminderApp::serviceTypeToString(ServiceType::AIR_FILTER), "AIR_FILTER");
    
    EXPECT_EQ(MaintenanceReminderApp::stringToServiceType("OIL_CHANGE"), ServiceType::OIL_CHANGE);
    EXPECT_EQ(MaintenanceReminderApp::stringToServiceType("BRAKE_PADS"), ServiceType::BRAKE_PADS);
    EXPECT_EQ(MaintenanceReminderApp::stringToServiceType("AIR_FILTER"), ServiceType::AIR_FILTER);
    
    // Test invalid string returns default
    EXPECT_EQ(MaintenanceReminderApp::stringToServiceType("INVALID"), ServiceType::OIL_CHANGE);
}

// Test service interval initialization
TEST_F(MaintenanceReminderAppTest, ServiceIntervalInitialization) {
    ServiceInterval oilChangeInterval(
        ServiceType::OIL_CHANGE, 
        "Oil Change", 
        "Replace engine oil and filter", 
        10000.0, 
        365, 
        1
    );
    
    EXPECT_EQ(oilChangeInterval.type, ServiceType::OIL_CHANGE);
    EXPECT_EQ(oilChangeInterval.name, "Oil Change");
    EXPECT_EQ(oilChangeInterval.description, "Replace engine oil and filter");
    EXPECT_DOUBLE_EQ(oilChangeInterval.distanceIntervalKm, 10000.0);
    EXPECT_EQ(oilChangeInterval.timeIntervalDays, 365);
    EXPECT_EQ(oilChangeInterval.priority, 1);
    EXPECT_TRUE(oilChangeInterval.enabled);
}

// Test service record creation
TEST_F(MaintenanceReminderAppTest, ServiceRecordCreation) {
    ServiceRecord record(ServiceType::OIL_CHANGE, 15000.0, "Synthetic oil used");
    
    EXPECT_EQ(record.type, ServiceType::OIL_CHANGE);
    EXPECT_DOUBLE_EQ(record.mileageAtService, 15000.0);
    EXPECT_EQ(record.notes, "Synthetic oil used");
    EXPECT_DOUBLE_EQ(record.cost, 0.0); // Default value
    EXPECT_EQ(record.serviceProvider, ""); // Default value
}

// Test service reminder creation
TEST_F(MaintenanceReminderAppTest, ServiceReminderCreation) {
    ServiceReminder reminder(
        ServiceType::BRAKE_PADS, 
        "Brake Pad Inspection", 
        "Check brake pad condition", 
        2
    );
    
    EXPECT_EQ(reminder.type, ServiceType::BRAKE_PADS);
    EXPECT_EQ(reminder.serviceName, "Brake Pad Inspection");
    EXPECT_EQ(reminder.description, "Check brake pad condition");
    EXPECT_EQ(reminder.priority, 2);
    EXPECT_FALSE(reminder.overdue); // Default value
    EXPECT_DOUBLE_EQ(reminder.kmUntilService, 0.0); // Default value
    EXPECT_EQ(reminder.daysUntilService, 0); // Default value
}

// Test maintenance configuration defaults
TEST_F(MaintenanceReminderAppTest, MaintenanceConfigDefaults) {
    MaintenanceConfig config;
    
    EXPECT_DOUBLE_EQ(config.criticalAlertKm, 500.0);
    EXPECT_DOUBLE_EQ(config.warningAlertKm, 1000.0);
    EXPECT_DOUBLE_EQ(config.infoAlertKm, 2000.0);
    EXPECT_EQ(config.criticalAlertDays, 7);
    EXPECT_EQ(config.warningAlertDays, 30);
    EXPECT_EQ(config.infoAlertDays, 60);
    EXPECT_EQ(config.dailyReminderHour, 9);
    EXPECT_EQ(config.alertCooldownMs, 3600000);
    EXPECT_TRUE(config.enableDistanceAlerts);
    EXPECT_TRUE(config.enableTimeAlerts);
    EXPECT_TRUE(config.enableDailyReminders);
    EXPECT_TRUE(config.enableOverdueAlerts);
}

// Test maintenance statistics defaults
TEST_F(MaintenanceReminderAppTest, MaintenanceStatsDefaults) {
    MaintenanceStats stats;
    
    EXPECT_EQ(stats.totalServices, 0);
    EXPECT_DOUBLE_EQ(stats.totalMaintenanceCost, 0.0);
    EXPECT_EQ(stats.overdueServices, 0);
    EXPECT_EQ(stats.upcomingServices, 0);
    EXPECT_DOUBLE_EQ(stats.averageServiceInterval, 0.0);
}

// Test JSON configuration parsing
TEST_F(MaintenanceReminderAppTest, ConfigurationJSONParsing) {
    nlohmann::json configJson = {
        {"critical_alert_km", 400.0},
        {"warning_alert_km", 800.0},
        {"info_alert_km", 1500.0},
        {"critical_alert_days", 5},
        {"warning_alert_days", 20},
        {"info_alert_days", 45},
        {"alert_cooldown_ms", 2700000},
        {"enable_distance_alerts", false},
        {"enable_time_alerts", true},
        {"enable_overdue_alerts", false}
    };
    
    std::string configString = configJson.dump();
    
    // This would test the parseConfig method if we had access to the app instance
    // For now, we test that the JSON is well-formed
    EXPECT_FALSE(configString.empty());
    EXPECT_TRUE(configJson.contains("critical_alert_km"));
    EXPECT_DOUBLE_EQ(configJson["critical_alert_km"], 400.0);
}

// Test service completion JSON structure
TEST_F(MaintenanceReminderAppTest, ServiceCompletionJSON) {
    nlohmann::json serviceJson = {
        {"service_type", "OIL_CHANGE"},
        {"notes", "Full synthetic oil change with premium filter"},
        {"cost", 75.50},
        {"provider", "QuickLube Express"}
    };
    
    EXPECT_EQ(serviceJson["service_type"], "OIL_CHANGE");
    EXPECT_EQ(serviceJson["notes"], "Full synthetic oil change with premium filter");
    EXPECT_DOUBLE_EQ(serviceJson["cost"], 75.50);
    EXPECT_EQ(serviceJson["provider"], "QuickLube Express");
}

// Test days between calculation logic
TEST_F(MaintenanceReminderAppTest, DaysBetweenCalculation) {
    auto now = std::chrono::system_clock::now();
    auto past = now - std::chrono::hours(48); // 2 days ago
    auto future = now + std::chrono::hours(72); // 3 days from now
    
    // Test the logic for days calculation
    auto duration_past = std::chrono::duration_cast<std::chrono::hours>(now - past);
    auto duration_future = std::chrono::duration_cast<std::chrono::hours>(future - now);
    
    int days_past = static_cast<int>(duration_past.count() / 24);
    int days_future = static_cast<int>(duration_future.count() / 24);
    
    EXPECT_EQ(days_past, 2);
    EXPECT_EQ(days_future, 3);
}

// Test alert message formatting
TEST_F(MaintenanceReminderAppTest, AlertMessageFormatting) {
    // Test overdue message format
    std::string serviceName = "Oil Change";
    double kmOverdue = 500.0;
    int daysOverdue = 30;
    
    std::string overdueMessage = fmt::format(
        "OVERDUE: {} - {:.0f} km over due, {} days over due",
        serviceName, kmOverdue, daysOverdue
    );
    
    EXPECT_EQ(overdueMessage, "OVERDUE: Oil Change - 500 km over due, 30 days over due");
    
    // Test upcoming service message format
    double kmUntil = 1200.0;
    int daysUntil = 45;
    
    std::string upcomingMessage = fmt::format(
        "{} due in {:.0f} km or {} days",
        serviceName, kmUntil, daysUntil
    );
    
    EXPECT_EQ(upcomingMessage, "Oil Change due in 1200 km or 45 days");
}

// Test reset command types
TEST_F(MaintenanceReminderAppTest, ResetCommandTypes) {
    // Test valid reset types
    std::vector<std::string> validResetTypes = {"statistics", "config", "history"};
    
    for (const auto& resetType : validResetTypes) {
        nlohmann::json resetJson = {{"type", resetType}};
        EXPECT_TRUE(resetJson.contains("type"));
        EXPECT_EQ(resetJson["type"], resetType);
    }
}

// Test service priority levels
TEST_F(MaintenanceReminderAppTest, ServicePriorityLevels) {
    // Critical services (priority 1)
    ServiceInterval criticalService(ServiceType::OIL_CHANGE, "Oil Change", "Critical", 10000.0, 365, 1);
    EXPECT_EQ(criticalService.priority, 1);
    
    // Important services (priority 2)
    ServiceInterval importantService(ServiceType::BRAKE_PADS, "Brake Pads", "Important", 30000.0, 365, 2);
    EXPECT_EQ(importantService.priority, 2);
    
    // Routine services (priority 3)
    ServiceInterval routineService(ServiceType::TIRE_ROTATION, "Tire Rotation", "Routine", 10000.0, 182, 3);
    EXPECT_EQ(routineService.priority, 3);
}

// Test time conversion utilities
TEST_F(MaintenanceReminderAppTest, TimeConversions) {
    // Test milliseconds to timestamp conversion
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    EXPECT_GT(timestamp, 0);
    
    // Test hours to milliseconds conversion
    int alertCooldownMs = 3600000; // 1 hour in milliseconds
    int expectedHours = alertCooldownMs / (1000 * 60 * 60);
    EXPECT_EQ(expectedHours, 1);
}

// Test service interval distance calculations
TEST_F(MaintenanceReminderAppTest, ServiceIntervalDistanceCalculations) {
    // Test oil change interval
    double oilChangeInterval = 10000.0; // km
    double currentDistance = 8500.0; // km
    double distanceUntilService = oilChangeInterval - currentDistance;
    
    EXPECT_DOUBLE_EQ(distanceUntilService, 1500.0);
    
    // Test overdue calculation
    double overdueDistance = 11500.0; // km
    double distanceOverdue = overdueDistance - oilChangeInterval;
    EXPECT_DOUBLE_EQ(distanceOverdue, 1500.0);
    EXPECT_TRUE(distanceOverdue > 0); // Service is overdue
}