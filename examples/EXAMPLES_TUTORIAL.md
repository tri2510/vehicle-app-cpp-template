# Vehicle App Examples - Complete Tutorial Guide

This comprehensive guide provides detailed walkthroughs for all 5 example vehicle applications included in this repository. Each example demonstrates different aspects of vehicle app development using the Eclipse Velocitas framework.

## 📋 Table of Contents

1. [Speed Monitor & Alert System](#1-speed-monitor--alert-system)
2. [Fuel Efficiency Tracker](#2-fuel-efficiency-tracker)
3. [Maintenance Reminder System](#3-maintenance-reminder-system)
4. [Parking Assistant](#4-parking-assistant)
5. [Climate Control Optimizer](#5-climate-control-optimizer)
6. [Testing Your Applications](#testing-your-applications)
7. [Common Development Patterns](#common-development-patterns)

## Prerequisites

Before starting with any example, ensure you have:

```bash
# 1. Docker development environment running
docker run -it --privileged -v $(pwd):/workspace \
  -p 8080:8080 -p 1883:1883 -p 55555:55555 velocitas-dev

## 1.1. With proxy
docker run -it --privileged \
    -e HTTP_PROXY=http://127.0.0.1:3128 \
    -e HTTPS_PROXY=http://127.0.0.1:3128 \
    -e http_proxy=http://127.0.0.1:3128 \
    -e https_proxy=http://127.0.0.1:3128 \
    -v $(pwd):/workspace \
    --network=host \
    velocitas-dev


# 2. Dependencies installed
install-deps

# 3. Runtime services started
runtime-up
```

---

## 1. Speed Monitor & Alert System

### Overview
The Speed Monitor app demonstrates real-time vehicle data processing, configurable alerts, and MQTT-based communication.

### Key Features
- Real-time speed monitoring with configurable limits
- Hard braking and rapid acceleration detection
- Configurable alert thresholds via MQTT
- Speed statistics tracking
- Alert cooldown to prevent spam

### Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Vehicle Data  │───→│  Speed Monitor   │───→│  MQTT Alerts    │
│     Broker      │    │      App         │    │   & Status      │
└─────────────────┘    └──────────────────┘    └─────────────────┘
      │                          │
      │                          ▼
      ▼                ┌──────────────────┐
┌─────────────────┐    │   Configuration  │
│  Acceleration   │    │    via MQTT      │
│     Data        │    └──────────────────┘
└─────────────────┘
```

### Step-by-Step Implementation

#### Step 1: Core Structure
```cpp
// SpeedMonitorApp.h - Main class structure
class SpeedMonitorApp : public velocitas::VehicleApp {
private:
    SpeedConfig m_config;           // Configuration settings
    SpeedStatistics m_stats;        // Runtime statistics
    double m_currentSpeedKmh;       // Current speed in km/h
    std::chrono::system_clock::time_point m_lastAlertTime;
};
```

#### Step 2: Vehicle Data Subscription
```cpp
void SpeedMonitorApp::onStart() {
    // Subscribe to vehicle speed changes
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { onError(std::forward<decltype(status)>(status)); });
        
    // Subscribe to acceleration data
    subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Acceleration.Longitudinal).build())
        ->onItem([this](auto&& item) { onAccelerationChanged(std::forward<decltype(item)>(item)); })
        ->onError([this](auto&& status) { /* Handle optional data */ });
}
```

#### Step 3: Speed Processing
```cpp
void SpeedMonitorApp::onSpeedChanged(const velocitas::DataPointReply& reply) {
    auto speedValue = reply.get(Vehicle.Speed)->value();
    m_currentSpeedKmh = msToKmh(speedValue);  // Convert m/s to km/h
    
    updateStatistics(m_currentSpeedKmh);       // Update running statistics
    
    if (m_config.enableSpeedLimitAlerts) {
        checkSpeedLimit(m_currentSpeedKmh);    // Check for violations
    }
}
```

#### Step 4: Alert Logic
```cpp
void SpeedMonitorApp::checkSpeedLimit(double currentSpeed) {
    if (currentSpeed > m_config.speedLimitKmh && canSendAlert()) {
        m_stats.speedLimitViolations++;
        
        std::string message = fmt::format(
            "Speed limit exceeded: {:.1f} km/h (limit: {:.1f} km/h)",
            currentSpeed, m_config.speedLimitKmh
        );
        
        publishAlert("speed_limit", message, "warning");
        m_lastAlertTime = std::chrono::system_clock::now();
    }
}
```

### Testing the Speed Monitor

#### Test 1: Basic Functionality
```bash
# Build and run the app
build-app
run-app &

# Send speed data via MQTT (simulate vehicle data)
docker run --rm --network host eclipse-mosquitto:2.0 mosquitto_pub \
  -h localhost -t "Vehicle/Speed" -m '25.0'  # 25 m/s = 90 km/h
```

#### Test 2: Configuration Update
```bash
# Update speed limit configuration
mosquitto_pub -h localhost -t "speedmonitor/config" -m '{
  "speed_limit_kmh": 60.0,
  "enable_speed_limit_alerts": true,
  "alert_cooldown_ms": 3000
}'
```

#### Test 3: Monitor Alerts
```bash
# Subscribe to alerts
mosquitto_sub -h localhost -t "speedmonitor/alerts"

# Subscribe to status updates  
mosquitto_sub -h localhost -t "speedmonitor/status"
```

### Expected Output
```json
// Alert message
{
  "timestamp": 1704067200000,
  "type": "speed_limit",
  "message": "Speed limit exceeded: 95.0 km/h (limit: 80.0 km/h)",
  "severity": "warning",
  "current_speed": 95.0,
  "current_acceleration": 0.0
}
```

---

## 2. Fuel Efficiency Tracker

### Overview
The Fuel Efficiency Tracker monitors fuel consumption patterns, calculates efficiency metrics, and provides eco-driving recommendations.

### Key Features
- Real-time fuel efficiency calculation (km/L and L/100km)
- Trip-based fuel tracking with start/stop commands
- Rolling efficiency averages with configurable windows
- Eco-driving tips based on driving patterns
- Fuel consumption alerts and optimization suggestions

### Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│ Fuel Consumption│───→│ Efficiency       │───→│ Trip Tracking   │
│    Data         │    │  Calculator      │    │   & Statistics  │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │
┌─────────────────┐             │                ┌─────────────────┐
│  Speed Data     │─────────────┼───────────────→│  Eco Tips       │
└─────────────────┘             │                │  Generator      │
                                │                └─────────────────┘
┌─────────────────┐             ▼
│ Distance Data   │    ┌──────────────────┐
└─────────────────┘───→│ MQTT Alerts &    │
                       │ Status Updates   │
                       └──────────────────┘
```

### Step-by-Step Implementation

#### Step 1: Efficiency Calculation Core
```cpp
void FuelEfficiencyApp::calculateEfficiency() {
    // Add data point to rolling window
    if (m_currentSpeed > 0.1 && m_currentConsumption > 0.001) {
        m_efficiencyData.emplace_back(m_currentConsumption, m_currentSpeed, m_currentDistance);
        
        // Limit window size for memory management
        while (m_efficiencyData.size() > static_cast<size_t>(m_config.efficiencyWindowSize)) {
            m_efficiencyData.pop_front();
        }
        
        updateRollingEfficiency();
    }
}
```

#### Step 2: Rolling Efficiency Algorithm
```cpp
void FuelEfficiencyApp::updateRollingEfficiency() {
    double totalDistance = 0.0;
    double totalFuelUsed = 0.0;
    
    // Calculate over sliding window
    for (size_t i = 1; i < m_efficiencyData.size(); ++i) {
        const auto& prev = m_efficiencyData[i-1];
        const auto& curr = m_efficiencyData[i];
        
        // Time difference in hours
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(curr.timestamp - prev.timestamp);
        double timeHours = timeDiff.count() / (1000.0 * 3600.0);
        
        // Distance and fuel calculations
        double avgSpeed = (prev.speed + curr.speed) / 2.0;
        double distanceTraveled = avgSpeed * timeHours;
        
        double avgConsumption = (prev.instantConsumption + curr.instantConsumption) / 2.0;
        double fuelUsed = avgConsumption * timeHours;
        
        totalDistance += distanceTraveled;
        totalFuelUsed += fuelUsed;
    }
    
    // Calculate efficiency
    if (totalFuelUsed > 0.001 && totalDistance > 0.001) {
        m_stats.currentEfficiencyKmL = totalDistance / totalFuelUsed;
    }
}
```

#### Step 3: Trip Management
```cpp
void FuelEfficiencyApp::startTrip(const std::string& tripName) {
    if (m_currentTrip.isActive) {
        endTrip(); // End current trip first
    }
    
    m_currentTrip = TripData();
    m_currentTrip.isActive = true;
    m_currentTrip.startTime = std::chrono::system_clock::now();
    m_currentTrip.startDistance = m_currentDistance;
    
    publishAlert("trip_started", fmt::format("Trip started: {}", tripName), "info");
}
```

#### Step 4: Eco-Driving Tips
```cpp
std::vector<std::string> FuelEfficiencyApp::generateEcoTips() {
    std::vector<std::string> tips;
    
    // Speed-based recommendations
    if (m_currentSpeed > 100) {
        tips.push_back("Reduce speed - driving over 100 km/h significantly increases fuel consumption");
    }
    
    // RPM-based recommendations
    if (m_currentRpm > 3000) {
        tips.push_back("Shift to higher gear - high RPM increases fuel consumption");
    }
    
    // Efficiency-based recommendations
    if (m_stats.currentEfficiencyKmL < m_config.targetEfficiencyKmL) {
        tips.push_back("Maintain steady acceleration and avoid aggressive driving");
    }
    
    return tips;
}
```

### Testing the Fuel Efficiency Tracker

#### Test 1: Start Trip and Monitor Efficiency
```bash
# Start a new trip
mosquitto_pub -h localhost -t "fueltracker/trip/start" -m '{"name": "Morning Commute"}'

# Monitor efficiency updates
mosquitto_sub -h localhost -t "fueltracker/efficiency"

# Simulate fuel consumption data
mosquitto_pub -h localhost -t "Vehicle/Powertrain/FuelSystem/InstantConsumption" -m '8.5'  # L/h
mosquitto_pub -h localhost -t "Vehicle/Speed" -m '22.2'  # 80 km/h in m/s
```

#### Test 2: Configuration and Alerts
```bash
# Configure efficiency targets
mosquitto_pub -h localhost -t "fueltracker/config" -m '{
  "target_efficiency_kmL": 18.0,
  "low_efficiency_threshold": 10.0,
  "enable_eco_tips": true
}'

# Monitor alerts and tips
mosquitto_sub -h localhost -t "fueltracker/alerts"
```

#### Test 3: Trip Summary
```bash
# End trip and view summary
mosquitto_pub -h localhost -t "fueltracker/trip/end" -m '{}'

# View trip summary
mosquitto_sub -h localhost -t "fueltracker/trip/summary"
```

### Expected Output
```json
// Efficiency data
{
  "timestamp": 1704067200000,
  "current_efficiency_kmL": 15.2,
  "current_consumption_L100km": 6.6,
  "average_efficiency_kmL": 14.8,
  "best_efficiency_kmL": 18.1,
  "instant_consumption_Lh": 8.5,
  "current_speed_kmh": 80.0,
  "target_efficiency_kmL": 18.0
}
```

---

## 3. Maintenance Reminder System

### Overview
The Maintenance Reminder System provides proactive vehicle maintenance tracking based on mileage, time intervals, and engine hours.

### Key Features
- Multiple service interval tracking (oil change, filters, brakes, etc.)
- Distance, time, and engine hour-based reminders
- Service history and cost tracking
- Overdue service alerts with priority levels
- Configurable reminder thresholds

### Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│  Vehicle Data   │───→│  Service         │───→│  Reminder       │
│ (Distance/Time) │    │  Calculators     │    │  Generator      │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │                        │
┌─────────────────┐             │                        ▼
│ Service History │◄────────────┼────────────┐  ┌─────────────────┐
│   Database      │             │            │  │  MQTT Alerts    │
└─────────────────┘             ▼            │  │  & Reminders    │
                       ┌──────────────────┐   │  └─────────────────┘
                       │  Service Config  │   │
                       │   & Intervals    │───┘
                       └──────────────────┘
```

### Step-by-Step Implementation

#### Step 1: Service Intervals Definition
```cpp
void MaintenanceReminderApp::initializeServiceIntervals() {
    // Define default service intervals
    m_serviceIntervals[ServiceType::OIL_CHANGE] = 
        ServiceInterval(ServiceType::OIL_CHANGE, "Oil Change", 
                       "Replace engine oil and oil filter", 10000.0, 365, 1);
    
    m_serviceIntervals[ServiceType::BRAKE_PADS] = 
        ServiceInterval(ServiceType::BRAKE_PADS, "Brake Pad Inspection", 
                       "Check brake pad thickness and condition", 30000.0, 730, 1);
    
    m_serviceIntervals[ServiceType::AIR_FILTER] = 
        ServiceInterval(ServiceType::AIR_FILTER, "Air Filter Replacement", 
                       "Replace engine air filter", 20000.0, 365, 2);
    
    // Add more services...
}
```

#### Step 2: Service Reminder Calculation
```cpp
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
    } else {
        // No previous service - use vehicle totals
        reminder.kmUntilService = interval.distanceIntervalKm - m_currentDistanceKm;
        reminder.daysUntilService = interval.timeIntervalDays;
    }
    
    // Check if overdue
    reminder.overdue = (reminder.kmUntilService <= 0) || (reminder.daysUntilService <= 0);
    
    return reminder;
}
```

#### Step 3: Alert Generation
```cpp
void MaintenanceReminderApp::checkMaintenanceReminders() {
    for (const auto& [serviceType, interval] : m_serviceIntervals) {
        if (!interval.enabled) continue;
        
        ServiceReminder reminder = calculateServiceReminder(serviceType);
        
        // Determine alert level
        std::string severity = "info";
        bool shouldAlert = false;
        
        if (reminder.overdue) {
            severity = "critical";
            shouldAlert = true;
        } else if (reminder.kmUntilService <= m_config.criticalAlertKm || 
                  reminder.daysUntilService <= m_config.criticalAlertDays) {
            severity = "warning";
            shouldAlert = true;
        } else if (reminder.kmUntilService <= m_config.warningAlertKm || 
                  reminder.daysUntilService <= m_config.warningAlertDays) {
            severity = "info";
            shouldAlert = true;
        }
        
        if (shouldAlert && canSendAlert(serviceType)) {
            std::string message = fmt::format(
                "{} {} in {:.0f} km or {} days",
                reminder.overdue ? "OVERDUE:" : "Due:",
                reminder.serviceName,
                std::abs(reminder.kmUntilService),
                std::abs(reminder.daysUntilService)
            );
            
            publishAlert("maintenance_reminder", message, severity);
            m_lastAlertTimes[serviceType] = std::chrono::system_clock::now();
        }
    }
}
```

#### Step 4: Service Completion
```cpp
void MaintenanceReminderApp::recordServiceCompletion(ServiceType serviceType, 
                                                   const std::string& notes, 
                                                   double cost, 
                                                   const std::string& provider) {
    ServiceRecord record(serviceType, m_currentDistanceKm, notes);
    record.cost = cost;
    record.serviceProvider = provider;
    
    m_serviceHistory.push_back(record);
    updateStatistics();
    
    std::string message = fmt::format(
        "{} completed at {:.0f} km{}",
        serviceTypeToString(serviceType),
        m_currentDistanceKm,
        cost > 0 ? fmt::format(" (${:.2f})", cost) : ""
    );
    
    publishAlert("service_completed", message, "info");
}
```

### Testing the Maintenance Reminder

#### Test 1: Check Current Maintenance Status
```bash
# Subscribe to maintenance status
mosquitto_sub -h localhost -t "maintenance/status"

# Subscribe to reminders
mosquitto_sub -h localhost -t "maintenance/reminders"

# Simulate distance increase
mosquitto_pub -h localhost -t "Vehicle/TraveledDistance" -m '150000'  # 150,000 meters = 150 km
```

#### Test 2: Complete a Service
```bash
# Record oil change completion
mosquitto_pub -h localhost -t "maintenance/service/completed" -m '{
  "service_type": "OIL_CHANGE",
  "notes": "Full synthetic oil change",
  "cost": 75.50,
  "provider": "Quick Lube Shop"
}'
```

#### Test 3: Configure Alert Thresholds
```bash
# Update alert configuration
mosquitto_pub -h localhost -t "maintenance/config" -m '{
  "critical_alert_km": 200.0,
  "warning_alert_km": 500.0,
  "enable_distance_alerts": true,
  "enable_overdue_alerts": true
}'
```

### Expected Output
```json
// Maintenance reminder
{
  "timestamp": 1704067200000,
  "type": "maintenance_reminder",
  "message": "Oil Change due in 1200 km or 45 days",
  "severity": "info",
  "service_type": "OIL_CHANGE",
  "km_until_service": 1200.0,
  "days_until_service": 45
}
```

---

## 4. Parking Assistant

### Overview
The Parking Assistant uses ultrasonic sensors and camera data to help with parking maneuvers, providing distance alerts and guidance.

### Key Features
- Multi-sensor distance monitoring (front, rear, sides)
- Progressive audio/visual alerts based on proximity
- Parking space detection and guidance
- Collision warning system
- Support for parallel and perpendicular parking

### Quick Implementation
```cpp
// Key components for parking assistance
class ParkingAssistantApp : public velocitas::VehicleApp {
private:
    struct SensorData {
        double frontDistance = 999.0;    // cm
        double rearDistance = 999.0;     // cm
        double leftDistance = 999.0;     // cm
        double rightDistance = 999.0;    // cm
    } m_sensors;
    
