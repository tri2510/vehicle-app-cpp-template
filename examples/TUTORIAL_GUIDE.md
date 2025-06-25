# 🎓 SDV Programming Tutorial Series

## 🚀 Learn Software-Defined Vehicle Development Step by Step

This tutorial series teaches you to build vehicle applications from simple speed monitoring to advanced fleet management systems. Each step builds upon the previous one, providing hands-on experience with real vehicle signals.

## 📚 Tutorial Structure

### **Progressive Learning Path:**
1. **Step 1**: Basic Speed Monitoring → Learn signal subscription and processing
2. **Step 2**: Multi-Signal Processing → Handle multiple vehicle data streams  
3. **Step 3**: Data Analysis & Alerts → Implement business logic and notifications
4. **Step 4**: Advanced Fleet Management → Build production-grade applications

### **What You'll Learn:**
- 🔧 Vehicle signal subscription and processing
- 📊 Real-time data analysis and pattern detection
- 🚨 Alert systems and business rule implementation
- 🗺️ GPS-based functionality and geofencing
- 📈 Fleet analytics and performance monitoring
- 🏗️ Scalable SDV application architecture

## 📋 Prerequisites

**Required:**
- Docker installed and running
- Basic C++ knowledge
- Command line familiarity

**Setup Commands:**
```bash
# Start KUKSA databroker
docker compose -f docker-compose.dev.yml up -d vehicledatabroker

# Option A: Use pre-built image (RECOMMENDED - instant start)
# No build needed! The pre-built image is used directly in commands below

# Option B: Build container locally (3-5 minutes)
docker build -f Dockerfile.quick -t velocitas-quick .

# Create persistent volumes for faster builds
docker volume create tutorial-build
docker volume create tutorial-deps
docker volume create tutorial-vss
```

**Note:** Throughout this tutorial, we use `velocitas-quick` in commands. If using the pre-built image, replace with:
```bash
ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest
```

## 🎯 Quick Start Guide

### **Run Any Tutorial Step:**

**Using Pre-built Image (Recommended):**
```bash
# Replace X with step number (1, 2, 3, or 4)
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v tutorial-deps:/home/vscode/.conan2 \
  -v tutorial-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/StepX_*.cpp:/app.cpp \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest build --skip-deps --verbose

# Run the application
docker run -d --network host --name tutorial-app \
  -v tutorial-build:/quickbuild/build \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  ghcr.io/tri2510/vehicle-app-cpp-template/velocitas-quick:prerelease-latest run 60
```

**Using Local Build:**
```bash
# Same commands but with 'velocitas-quick' instead of the full image name
docker run --rm --network host \
  -v tutorial-build:/quickbuild/build \
  -v tutorial-deps:/home/vscode/.conan2 \
  -v tutorial-vss:/quickbuild/app/vehicle_model \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/examples/StepX_*.cpp:/app.cpp \
  velocitas-quick build --skip-deps --verbose
```

