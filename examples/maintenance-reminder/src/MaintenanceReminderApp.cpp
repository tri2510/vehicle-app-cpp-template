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

#include "MaintenanceReminderApp.h"
#include "sdk/IPubSubClient.h"
#include "sdk/Logger.h"
#include "sdk/QueryBuilder.h"
#include "sdk/vdb/IVehicleDataBrokerClient.h"

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <numeric>

namespace maintenance {

MaintenanceReminderApp::MaintenanceReminderApp()
    : VehicleApp(velocitas::IVehicleDataBrokerClient::createInstance("vehicledatabroker"),
                 velocitas::IPubSubClient::createInstance("MaintenanceReminderApp")) {
    m_lastReminderCheck = std::chrono::system_clock::now() - std::chrono::hours(1);
    initializeServiceIntervals();
}

void MaintenanceReminderApp::onStart() {
    velocitas::logger().info("Maintenance Reminder App starting...");

    // Subscribe to vehicle distance data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.TraveledDistance).build())
        ->onItem([this](auto&& item) { onDistanceChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    // Subscribe to service distance to service data (if available)
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Service.DistanceToService).build())
        ->onItem([this](auto&& item) { onServiceDistanceChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { 
            velocitas::logger().debug("Service distance data not available: {}", status.errorMessage()); 
        });

    // Subscribe to MQTT topics
    subscribeToTopic(TOPIC_CONFIG)
        ->onItem([this](auto&& data) { onConfigReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    subscribeToTopic(TOPIC_SERVICE_COMPLETED)
        ->onItem([this](auto&& data) { onServiceCompletedReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    subscribeToTopic(TOPIC_RESET)
        ->onItem([this](auto&& data) { onResetReceived(std::forward<decltype(data)>(data)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });

    velocitas::logger().info("Maintenance Reminder App started successfully");
    publishStatus();
}

void MaintenanceReminderApp::onDistanceChanged(const velocitas::DataPointReply& reply) {
    try {
        auto distanceValue = reply.get(Vehicle.TraveledDistance)->value();
        m_currentDistanceKm = distanceValue / 1000.0; // Convert m to km
        
        velocitas::logger().debug("Distance changed: {:.2f} km", m_currentDistanceKm);
        
        // Check maintenance reminders periodically (every 1000 km or 1 hour)
        auto now = std::chrono::system_clock::now();
        auto timeSinceLastCheck = std::chrono::duration_cast<std::chrono::hours>(now - m_lastReminderCheck);
        
        if (timeSinceLastCheck.count() >= 1) {
            checkMaintenanceReminders();
            m_lastReminderCheck = now;
        }
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing distance data: {}", e.what());
    }
}

void MaintenanceReminderApp::onEngineHoursChanged(const velocitas::DataPointReply& reply) {
    try {
        auto hoursValue = reply.get(Vehicle.Powertrain.CombustionEngine.TotalOperatingTime)->value();
        m_currentEngineHours = hoursValue / 3600.0; // Convert seconds to hours
        
        velocitas::logger().debug("Engine hours changed: {:.2f} hours", m_currentEngineHours);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing engine hours data: {}", e.what());
    }
}

void MaintenanceReminderApp::onServiceDistanceChanged(const velocitas::DataPointReply& reply) {
    try {
        auto distanceValue = reply.get(Vehicle.Service.DistanceToService)->value();
        m_serviceDistanceKm = distanceValue / 1000.0; // Convert m to km
        
        velocitas::logger().debug("Service distance changed: {:.2f} km", m_serviceDistanceKm);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing service distance data: {}", e.what());
    }
}

void MaintenanceReminderApp::onConfigReceived(const std::string& data) {
    velocitas::logger().info("Configuration update received: {}", data);
    
    if (parseConfig(data)) {
        velocitas::logger().info("Configuration updated successfully");
        publishStatus();
    } else {
        velocitas::logger().error("Failed to parse configuration");
        publishAlert("config_error", "Invalid configuration received", "error");
    }
}

void MaintenanceReminderApp::onServiceCompletedReceived(const std::string& data) {
    try {
        auto json = nlohmann::json::parse(data);
        
        std::string serviceTypeStr = json.value("service_type", "");
        ServiceType serviceType = stringToServiceType(serviceTypeStr);
        
        std::string notes = json.value("notes", "");
        double cost = json.value("cost", 0.0);
        std::string provider = json.value("provider", "");
        
        recordServiceCompletion(serviceType, notes, cost, provider);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing service completion: {}", e.what());
        publishAlert("service_error", "Failed to process service completion", "error");
    }
}

void MaintenanceReminderApp::onServiceScheduledReceived(const std::string& data) {
    try {
        auto json = nlohmann::json::parse(data);
        
        std::string serviceTypeStr = json.value("service_type", "");
        ServiceType serviceType = stringToServiceType(serviceTypeStr);
        
        std::string message = fmt::format(
            "{} scheduled for next service",
            serviceTypeToString(serviceType)
        );
        
        publishAlert("service_scheduled", message, "info");
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing service scheduling: {}", e.what());
    }
}

void MaintenanceReminderApp::onResetReceived(const std::string& data) {
    try {
        auto json = nlohmann::json::parse(data);
        std::string resetType = json.value("type", "statistics");
        
        if (resetType == "statistics") {
            m_stats = MaintenanceStats();
            velocitas::logger().info("Statistics reset");
            publishAlert("reset", "Statistics have been reset", "info");
        } else if (resetType == "config") {
            m_config = MaintenanceConfig();
            velocitas::logger().info("Configuration reset to defaults");
            publishAlert("reset", "Configuration reset to defaults", "info");
        } else if (resetType == "history") {
            m_serviceHistory.clear();
            velocitas::logger().info("Service history cleared");
            publishAlert("reset", "Service history has been cleared", "info");
        }
        
        publishStatus();
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error processing reset command: {}", e.what());
        publishAlert("reset_error", "Failed to process reset command", "error");
    }
}

void MaintenanceReminderApp::onError(const velocitas::Status& status) {
    velocitas::logger().error("Error in Maintenance Reminder App: {}", status.errorMessage());
    publishAlert("system_error", fmt::format("System error: {}", status.errorMessage()), "error");
}

void MaintenanceReminderApp::initializeServiceIntervals() {
    // Critical services
    m_serviceIntervals[ServiceType::OIL_CHANGE] = 
        ServiceInterval(ServiceType::OIL_CHANGE, "Oil Change", 
                       "Replace engine oil and oil filter", 10000.0, 365, 1);
    
    m_serviceIntervals[ServiceType::BRAKE_FLUID] = 
        ServiceInterval(ServiceType::BRAKE_FLUID, "Brake Fluid Change", 
                       "Replace brake fluid", 40000.0, 730, 1);
                       
    // Important services
    m_serviceIntervals[ServiceType::BRAKE_PADS] = 
        ServiceInterval(ServiceType::BRAKE_PADS, "Brake Pad Inspection", 
                       "Check brake pad thickness and condition", 30000.0, 365, 2);
    
    m_serviceIntervals[ServiceType::AIR_FILTER] = 
        ServiceInterval(ServiceType::AIR_FILTER, "Air Filter Replacement", 
                       "Replace engine air filter", 20000.0, 365, 2);
                       
    m_serviceIntervals[ServiceType::TRANSMISSION_FLUID] = 
        ServiceInterval(ServiceType::TRANSMISSION_FLUID, "Transmission Fluid Change", 
                       "Replace transmission fluid", 60000.0, 1095, 2);
    
    // Routine services
    m_serviceIntervals[ServiceType::TIRE_ROTATION] = 
        ServiceInterval(ServiceType::TIRE_ROTATION, "Tire Rotation", 
                       "Rotate tires for even wear", 10000.0, 182, 3);
                       
    m_serviceIntervals[ServiceType::GENERAL_INSPECTION] = 
        ServiceInterval(ServiceType::GENERAL_INSPECTION, "General Inspection", 
                       "Comprehensive vehicle inspection", 20000.0, 365, 3);
                       
    m_serviceIntervals[ServiceType::COOLANT] = 
        ServiceInterval(ServiceType::COOLANT, "Coolant Change", 
                       "Replace engine coolant", 80000.0, 1460, 2);
    
    m_serviceIntervals[ServiceType::SPARK_PLUGS] = 
        ServiceInterval(ServiceType::SPARK_PLUGS, "Spark Plug Replacement", 
                       "Replace spark plugs", 50000.0, 1095, 2);
                       
    m_serviceIntervals[ServiceType::TIMING_BELT] = 
        ServiceInterval(ServiceType::TIMING_BELT, "Timing Belt Replacement", 
                       "Replace timing belt", 100000.0, 2190, 1);
                       
    m_serviceIntervals[ServiceType::TIRE_REPLACEMENT] = 
        ServiceInterval(ServiceType::TIRE_REPLACEMENT, "Tire Replacement", 
                       "Replace worn tires", 80000.0, 1825, 2);
                       
    m_serviceIntervals[ServiceType::BATTERY] = 
        ServiceInterval(ServiceType::BATTERY, "Battery Inspection", 
                       "Check battery condition", 0.0, 365, 3);
                       
    velocitas::logger().info("Initialized {} service intervals", m_serviceIntervals.size());
}

void MaintenanceReminderApp::checkMaintenanceReminders() {
    std::vector<ServiceReminder> dueReminders;
    std::vector<ServiceReminder> upcomingReminders;
    
    for (const auto& [serviceType, interval] : m_serviceIntervals) {
        if (!interval.enabled) continue;
        
        ServiceReminder reminder = calculateServiceReminder(serviceType);
        
        if (reminder.overdue) {
            dueReminders.push_back(reminder);
        } else if (reminder.kmUntilService <= m_config.warningAlertKm || 
                  reminder.daysUntilService <= m_config.warningAlertDays) {
            upcomingReminders.push_back(reminder);
        }
    }
    
    // Update statistics
    m_stats.overdueServices = static_cast<int>(dueReminders.size());
    m_stats.upcomingServices = static_cast<int>(upcomingReminders.size());
    
    // Send alerts for overdue services
    for (const auto& reminder : dueReminders) {
        if (canSendAlert(reminder.type)) {
            std::string message = fmt::format(
                "OVERDUE: {} - {:.0f} km over due, {} days over due",
                reminder.serviceName,
                std::abs(reminder.kmUntilService),
                std::abs(reminder.daysUntilService)
            );
            
            publishAlert("maintenance_overdue", message, "critical");
            m_lastAlertTimes[reminder.type] = std::chrono::system_clock::now();
        }
    }
    
    // Send alerts for upcoming services
    for (const auto& reminder : upcomingReminders) {
        if (canSendAlert(reminder.type)) {
            std::string severity = reminder.kmUntilService <= m_config.criticalAlertKm || 
                                 reminder.daysUntilService <= m_config.criticalAlertDays ? "warning" : "info";
            
            std::string message = fmt::format(
                "{} due in {:.0f} km or {} days",
                reminder.serviceName,
                reminder.kmUntilService,
                reminder.daysUntilService
            );
            
            publishAlert("maintenance_reminder", message, severity);
            m_lastAlertTimes[reminder.type] = std::chrono::system_clock::now();
        }
    }
    
    // Publish reminders
    publishReminders();
    publishSchedule();
}

ServiceReminder MaintenanceReminderApp::calculateServiceReminder(ServiceType serviceType) {
    const auto& interval = m_serviceIntervals[serviceType];
    ServiceReminder reminder(serviceType, interval.name, interval.description, interval.priority);
    
    // Get last service record
    const ServiceRecord* lastService = getLastServiceRecord(serviceType);
    
    if (lastService) {
        // Calculate based on last service
        double kmSinceService = m_currentDistanceKm - lastService->mileageAtService;
        auto daysSinceService = getDaysBetween(lastService->serviceDate, std::chrono::system_clock::now());
        
        reminder.kmUntilService = interval.distanceIntervalKm - kmSinceService;
        reminder.daysUntilService = interval.timeIntervalDays - daysSinceService;
        
        if (interval.engineHoursInterval > 0) {
            double hoursSinceService = m_currentEngineHours - lastService->engineHoursAtService;
            reminder.engineHoursUntilService = interval.engineHoursInterval - hoursSinceService;
        }
    } else {
        // No previous service - use vehicle totals
        reminder.kmUntilService = interval.distanceIntervalKm - m_currentDistanceKm;
        reminder.daysUntilService = interval.timeIntervalDays; // Assume due from beginning
        
        if (interval.engineHoursInterval > 0) {
            reminder.engineHoursUntilService = interval.engineHoursInterval - m_currentEngineHours;
        }
    }
    
    // Check if overdue
    reminder.overdue = (reminder.kmUntilService <= 0) || (reminder.daysUntilService <= 0);
    if (interval.engineHoursInterval > 0) {
        reminder.overdue = reminder.overdue || (reminder.engineHoursUntilService <= 0);
    }
    
    return reminder;
}

void MaintenanceReminderApp::recordServiceCompletion(ServiceType serviceType, const std::string& notes, 
                                                   double cost, const std::string& provider) {
    ServiceRecord record(serviceType, m_currentDistanceKm, notes);
    record.cost = cost;
    record.serviceProvider = provider;
    record.engineHoursAtService = m_currentEngineHours;
    
    m_serviceHistory.push_back(record);
    updateStatistics();
    
    std::string message = fmt::format(
        "{} completed at {:.0f} km{}",
        serviceTypeToString(serviceType),
        m_currentDistanceKm,
        cost > 0 ? fmt::format(" (${:.2f})", cost) : ""
    );
    
    publishAlert("service_completed", message, "info");
    velocitas::logger().info("Service recorded: {}", message);
}

const ServiceRecord* MaintenanceReminderApp::getLastServiceRecord(ServiceType serviceType) const {
    const ServiceRecord* lastRecord = nullptr;
    
    for (const auto& record : m_serviceHistory) {
        if (record.type == serviceType) {
            if (!lastRecord || record.serviceDate > lastRecord->serviceDate) {
                lastRecord = &record;
            }
        }
    }
    
    return lastRecord;
}

void MaintenanceReminderApp::publishStatus() {
    try {
        nlohmann::json statusJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"current_distance_km", m_currentDistanceKm},
            {"current_engine_hours", m_currentEngineHours},
            {"overdue_services", m_stats.overdueServices},
            {"upcoming_services", m_stats.upcomingServices},
            {"total_services_completed", m_stats.totalServices},
            {"total_maintenance_cost", m_stats.totalMaintenanceCost},
            {"config", {
                {"critical_alert_km", m_config.criticalAlertKm},
                {"warning_alert_km", m_config.warningAlertKm},
                {"info_alert_km", m_config.infoAlertKm},
                {"critical_alert_days", m_config.criticalAlertDays},
                {"warning_alert_days", m_config.warningAlertDays},
                {"info_alert_days", m_config.infoAlertDays},
                {"distance_alerts_enabled", m_config.enableDistanceAlerts},
                {"time_alerts_enabled", m_config.enableTimeAlerts},
                {"overdue_alerts_enabled", m_config.enableOverdueAlerts}
            }},
            {"status", "active"}
        };
        
        publishToTopic(TOPIC_STATUS, statusJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing status: {}", e.what());
    }
}

void MaintenanceReminderApp::publishReminders() {
    try {
        nlohmann::json remindersJson = nlohmann::json::array();
        
        for (const auto& [serviceType, interval] : m_serviceIntervals) {
            if (!interval.enabled) continue;
            
            ServiceReminder reminder = calculateServiceReminder(serviceType);
            
            nlohmann::json reminderJson = {
                {"service_type", serviceTypeToString(serviceType)},
                {"service_name", reminder.serviceName},
                {"description", reminder.description},
                {"km_until_service", reminder.kmUntilService},
                {"days_until_service", reminder.daysUntilService},
                {"engine_hours_until_service", reminder.engineHoursUntilService},
                {"priority", reminder.priority},
                {"overdue", reminder.overdue}
            };
            
            remindersJson.push_back(reminderJson);
        }
        
        nlohmann::json messageJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"reminders", remindersJson}
        };
        
        publishToTopic(TOPIC_REMINDERS, messageJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing reminders: {}", e.what());
    }
}

void MaintenanceReminderApp::publishSchedule() {
    try {
        nlohmann::json historyJson = nlohmann::json::array();
        
        for (const auto& record : m_serviceHistory) {
            nlohmann::json recordJson = {
                {"service_type", serviceTypeToString(record.type)},
                {"service_date", std::chrono::duration_cast<std::chrono::milliseconds>(
                    record.serviceDate.time_since_epoch()).count()},
                {"mileage_at_service", record.mileageAtService},
                {"engine_hours_at_service", record.engineHoursAtService},
                {"notes", record.notes},
                {"service_provider", record.serviceProvider},
                {"cost", record.cost}
            };
            
            historyJson.push_back(recordJson);
        }
        
        nlohmann::json messageJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"service_history", historyJson}
        };
        
        publishToTopic(TOPIC_SCHEDULE, messageJson.dump());
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing schedule: {}", e.what());
    }
}

void MaintenanceReminderApp::publishAlert(const std::string& alertType, const std::string& message, 
                                        const std::string& severity) {
    try {
        nlohmann::json alertJson = {
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"type", alertType},
            {"message", message},
            {"severity", severity},
            {"current_distance_km", m_currentDistanceKm},
            {"overdue_services", m_stats.overdueServices}
        };
        
        publishToTopic(TOPIC_ALERTS, alertJson.dump());
        velocitas::logger().info("Alert published: {} - {}", alertType, message);
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error publishing alert: {}", e.what());
    }
}

void MaintenanceReminderApp::updateStatistics() {
    m_stats.totalServices = static_cast<int>(m_serviceHistory.size());
    
    double totalCost = 0.0;
    std::chrono::system_clock::time_point lastDate;
    
    for (const auto& record : m_serviceHistory) {
        totalCost += record.cost;
        if (record.serviceDate > lastDate) {
            lastDate = record.serviceDate;
        }
    }
    
    m_stats.totalMaintenanceCost = totalCost;
    m_stats.lastServiceDate = lastDate;
    
    // Calculate average service interval
    if (m_serviceHistory.size() >= 2) {
        std::vector<double> intervals;
        
        for (size_t i = 1; i < m_serviceHistory.size(); ++i) {
            const auto& prev = m_serviceHistory[i-1];
            const auto& curr = m_serviceHistory[i];
            
            if (prev.type == curr.type) {
                double intervalKm = curr.mileageAtService - prev.mileageAtService;
                intervals.push_back(intervalKm);
            }
        }
        
        if (!intervals.empty()) {
            m_stats.averageServiceInterval = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
        }
    }
}

bool MaintenanceReminderApp::canSendAlert(ServiceType serviceType) {
    auto it = m_lastAlertTimes.find(serviceType);
    if (it == m_lastAlertTimes.end()) {
        return true;
    }
    
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    return timeSinceLastAlert.count() >= m_config.alertCooldownMs;
}

std::string MaintenanceReminderApp::serviceTypeToString(ServiceType type) {
    switch (type) {
        case ServiceType::OIL_CHANGE: return "OIL_CHANGE";
        case ServiceType::OIL_FILTER: return "OIL_FILTER";
        case ServiceType::AIR_FILTER: return "AIR_FILTER";
        case ServiceType::BRAKE_PADS: return "BRAKE_PADS";
        case ServiceType::BRAKE_FLUID: return "BRAKE_FLUID";
        case ServiceType::TRANSMISSION_FLUID: return "TRANSMISSION_FLUID";
        case ServiceType::COOLANT: return "COOLANT";
        case ServiceType::SPARK_PLUGS: return "SPARK_PLUGS";
        case ServiceType::TIMING_BELT: return "TIMING_BELT";
        case ServiceType::TIRE_ROTATION: return "TIRE_ROTATION";
        case ServiceType::TIRE_REPLACEMENT: return "TIRE_REPLACEMENT";
        case ServiceType::BATTERY: return "BATTERY";
        case ServiceType::GENERAL_INSPECTION: return "GENERAL_INSPECTION";
        default: return "UNKNOWN";
    }
}

ServiceType MaintenanceReminderApp::stringToServiceType(const std::string& typeStr) {
    if (typeStr == "OIL_CHANGE") return ServiceType::OIL_CHANGE;
    if (typeStr == "OIL_FILTER") return ServiceType::OIL_FILTER;
    if (typeStr == "AIR_FILTER") return ServiceType::AIR_FILTER;
    if (typeStr == "BRAKE_PADS") return ServiceType::BRAKE_PADS;
    if (typeStr == "BRAKE_FLUID") return ServiceType::BRAKE_FLUID;
    if (typeStr == "TRANSMISSION_FLUID") return ServiceType::TRANSMISSION_FLUID;
    if (typeStr == "COOLANT") return ServiceType::COOLANT;
    if (typeStr == "SPARK_PLUGS") return ServiceType::SPARK_PLUGS;
    if (typeStr == "TIMING_BELT") return ServiceType::TIMING_BELT;
    if (typeStr == "TIRE_ROTATION") return ServiceType::TIRE_ROTATION;
    if (typeStr == "TIRE_REPLACEMENT") return ServiceType::TIRE_REPLACEMENT;
    if (typeStr == "BATTERY") return ServiceType::BATTERY;
    if (typeStr == "GENERAL_INSPECTION") return ServiceType::GENERAL_INSPECTION;
    return ServiceType::OIL_CHANGE; // Default fallback
}

bool MaintenanceReminderApp::parseConfig(const std::string& configJson) {
    try {
        auto json = nlohmann::json::parse(configJson);
        
        if (json.contains("critical_alert_km")) {
            m_config.criticalAlertKm = json["critical_alert_km"].get<double>();
        }
        if (json.contains("warning_alert_km")) {
            m_config.warningAlertKm = json["warning_alert_km"].get<double>();
        }
        if (json.contains("info_alert_km")) {
            m_config.infoAlertKm = json["info_alert_km"].get<double>();
        }
        if (json.contains("critical_alert_days")) {
            m_config.criticalAlertDays = json["critical_alert_days"].get<int>();
        }
        if (json.contains("warning_alert_days")) {
            m_config.warningAlertDays = json["warning_alert_days"].get<int>();
        }
        if (json.contains("info_alert_days")) {
            m_config.infoAlertDays = json["info_alert_days"].get<int>();
        }
        if (json.contains("alert_cooldown_ms")) {
            m_config.alertCooldownMs = json["alert_cooldown_ms"].get<int>();
        }
        if (json.contains("enable_distance_alerts")) {
            m_config.enableDistanceAlerts = json["enable_distance_alerts"].get<bool>();
        }
        if (json.contains("enable_time_alerts")) {
            m_config.enableTimeAlerts = json["enable_time_alerts"].get<bool>();
        }
        if (json.contains("enable_overdue_alerts")) {
            m_config.enableOverdueAlerts = json["enable_overdue_alerts"].get<bool>();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        velocitas::logger().error("Error parsing configuration JSON: {}", e.what());
        return false;
    }
}

int MaintenanceReminderApp::getDaysBetween(const std::chrono::system_clock::time_point& from,
                                         const std::chrono::system_clock::time_point& to) {
    auto duration = std::chrono::duration_cast<std::chrono::hours>(to - from);
    return static_cast<int>(duration.count() / 24);
}

} // namespace maintenance