    void checkProximityAlerts() {
        if (m_sensors.rearDistance < 30.0) {
            publishAlert("collision_warning", "STOP! Object very close behind", "critical");
        } else if (m_sensors.rearDistance < 100.0) {
            publishAlert("proximity_warning", fmt::format("Object {:.0f}cm behind", m_sensors.rearDistance), "warning");
        }
    }
};
```

---

## 5. Climate Control Optimizer

### Overview
The Climate Control Optimizer monitors cabin temperature, external conditions, and energy consumption to provide efficient climate control recommendations.

### Key Features
- Automatic temperature optimization based on occupancy
- Energy-efficient climate control recommendations
- Integration with vehicle energy management
- Predictive climate adjustment based on route and weather
- Personalized comfort preferences

### Quick Implementation
```cpp
// Key components for climate optimization
class ClimateControlApp : public velocitas::VehicleApp {
private:
    struct ClimateData {
        double cabinTemp = 20.0;         // °C
        double outsideTemp = 20.0;       // °C
        double targetTemp = 22.0;        // °C
        double humidity = 50.0;          // %
        bool occupancyDetected = false;
    } m_climate;
    
    void optimizeClimate() {
        // Calculate optimal settings based on conditions
        if (m_climate.occupancyDetected) {
            adjustTemperatureForComfort();
        } else {
            enableEcoMode();
        }
    }
};
```

---

## Testing Your Applications

### Unit Testing Framework

Create comprehensive unit tests for each application:

```cpp
// Example test structure
class SpeedMonitorAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    SpeedMonitorApp app;
};