### **Test with Vehicle Signals:**
```bash
# Monitor application logs
docker logs tutorial-app --follow

# Inject test signals
echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

## 📖 Tutorial Steps

| Step | Focus | Complexity | Time | Key Concepts |
|------|-------|------------|------|--------------|
| **[Step 1](Step1_TUTORIAL.md)** | Basic Speed Monitoring | ⭐ Beginner | 15 min | Signal subscription, speed conversion |
| **[Step 2](Step2_TUTORIAL.md)** | Multi-Signal Processing | ⭐⭐ Intermediate | 30 min | Multiple signals, data correlation |
| **[Step 3](Step3_TUTORIAL.md)** | Data Analysis & Alerts | ⭐⭐⭐ Advanced | 45 min | Business logic, alerts, pattern analysis |
| **[Step 4](Step4_TUTORIAL.md)** | Fleet Management | ⭐⭐⭐⭐ Professional | 60 min | Production architecture, GPS, analytics |

## 🛠️ Learning Approach

### **Incremental Development:**
- **Step 1**: Start with minimal signal processing
- **Step 2**: Add more signals and data types  
- **Step 3**: Implement business rules and alerts
- **Step 4**: Build comprehensive fleet system

### **Hands-On Testing:**
- Each step includes interactive testing procedures
- Real vehicle signals injected via KUKSA client
- Immediate feedback and validation
- Progressive complexity building

### **Production Readiness:**
- Learn industry best practices
- Understand SDV architecture patterns
- Implement scalable solutions
- Build real-world applications

## 🔍 What Each Step Teaches

### **Step 1: Basic Speed Monitoring** 
```cpp
// Learn: Signal subscription, basic processing
void onSignalChanged(const velocitas::DataPointReply& reply) {
    if (reply.get(Vehicle.Speed)->isValid()) {
        double speed = reply.get(Vehicle.Speed)->value();
        processSpeed(speed);  // Convert m/s to km/h
    }
}
```

### **Step 2: Multi-Signal Processing**
```cpp
// Learn: Multiple signals, data correlation
void onSignalChanged(const velocitas::DataPointReply& reply) {
    double speed = reply.get(Vehicle.Speed)->value();
    double rpm = reply.get(Vehicle.Powertrain.Engine.Speed)->value();
    double fuel = reply.get(Vehicle.Powertrain.FuelSystem.Level)->value();
    
    analyzeVehiclePerformance(speed, rpm, fuel);
}
```

### **Step 3: Data Analysis & Alerts**
```cpp
// Learn: Business logic, pattern detection
void analyzeSpeedPattern(double speed) {
    if (detectErraticDriving(speed)) {
        generateAlert("ERRATIC_DRIVING", speed);
    }
    updatePerformanceMetrics(speed);
}
```

### **Step 4: Advanced Fleet Management**
```cpp
// Learn: Production architecture, GPS, analytics
class FleetManager {
    void processFleetData(double speed, double lat, double lon) {
        checkSpeedZones(speed, lat, lon);
        updateFleetAnalytics(speed);
        generateComplianceReports();
    }
};
```

## 📊 Expected Learning Outcomes

### **By Step 1 Completion:**
- ✅ Understand vehicle signal subscription
- ✅ Process speed data with unit conversion
- ✅ Build and test simple SDV applications

### **By Step 2 Completion:**
- ✅ Handle multiple simultaneous signals
- ✅ Correlate different data types
- ✅ Implement efficient data processing

### **By Step 3 Completion:**
- ✅ Implement business rules and alerts
- ✅ Detect patterns in vehicle data
- ✅ Build production-ready logic

### **By Step 4 Completion:**
- ✅ Design scalable SDV architectures
- ✅ Implement GPS-based functionality
- ✅ Build comprehensive fleet systems

## 🧪 Testing Strategy

### **Validation at Each Step:**
1. **Build Verification**: Application compiles successfully
2. **Signal Processing**: Correct data handling and conversion
3. **Business Logic**: Alerts and rules work as expected
4. **Performance**: Efficient processing and resource usage

### **Interactive Testing:**
- Real-time signal injection with KUKSA client
- Log monitoring for immediate feedback
- Progressive complexity validation
- Performance benchmarking

## 🎯 Success Metrics

### **Technical Proficiency:**
- **Code Quality**: Clean, readable, maintainable C++
- **Architecture**: Well-structured, scalable design
- **Performance**: Efficient signal processing
- **Testing**: Comprehensive validation procedures

### **SDV Understanding:**
- **Vehicle Signals**: VSS (Vehicle Signal Specification) knowledge
- **Data Processing**: Real-time automotive data handling
- **Business Logic**: Fleet management and compliance
- **Integration**: KUKSA databroker and ecosystem

## 📝 Next Steps After Tutorial

### **Advanced Topics:**
- Custom vehicle signal specifications
- Integration with external APIs (weather, traffic, fleet management)
- Machine learning for predictive maintenance
- Mobile app integration for fleet managers
- Database integration for historical data
- CI/CD pipelines for vehicle app deployment

### **Production Considerations:**
- Security and authentication
- Scalability and load balancing
- Monitoring and observability
- Compliance and regulatory requirements
- Over-the-air updates and deployment

---

## 🚀 Start Your SDV Journey

**Begin with Step 1** and progress through each tutorial at your own pace. Each step builds essential skills for Software-Defined Vehicle development.

**[Start with Step 1: Basic Speed Monitoring →](Step1_TUTORIAL.md)**

---

*🚗💨 Master SDV development with hands-on vehicle programming!*