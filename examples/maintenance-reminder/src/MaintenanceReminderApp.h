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

#ifndef MAINTENANCEREMINDERAPP_H
#define MAINTENANCEREMINDERAPP_H

#include "sdk/VehicleApp.h"
#include "sdk/DataPointReply.h"
#include "sdk/Status.h"
#include "vehicle_model/Vehicle.hpp"

#include <chrono>
#include <memory>
#include <map>
#include <vector>

namespace maintenance {

/**
 * @brief Types of maintenance services
 */
enum class ServiceType {
    OIL_CHANGE,
    OIL_FILTER,
    AIR_FILTER,
    BRAKE_PADS,
    BRAKE_FLUID,
    TRANSMISSION_FLUID,
    COOLANT,
    SPARK_PLUGS,
    TIMING_BELT,
    TIRE_ROTATION,
    TIRE_REPLACEMENT,
    BATTERY,
    GENERAL_INSPECTION
};

/**
 * @brief Service intervals and configuration
 */
struct ServiceInterval {
    ServiceType type;
    std::string name;
    std::string description;
    double distanceIntervalKm = 0.0;        // Distance-based interval
    int timeIntervalDays = 0;               // Time-based interval
    double engineHoursInterval = 0.0;       // Engine hours-based interval
    int priority = 1;                       // 1=critical, 2=important, 3=routine
    bool enabled = true;
    
    ServiceInterval(ServiceType t, const std::string& n, const std::string& desc, 
                   double distKm, int timeDays, int prio = 2)
        : type(t), name(n), description(desc), distanceIntervalKm(distKm), 
          timeIntervalDays(timeDays), priority(prio) {}
};

/**
 * @brief Service record for tracking completed maintenance
 */
struct ServiceRecord {
    ServiceType type;
    std::chrono::system_clock::time_point serviceDate;
    double mileageAtService = 0.0;          // km
    double engineHoursAtService = 0.0;      // hours
    std::string notes;
    std::string serviceProvider;
    double cost = 0.0;
    
    ServiceRecord(ServiceType t, double mileage, const std::string& note = "")
        : type(t), serviceDate(std::chrono::system_clock::now()), 
          mileageAtService(mileage), notes(note) {}
};

/**
 * @brief Upcoming service reminder
 */
struct ServiceReminder {
    ServiceType type;
    std::string serviceName;
    std::string description;
    double kmUntilService = 0.0;
    int daysUntilService = 0;
    double engineHoursUntilService = 0.0;
    int priority = 2;
    bool overdue = false;
    
    ServiceReminder(ServiceType t, const std::string& name, const std::string& desc, int prio)
        : type(t), serviceName(name), description(desc), priority(prio) {}
};

/**
 * @brief Configuration for maintenance reminders
 */
struct MaintenanceConfig {
    // Alert thresholds (distance before service due)
    double criticalAlertKm = 500.0;         // Critical alert at 500km before service
    double warningAlertKm = 1000.0;         // Warning alert at 1000km before service
    double infoAlertKm = 2000.0;            // Info alert at 2000km before service
    
    // Alert thresholds (time before service due)
    int criticalAlertDays = 7;              // Critical alert 7 days before
    int warningAlertDays = 30;              // Warning alert 30 days before
    int infoAlertDays = 60;                 // Info alert 60 days before
    
    // Alert frequency
    int dailyReminderHour = 9;              // Send daily reminders at 9 AM
    int alertCooldownMs = 3600000;          // 1 hour cooldown between same alerts
    
    // Features
    bool enableDistanceAlerts = true;
    bool enableTimeAlerts = true;
    bool enableDailyReminders = true;
    bool enableOverdueAlerts = true;
};

/**
 * @brief Maintenance statistics
 */
struct MaintenanceStats {
    int totalServices = 0;
    double totalMaintenanceCost = 0.0;
    int overdueServices = 0;
    int upcomingServices = 0;
    std::chrono::system_clock::time_point lastServiceDate;
    double averageServiceInterval = 0.0;    // km
    
    MaintenanceStats() : lastServiceDate(std::chrono::system_clock::now()) {}
};

/**
 * @brief Maintenance Reminder Vehicle Application
 * 
 * This application provides comprehensive vehicle maintenance tracking:
 * - Tracks multiple service intervals (distance, time, engine hours)
 * - Sends proactive maintenance reminders
 * - Maintains service history and costs
 * - Provides overdue service alerts
 * - Supports configurable service schedules
 * - Calculates maintenance statistics
 */
class MaintenanceReminderApp : public velocitas::VehicleApp {
public:
    MaintenanceReminderApp();

protected:
    /**
     * @brief Called when the application starts and VDB connection is ready
     */
    void onStart() override;

private:
    /**
     * @brief Handles distance changes
     * @param reply DataPoint reply containing distance data
     */
    void onDistanceChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles engine hours changes
     * @param reply DataPoint reply containing engine hours data
     */
    void onEngineHoursChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles service distance changes
     * @param reply DataPoint reply containing service distance data
     */
    void onServiceDistanceChanged(const velocitas::DataPointReply& reply);