TEST_F(SpeedMonitorAppTest, SpeedLimitViolationDetection) {
    // Test speed limit violation logic
    EXPECT_TRUE(app.isSpeedLimitViolated(90.0, 80.0));
    EXPECT_FALSE(app.isSpeedLimitViolated(75.0, 80.0));
}
```

### Integration Testing

Test complete workflows in the Docker environment:

```bash
# Build all examples
build-app

# Run tests
./build/bin/app_utests

# Start runtime services for integration testing
runtime-up

# Test with real MQTT and VDB
run-app &
sleep 5

# Send test data and verify responses
mosquitto_pub -h localhost -t "test/speed" -m '25.0'
mosquitto_sub -h localhost -t "speedmonitor/alerts" -C 1
```

### Performance Testing

Monitor resource usage and response times:

```bash
# Monitor memory and CPU usage
docker stats

# Test message throughput
for i in {1..100}; do
  mosquitto_pub -h localhost -t "Vehicle/Speed" -m "$((RANDOM % 40 + 10))"
  sleep 0.1
done
```

---

## Common Development Patterns

### 1. Vehicle Data Subscription Pattern
```cpp
// Standard pattern for subscribing to vehicle data
subscribeDataPoints(velocitas::QueryBuilder::select(Vehicle.Signal.Path).build())
    ->onItem([this](auto&& item) { 
        onDataChanged(std::forward<decltype(item)>(item)); 
    })
    ->onError([this](auto&& status) { 
        handleError(std::forward<decltype(status)>(status)); 
    });
