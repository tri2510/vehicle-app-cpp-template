# Vehicle App Development Guide

## Quick Start (5 Minutes)

### 1. Setup Environment
```bash
git clone <vehicle-app-cpp-template>
cd vehicle-app-cpp-template
code .  # Opens VSCode
```
- Click **"Reopen in Container"** when prompted
- Wait 5 minutes for devcontainer setup

### 2. Start Runtime Services
- Press `Ctrl+Shift+P` → **"Tasks: Run Task"**
- Select **"Local Runtime - Up"**
- Wait for: `✅ Runtime is ready to use!`

### 3. Build & Run
```bash
./build.sh                    # Build app
# Then: Tasks → "Local Runtime - Run VehicleApp"
```

## Understanding the Sample App

### Core Structure
```
app/
├── AppManifest.json         # What vehicle data you need
├── src/
    ├── SampleApp.h          # App interface
    ├── SampleApp.cpp        # Main logic
    └── Launcher.cpp         # Entry point
```

### AppManifest.json - "What I Need"
```json
{
    "name": "SampleApp",
    "interfaces": [
        {
            "type": "vehicle-signal-interface",
            "datapoints": { 
                "path": "Vehicle.Speed",    // I need speed data
                "access": "read"            // Read-only
            }
        },
        {
            "type": "pubsub",
            "reads": ["sampleapp/getSpeed"],        // I listen to these topics
            "writes": ["sampleapp/currentSpeed"]    // I publish to these topics
        }
    ]
}
```

### SampleApp.cpp - Main Logic

#### 1. Constructor - Connect to Services
```cpp
SampleApp::SampleApp()
    : VehicleApp(
        IVehicleDataBrokerClient::createInstance("vehicledatabroker"),  // Connect to vehicle data
        IPubSubClient::createInstance("SampleApp"))                     // Connect to MQTT
{}
```

#### 2. onStart() - Subscribe to Data
```cpp
void SampleApp::onStart() {
    // Subscribe to vehicle speed changes
    subscribeDataPoints(QueryBuilder::select(Vehicle.Speed).build())
        ->onItem([this](auto&& item) { onSpeedChanged(item); });
    
    // Subscribe to MQTT requests  
    subscribeToTopic("sampleapp/getSpeed")
        ->onItem([this](auto&& data) { onGetSpeedRequestReceived(data); });
}
```

#### 3. Handle Vehicle Data Changes
```cpp
void SampleApp::onSpeedChanged(const DataPointReply& reply) {
    auto speed = reply.get(Vehicle.Speed)->value();
    
    // Publish speed to MQTT
    nlohmann::json json({{"speed", speed}});
    publishToTopic("sampleapp/currentSpeed", json.dump());
}
```

#### 4. Handle MQTT Requests
```cpp
void SampleApp::onGetSpeedRequestReceived(const std::string& data) {
    // Get current speed from vehicle
    auto speed = Vehicle.Speed.get()->await().value();
    
    // Send response
    nlohmann::json response({{"current_speed", speed}});
    publishToTopic("sampleapp/getSpeed/response", response.dump());
}
```

## Development Pattern

### Traditional Embedded
```c
// Direct hardware access
CAN_MSG msg;
CAN_Read(&msg);
if (msg.id == SPEED_ID) {
    speed = msg.data[0];
}
```

### Vehicle App
```cpp
// High-level vehicle signals  
auto speed = Vehicle.Speed.get()->await().value();
// OR subscribe to changes:
subscribeDataPoints(QueryBuilder::select(Vehicle.Speed).build())
    ->onItem([](auto item) { /* handle change */ });
```

## Creating Your Own App

### 1. Modify AppManifest.json
```json
{
    "name": "MySpeedMonitor",
    "interfaces": [
        {
            "type": "vehicle-signal-interface", 
            "datapoints": { "path": "Vehicle.Speed", "access": "read" }
        },
        {
            "type": "pubsub",
            "writes": ["myapp/alert"]  // Add your topics
        }
    ]
}
```

### 2. Update SampleApp.cpp
```cpp
void SampleApp::onSpeedChanged(const DataPointReply& reply) {
    auto speed = reply.get(Vehicle.Speed)->value();
    
    // Your logic here
    if (speed > 80.0) {
        nlohmann::json alert({{"alert", "SPEED_LIMIT_EXCEEDED"}});
        publishToTopic("myapp/alert", alert.dump());
    }
}
```

### 3. Build & Test
```bash
./build.sh
# Tasks → "Local Runtime - Run VehicleApp"
```

## Key Concepts

- **Vehicle.Speed** - Auto-generated from Vehicle Signal Specification
- **DataPointReply** - Container for vehicle data responses  
- **MQTT Topics** - Communication with other apps/services
- **subscribeDataPoints()** - Listen for vehicle data changes
- **publishToTopic()** - Send messages via MQTT

## Debug & Monitor

- **Logs**: Check VSCode terminal for app output
- **MQTT**: Use VSCode MQTT extension to monitor topics
- **Debug**: Press F5 to debug with breakpoints

**Bottom Line**: Vehicle apps = Linux apps that talk to car via network protocols instead of direct hardware access.