    /**
     * @brief Handles configuration updates via MQTT
     * @param data JSON configuration data
     */
    void onConfigReceived(const std::string& data);

    /**
     * @brief Handles service completion notifications
     * @param data Service completion data
     */
    void onServiceCompletedReceived(const std::string& data);

    /**
     * @brief Handles service scheduling requests
     * @param data Service scheduling data
     */
    void onServiceScheduledReceived(const std::string& data);

    /**
     * @brief Handles reset commands
     * @param data Reset command data
     */
    void onResetReceived(const std::string& data);

    /**
     * @brief Handles errors from vehicle data subscriptions
     * @param status Error status
     */
    void onError(const velocitas::Status& status);

    /**
     * @brief Initializes default service intervals
     */
    void initializeServiceIntervals();

    /**
     * @brief Checks all services for upcoming maintenance
     */
    void checkMaintenanceReminders();

    /**
     * @brief Calculates when next service is due for a specific type
     * @param serviceType Type of service to check
     * @return ServiceReminder object with calculated values
     */
    ServiceReminder calculateServiceReminder(ServiceType serviceType);

    /**
     * @brief Records a completed service
     * @param serviceType Type of service completed
     * @param notes Optional notes about the service
     * @param cost Optional cost of the service
     * @param provider Optional service provider name
     */
    void recordServiceCompletion(ServiceType serviceType, const std::string& notes = "", 
                               double cost = 0.0, const std::string& provider = "");

    /**
     * @brief Gets the last service record for a specific type
     * @param serviceType Type of service to look up
     * @return Pointer to service record, or nullptr if not found
     */
    const ServiceRecord* getLastServiceRecord(ServiceType serviceType) const;

    /**
     * @brief Publishes current maintenance status
     */
    void publishStatus();

    /**
     * @brief Publishes maintenance reminders
     */
    void publishReminders();

    /**
     * @brief Publishes maintenance schedule
     */
    void publishSchedule();

    /**
     * @brief Publishes an alert
     * @param alertType Type of alert
     * @param message Alert message
     * @param severity Alert severity
     */
    void publishAlert(const std::string& alertType, const std::string& message, 
                     const std::string& severity = "info");

    /**
     * @brief Updates maintenance statistics
     */
    void updateStatistics();

    /**
     * @brief Checks if enough time has passed since last alert
     * @param serviceType Service type for cooldown tracking
     * @return true if alert can be sent
     */
    bool canSendAlert(ServiceType serviceType);

    /**
     * @brief Converts ServiceType enum to string
     * @param type Service type
     * @return String representation
     */
    static std::string serviceTypeToString(ServiceType type);

    /**
     * @brief Converts string to ServiceType enum
     * @param typeStr String representation
     * @return ServiceType enum value
     */
    static ServiceType stringToServiceType(const std::string& typeStr);

    /**
     * @brief Parses configuration JSON
     * @param configJson JSON configuration string
     * @return true if parsing successful
     */
    bool parseConfig(const std::string& configJson);

    /**
     * @brief Gets days between two time points
     * @param from Start time
     * @param to End time
     * @return Number of days
     */
    static int getDaysBetween(const std::chrono::system_clock::time_point& from,
                             const std::chrono::system_clock::time_point& to);

private:
    // Configuration
    MaintenanceConfig m_config;
    
    // Service intervals and records
    std::map<ServiceType, ServiceInterval> m_serviceIntervals;
    std::vector<ServiceRecord> m_serviceHistory;
    std::map<ServiceType, std::chrono::system_clock::time_point> m_lastAlertTimes;
    
    // Current vehicle data
    double m_currentDistanceKm = 0.0;
    double m_currentEngineHours = 0.0;
    double m_serviceDistanceKm = 0.0;
    
    // Statistics
    MaintenanceStats m_stats;
    
    // State tracking
    std::chrono::system_clock::time_point m_lastReminderCheck;
    
    // MQTT topic constants
    static constexpr const char* TOPIC_CONFIG = "maintenance/config";
    static constexpr const char* TOPIC_SERVICE_COMPLETED = "maintenance/service/completed";
    static constexpr const char* TOPIC_SERVICE_SCHEDULED = "maintenance/service/scheduled";
    static constexpr const char* TOPIC_RESET = "maintenance/reset";
    static constexpr const char* TOPIC_STATUS = "maintenance/status";
    static constexpr const char* TOPIC_REMINDERS = "maintenance/reminders";
    static constexpr const char* TOPIC_ALERTS = "maintenance/alerts";
    static constexpr const char* TOPIC_SCHEDULE = "maintenance/schedule";
};

} // namespace maintenance

#endif // MAINTENANCEREMINDERAPP_H