```

### 2. Configuration Management Pattern
```cpp
// Standard configuration update handling
void onConfigReceived(const std::string& data) {
    try {
        auto config = nlohmann::json::parse(data);
        updateConfiguration(config);
        publishStatus();  // Confirm configuration update
    } catch (const std::exception& e) {
        publishAlert("config_error", e.what(), "error");
    }
}
```

### 3. Alert Cooldown Pattern
```cpp
// Prevent alert spam with cooldown logic
bool canSendAlert() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastAlert = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastAlertTime);
    return timeSinceLastAlert.count() >= m_config.alertCooldownMs;
}
```

### 4. Statistics Tracking Pattern
```cpp
// Rolling statistics with bounded memory
void updateStatistics(double newValue) {
    m_samples.push_back(newValue);
    
    // Limit sample size
    if (m_samples.size() > MAX_SAMPLES) {
        m_samples.erase(m_samples.begin());
    }
    
    // Calculate rolling average
    m_average = std::accumulate(m_samples.begin(), m_samples.end(), 0.0) / m_samples.size();
}
```

### 5. Error Handling Pattern
```cpp
// Consistent error handling across all apps
void onError(const velocitas::Status& status) {
    velocitas::logger().error("App error: {}", status.errorMessage());
    publishAlert("system_error", status.errorMessage(), "error");
    
    // Attempt recovery if possible
    if (status.isRecoverable()) {
        scheduleRetry();
    }
}
```

---

## Next Steps

1. **Choose an Example**: Start with the Speed Monitor for basic concepts
2. **Modify and Extend**: Add your own features and logic
3. **Test Thoroughly**: Use the provided test patterns
4. **Deploy**: Build production-ready containers
5. **Monitor**: Use the MQTT topics for real-time monitoring

Each example builds upon the previous concepts, providing a comprehensive learning path for vehicle application development with Eclipse Velocitas.

For detailed API documentation, see the [Velocitas Documentation](https://eclipse-velocitas.github.io/velocitas-docs/).

---

**Happy Vehicle App Development! 🚗💨**